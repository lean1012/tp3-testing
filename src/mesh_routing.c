/************************************************************************************************
Copyright (c) 2023, Leandro Diaz <diazleandro1012@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*************************************************************************************************/

/** @file mesh_routing.c
 ** @brief Esta capa se encarga se encarga de generar la tabla de enrutamiento, decidir el próximo
 *         destino de los msg e informar a los vecinos su tabla de ruta. Se comunica con las capas
 *         mesh_conn y mesh_app (mesh_conn capa encargada de guardar el estado de la conexión entre
 *         los nodos e identificarlos y mesh_app encargado de procesar el msg de aplicación). Para
 *         cada destino se almacenan dos posibles proximos saltos, para el enrutamiento se utiliza
 *         siempre el primero. En caso que la primera ruta no se pueda alcanzar más, el segundo
 *         camino pasará a ser el prinicipal. Las rutas se envían periódicamente  en caso de dejar
 *         de recivir una ruta se eliminara
 */

/* === Headers files inclusions =============================================================== */
#include "mesh_routing.h"
#include "mesh.h"
#include "mesh_app.h"
#include "mesh_conn.h"
#include "mesh_port.h"
#include "stdio.h"

/* === Macros definitions ====================================================================== */

#define RCV_NEIGHBOR_OPCODE 21 // opcode para recivir vecinos

/* === Private data type declarations ========================================================== */

/**
 * @brief Representación de un elemento de la tabla de rutas.
 *
 */
struct neighbor_list {
  bool used;
  uint8_t dst;
  uint8_t next_hop;
  uint8_t metric;
  bool time_out;
  bool second_used;
  uint8_t second_next_hop;
  uint8_t second_metric;
  bool second_time_out;
};

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/**
 * @brief Tabla de rutas
 *
 */
struct neighbor_list neig_list[MAX_NEIGHBOR] = {0};

/**
 * @todo Solucionar el problema de variables compartidas,
 *
 */
static bool adding_neighbod = false;

/* === Private function implementation ========================================================= */
/**
 * @brief Función para buscar un elemento dentro de la tabla de rutas en base al destino
 *
 * @param dst destino a buscar
 * @return struct neighbor_list* devuelve un puntero a la tabla de rutas correspondiente al destino
 */
static struct neighbor_list * mesh_routing_search_element_in_table(uint8_t dst) {
  for (uint8_t i = 0; i < MAX_NEIGHBOR; i++) {
    if (neig_list[i].used == true && neig_list[i].dst == dst) {
      return &neig_list[i];
    }
  }
  return NULL;
}

/**
 * @brief Función para buscar un elemento libre dentro de la tabla de rutas para almacenar una nueva
 * ruta
 *
 * @return struct neighbor_list* devuelve un puntero que apunta al elemento vacio
 */
static struct neighbor_list * mesh_routing_get_free_element_in_table() {
  for (uint8_t i = 0; i < MAX_NEIGHBOR; i++) {
    if (neig_list[i].used == false) {
      return &neig_list[i];
    }
  }
  return NULL;
}

/**
 * @brief Función para agregar un nueva ruta y su camino a la tabla de rutas
 *
 * @param neighbor_aux  elemento de la tabla de ruta
 * @param dst destino
 * @param next_hop próximo salto
 * @param metric métrica
 */
static void mesh_routing_add_element_first_in_table(struct neighbor_list * neighbor_aux,
                                                    uint8_t dst, uint8_t next_hop, uint8_t metric) {

  neighbor_aux->used = true;
  neighbor_aux->dst = dst;
  neighbor_aux->metric = metric;
  neighbor_aux->next_hop = next_hop;
  neighbor_aux->time_out = false;
}

/**
 * @brief Función para agregar un segundo camino a la tabla de rutas
 *
 * @param neighbor_aux elemento de la tabla de ruta
 * @param next_hop próximo salto
 * @param metric métrica
 */
static void mesh_routing_add_element_second_in_table(struct neighbor_list * neighbor_aux,
                                                     uint8_t next_hop, uint8_t metric) {

  neighbor_aux->second_used = true;
  neighbor_aux->second_metric = metric;
  neighbor_aux->second_next_hop = next_hop;
  neighbor_aux->second_time_out = false;
}

/**
 * @brief Funcion para pasar el segundo camino al primer camino y el primer camino al segundo
 *
 * @param neighbor_aux elemento de la talba de ruta
 */
static void
mesh_routing_swap_first_element_in_table_to_second(struct neighbor_list * neighbor_aux) {

  uint8_t second_metric_aux, second_next_hop_aux;
  second_metric_aux = neighbor_aux->second_metric;
  second_next_hop_aux = neighbor_aux->second_next_hop;
  neighbor_aux->second_metric = neighbor_aux->metric;
  neighbor_aux->second_next_hop = neighbor_aux->next_hop;
  neighbor_aux->metric = second_metric_aux;
  neighbor_aux->next_hop = second_next_hop_aux;
}

/**
 * @brief Función para actualizar el timeout de las rutas para eliminar posibles rutas
 * desactualizadas
 *
 * @param neighbor_aux elemento de la talba de ruta
 * @param next_hop próximo salto
 * @param metric métrica
 */
static void mesh_routing_update_time_out(struct neighbor_list * neighbor_aux, uint8_t next_hop,
                                         uint8_t metric) {

  if (neighbor_aux->next_hop == next_hop && neighbor_aux->metric == metric) {

    neighbor_aux->time_out = false;

  } else if (neighbor_aux->second_next_hop == next_hop && neighbor_aux->second_metric == metric) {
    neighbor_aux->second_time_out = false;
  }
}

/**
 * @brief Función que permite agregar un rutas a determinado destino. Guarda hasta 2 caminos
 * posibles. Si recive una ruta con una menor métrica que la almacenada lo guarda en el primer lugar
 * y metrica anterior pasa a ser la segunda opción. Si recive una métrica mayor que la primera pero
 * menor que la segunda esta pasa a remplazar la segunda. Cuando se reciven las rutas ya almacenadas
 * se setea el time_out en false para evitar que se borren
 *
 * @param dst destino
 * @param next_hop próximo salto
 * @param metric métrica
 */
static void mesh_routing_add_neighbor(uint8_t dst, uint8_t next_hop, uint8_t metric) {

  if ((dst == SRC_DIR || next_hop == SRC_DIR)) // si el dst o src es el mismo no hago nada
    return;

  struct neighbor_list * neig_search = mesh_routing_search_element_in_table(dst);

  if (neig_search == NULL) {

    struct neighbor_list * neighbor_aux = mesh_routing_get_free_element_in_table();

    mesh_routing_add_element_first_in_table(neighbor_aux, dst, next_hop, metric);
    mesh_routing_update_time_out(neighbor_aux, next_hop, metric);
    return;
  }

  if (neig_search->metric > metric) {

    mesh_routing_swap_first_element_in_table_to_second(neig_search);
    mesh_routing_add_element_first_in_table(neig_search, dst, next_hop, metric);

  } else if (neig_search->second_used == false || neig_search->second_metric > metric) {

    mesh_routing_add_element_second_in_table(neig_search, next_hop, metric);
  }

  mesh_routing_update_time_out(neig_search, next_hop, metric);
}

/**
 * @brief Elimina un elemento de la tabla de rutas
 *
 * @param dst destino a eliminar de la tabla de rutas
 */
static void mesh_routing_delete_neighbor(uint8_t dst) {

  struct neighbor_list * neig_search = mesh_routing_search_element_in_table(dst);

  neig_search->used = false;
}

/**
 * @brief Busca el siguiente salto segun el destino
 *
 * @param dst destino
 * @return uint8_t próximo salto
 */
static uint8_t mesh_routing_search_next_hop(uint8_t dst) {
  if (dst == BROADCAST_DIR) {
    return BROADCAST_DIR;
  }

  struct neighbor_list * neig_search = mesh_routing_search_element_in_table(dst);
  if (neig_search == NULL) {
    return UNREACHABLE_DIR;
  } else {
    return neig_search->next_hop;
  }
}

/**
 * @brief Setea el campo time_out en true en la tabla de rutas
 *
 */
static void mesh_routing_set_time_out_true() {
  for (int i = 0; i < MAX_NEIGHBOR; i++) {
    if (neig_list[i].used == true && neig_list[i].dst != SRC_DIR) {
      neig_list[i].time_out = true;
      if (neig_list[i].second_used == true) {
        neig_list[i].second_time_out = true;
      }
    }
  }
}

/**
 * @brief Elimina un elemento de la tabla de ruta por debido a que expiro el timeout
 *
 */
static void mesh_routing_delete_item_due_to_timeout() {

  for (int i = 0; i < MAX_NEIGHBOR; i++) {
    if (neig_list[i].used == true && neig_list[i].dst != SRC_DIR && neig_list[i].time_out == true) {
      if (neig_list[i].second_used == false || neig_list[i].second_time_out == true) {
        mesh_routing_delete_neighbor(neig_list[i].dst);
      } else {
        mesh_routing_swap_first_element_in_table_to_second(&neig_list[i]);
        neig_list[i].second_used = false;
        neig_list[i].used = true;
        // mesh_app_process_msg(NULL);
      }
    }
  }
}

/**
 * @brief Elimina toda la tabla de ruta poniendo el campo used y second use en false
 *
 */
static void mesh_routing_erase_routing_table() {

  for (uint8_t i = 0; i < MAX_NEIGHBOR; i++) {
    neig_list[i].used = false;
    neig_list[i].second_used = false;
  }
}

/**
 * @brief Función que envía la información de toda las rutas alcanzadas con el siguiente formato:
 * {dst, next_hop (él mismo), metric}.
 *
 */
static void mesh_routing_send_neighbor() {

  struct msg msg_send;
  msg_send.dst = BROADCAST_DIR;
  msg_send.src = SRC_DIR;
  msg_send.opcode = RCV_NEIGHBOR_OPCODE;

  uint8_t j = 0;

  for (int i = 0; i < MAX_NEIGHBOR; i++) {
    if (neig_list[i].used == true) {
      msg_send.msg[j] = i;
      msg_send.msg[j + 1] = SRC_DIR;
      msg_send.msg[j + 2] = neig_list[i].metric;
      j = j + 3;
    }
  }

  msg_send.lenght = j + 3;

  mesh_conn_send_msg(BROADCAST_DIR, (uint8_t *)&msg_send);
}

/**
 * @brief Función que agrega elementos a la tabla de rutas.
 *
 * @param p_neighbor puntero con las rutas a agregar con el siguiente formato {destino, next_hop,
 * metrica}. Por ejemplo si se quiere agregar que el destino 9 se llega a traves de 3 con métrica 3
 * y al destino 5 a través de 10 con metrica 1 se debe enviar con el siguiente formato: uint8_t[] =
 * {9,3,3,5,10,1}
 * @param len largo del mensaje. En el ejemplo 6.
 */
static void mesh_routing_add_neig_msg(uint8_t * p_neighbor, uint8_t len) {

  for (uint8_t i = 0; i < len; i = i + 3) {

    mesh_routing_add_neighbor(p_neighbor[i], p_neighbor[i + 1], p_neighbor[i + 2] + 1);
  }
}

/**
 * @brief El msg es para la capa routing. Procesa el mensaje segun el OPCODE. OPCODE SEND_NEIGBOR =
 * mensaje donde estan las rutas alcanzadas por determinado vecino.
 *
 * @param msg puntero al msg a procesar
 */
static void mesh_routing_process_msg(uint8_t * msg) {

  switch (msg[OPCODE]) {

  case RCV_NEIGHBOR_OPCODE:
    mesh_routing_add_neig_msg(&msg[MSG], msg[LENGHT]);
    break;

  default:
    break;
  }
}

/**
 * @brief Función que rutea el mensaje poniendo el próximo salto en el campo NEXT_HOP del msg en
 * caso que el msg no sea para él mismo.
 *
 * @param msg puntero al msg a rutear
 */
static void mesh_routing_routing_msg(uint8_t * msg) {

  if (msg[DST] == SRC_DIR || msg[DST] == BROADCAST_DIR) {
    mesh_app_process_msg(msg);
  } else {
    uint8_t next_hop = mesh_routing_search_next_hop(msg[DST]);
    if (next_hop != UNREACHABLE_DIR) {
      msg[NEXT_HOP] = next_hop;
      mesh_conn_send_msg(next_hop, msg);
    }
  }
}

/* === Public function implementation ========================================================== */

void mesh_routing_init(void) {

  mesh_routing_erase_routing_table();
  struct neighbor_list * neighbor_aux = mesh_routing_get_free_element_in_table();
  neighbor_aux->used = true;
  neighbor_aux->dst = SRC_DIR;
  neighbor_aux->next_hop = SRC_DIR;
  neighbor_aux->metric = 0;
}

void mesh_routing_send_msg(uint8_t * msg) {

  if (msg[OPCODE] >= OPCODE_ROUTING_MIN && msg[OPCODE] <= OPCODE_ROUTING_MAX) {
    mesh_routing_process_msg(msg);

  } else {
    mesh_routing_routing_msg(msg);
  }
}

void mesh_routing_handler_time_out() {
  static uint8_t paso = 0;
  switch (paso) {
  case 0:
    mesh_routing_send_neighbor();
    paso = 1;
    break;
  case 1:
    mesh_routing_set_time_out_true();
    paso = 2;
    break;
  case 2:
    mesh_routing_send_neighbor();
    paso = 3;
    break;
  case 3:
    mesh_routing_delete_item_due_to_timeout();
    paso = 0;
    break;
  default:
    paso = 0;
    break;
  };
}

void mesh_routing_display_routing_table() {

  for (int i = 0; i < MAX_NEIGHBOR; i++) {

    if (neig_list[i].used == true) {
      uint8_t msg[50];
      sprintf(msg, "DST: %d,NEXT HOP: %d, METRIC: %d\r\n", neig_list[i].dst, neig_list[i].next_hop,
              neig_list[i].metric);
      mesh_print(msg);
    }
  }
}

/* === End of documentation
 * ==================================================================== */
