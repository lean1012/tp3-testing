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

/** @file
 ** @brief Test para mesh_routing.c
 */

/* === Headers files inclusions
 * =============================================================== */

#include "unity.h"
#include <stdint.h>

#include "Mockmesh_app.h"
#include "Mockmesh_conn.h"
#include "Mockmesh_port.h"

#include "Mockmesh.h"
#include "mesh_routing.h"

/* === Macros definitions
 * ====================================================================== */
#define SRC_TEST_MSG       0
#define DST_TEST_MSG       1
#define NEXT_HOP_TEST_MSG  2
#define OPCODE_TEST_MSG    3
#define LENGHT_TEST_MSG    4
#define MSG_TEST_MSG       5

#define SRC_DIR_TEST       10
#define BROADCAST_DIR_TEST 0xFD

/* === Private data type declarations
 * ========================================================== */

/* === Private variable declarations
 * =========================================================== */

/* === Private function declarations
 * =========================================================== */

/* === Public variable definitions
 * ============================================================= */

/* === Private variable definitions
 * ============================================================ */

uint8_t msg_send[50];

/* === Private function implementation
 * ========================================================= */

void setUp() {
  mesh_routing_init();
}

/** @test Función auxiliar para generar mensajes para agregar elementos a la tabla de ruta con su
 * correspondiente formato*/

void aux_generar_msg_para_agregar_tablas_de_ruta(uint8_t * rutas, uint8_t len) {

  msg_send[SRC_TEST_MSG] = 2;
  msg_send[DST_TEST_MSG] = 4;
  msg_send[OPCODE_TEST_MSG] = 21; // opcode send neighbor
  msg_send[LENGHT_TEST_MSG] = len;
  for (size_t i = 0; i < len; i++) { // agrego las rutas al msg
    msg_send[MSG_TEST_MSG + i] = rutas[i];
  }
}

/* === Public function implementation
 * ========================================================== */

/** @test Al inicializar el módulo routing la tabla de ruta debe estar vacia, a exepción del mismo
 * nodo el cual tiene como próximo salto el mismo y métrica 0 */
void test_inicializo_tabla_ruteo_vacia_menos_con_el_mismo() {
  uint8_t msg[] = "DST: 10,NEXT HOP: 10, METRIC: 0\r\n";
  mesh_print_Expect(msg);
  mesh_routing_display_routing_table();
}

/** @test Con la tabla de rutas vacía agregar una ruta */
void test_agrego_un_elemento_tabla_ruteo() {
  uint8_t routes[] = {1, 9, 3}; // 1er elemento dst, 2do elemento next hop, 3er elemento metrica
  aux_generar_msg_para_agregar_tablas_de_ruta(routes, sizeof(routes));
  uint8_t msg[] = "DST: 10,NEXT HOP: 10, METRIC: 0\r\n";
  mesh_print_Expect(msg);
  uint8_t msg2[] = "DST: 1,NEXT HOP: 9, METRIC: 4\r\n";
  mesh_print_Expect(msg2);
  mesh_routing_send_msg(msg_send);
  mesh_routing_display_routing_table();
}

/** @test Con la tabla de rutas vacía agregar varias rutas */
void test_agrego_varios_elementos_tabla_ruteo() {
  uint8_t routes[] = {1, 9, 3, 2, 8, 2, 13, 11, 7};
  aux_generar_msg_para_agregar_tablas_de_ruta(routes, sizeof(routes));
  uint8_t msg0[] = "DST: 10,NEXT HOP: 10, METRIC: 0\r\n";
  mesh_print_Expect(msg0);
  uint8_t msg1[] = "DST: 1,NEXT HOP: 9, METRIC: 4\r\n";
  mesh_print_Expect(msg1);
  uint8_t msg2[] = "DST: 2,NEXT HOP: 8, METRIC: 3\r\n";
  mesh_print_Expect(msg2);
  uint8_t msg3[] = "DST: 13,NEXT HOP: 11, METRIC: 8\r\n";
  mesh_print_Expect(msg3);
  mesh_routing_send_msg(msg_send);
  mesh_routing_display_routing_table();
}

/** @test enviar un mensaje para el mismo y ver que el mensaje es pasado a la capa de aplicación*/
void test_rutear_mensaje_para_el_mismo() {
  msg_send[SRC_TEST_MSG] = 3;
  msg_send[DST_TEST_MSG] = SRC_DIR_TEST;
  msg_send[OPCODE_TEST_MSG] = 78;
  msg_send[LENGHT_TEST_MSG] = 1;
  msg_send[MSG_TEST_MSG] = '1';
  mesh_app_process_msg_Expect((uint8_t *)&msg_send[0]);
  mesh_routing_send_msg((uint8_t *)&msg_send[0]);
}

/** @test enviar un mensaje para broadcast y ver que el mensaje es pasado a la capa de aplicación*/
void test_rutear_mensaje_para_broadcast() {
  msg_send[SRC_TEST_MSG] = 2;
  msg_send[DST_TEST_MSG] = BROADCAST_DIR_TEST;
  msg_send[OPCODE_TEST_MSG] = 78;
  msg_send[LENGHT_TEST_MSG] = 1;
  msg_send[MSG_TEST_MSG] = '1';
  mesh_app_process_msg_Expect((uint8_t *)&msg_send[0]);
  mesh_routing_send_msg((uint8_t *)&msg_send[0]);
}

/** @test enviar un mensaje para una ruta que no está definida en la tabla y ver que no se pasa a
 * ninguna capa*/
void test_rutear_mensaje_para_una_ruta_no_alcanzable() {
  msg_send[SRC_TEST_MSG] = 2;
  msg_send[DST_TEST_MSG] = 4;
  msg_send[OPCODE_TEST_MSG] = 78;
  msg_send[LENGHT_TEST_MSG] = 1;
  msg_send[MSG_TEST_MSG] = '1';
  mesh_routing_send_msg((uint8_t *)&msg_send[0]);
}

/** @test enviar un mensaje para una ruta definida en la tabla y ver que se pasa a la capa de conn
 * con su próximo salto
 */
void test_rutear_mensaje_para_una_ruta_alcanzable() {
  uint8_t dst = 1;
  uint8_t proxsalto = 9;
  uint8_t routes[] = {dst, proxsalto, 3};
  aux_generar_msg_para_agregar_tablas_de_ruta(routes, sizeof(routes));
  mesh_routing_send_msg(msg_send);
  msg_send[SRC_TEST_MSG] = 4;
  msg_send[DST_TEST_MSG] = dst;
  msg_send[OPCODE_TEST_MSG] = 78;
  msg_send[LENGHT_TEST_MSG] = 1;
  msg_send[MSG_TEST_MSG] = '1';
  mesh_conn_send_msg_Expect(proxsalto, (uint8_t *)&msg_send[0]);
  mesh_routing_send_msg((uint8_t *)&msg_send[0]);
}

/** @test enviar varios mensaje para una rutas definida en la tabla y ver que cada msj se pasa a la
 * capa de conn con sus respectivo próximo salto
 */
void test_rutear_mensaje_para_varias_rutas_alcanzable() {

  uint8_t dst1 = 1;
  uint8_t proxsalto1 = 9;
  uint8_t dst2 = 2;
  uint8_t proxsalto2 = 8;
  uint8_t dst3 = 13;
  uint8_t proxsalto3 = 11;

  uint8_t routes[] = {dst1, proxsalto1, 3, dst2, proxsalto2, 2, dst3, proxsalto3, 7};
  aux_generar_msg_para_agregar_tablas_de_ruta(routes, sizeof(routes));

  uint8_t msg_rcv1[7];
  msg_rcv1[SRC_TEST_MSG] = 4;
  msg_rcv1[DST_TEST_MSG] = dst1;
  msg_rcv1[OPCODE_TEST_MSG] = 58;
  msg_rcv1[LENGHT_TEST_MSG] = 1;
  msg_rcv1[MSG_TEST_MSG] = '4';

  uint8_t msg_rcv2[7];
  msg_rcv2[SRC_TEST_MSG] = 4;
  msg_rcv2[DST_TEST_MSG] = dst2;
  msg_rcv2[OPCODE_TEST_MSG] = 48;
  msg_rcv2[LENGHT_TEST_MSG] = 1;
  msg_rcv2[MSG_TEST_MSG] = '1';

  uint8_t msg_rcv3[7];
  msg_rcv3[SRC_TEST_MSG] = 4;
  msg_rcv3[DST_TEST_MSG] = dst3;
  msg_rcv3[OPCODE_TEST_MSG] = 48;
  msg_rcv3[LENGHT_TEST_MSG] = 1;
  msg_rcv3[MSG_TEST_MSG] = '7';

  mesh_conn_send_msg_Expect(proxsalto1, (uint8_t *)&msg_rcv1[0]);
  mesh_conn_send_msg_Expect(proxsalto2, (uint8_t *)&msg_rcv2[0]);
  mesh_conn_send_msg_Expect(proxsalto3, (uint8_t *)&msg_rcv3[0]);
  mesh_routing_send_msg(msg_send);
  mesh_routing_send_msg((uint8_t *)&msg_rcv1[0]);
  mesh_routing_send_msg((uint8_t *)&msg_rcv2[0]);
  mesh_routing_send_msg((uint8_t *)&msg_rcv3[0]);
}

/** @test Agregar y eliminar una ruta en la tabla de rutas */
void test_eliminar_elemento_tabla_ruteo_time_out() {
  uint8_t routes[] = {1, 9, 3};
  aux_generar_msg_para_agregar_tablas_de_ruta(routes, sizeof(routes));

  uint8_t msg[] = "DST: 10,NEXT HOP: 10, METRIC: 0\r\n";
  mesh_conn_send_msg_Ignore();
  mesh_print_Expect(msg);

  mesh_routing_send_msg(msg_send);
  mesh_routing_handler_time_out();
  mesh_routing_handler_time_out();
  mesh_routing_handler_time_out();
  mesh_routing_handler_time_out();

  mesh_routing_display_routing_table();
}

/** @test Agregar dos posibles rutas a un mismo destino. Eliminar la primera ruta por timeout y ver
 * que la segunda ruta pasa a la primera */
void test_eliminar_elemento_tabla_ruteo_y_quedarme_con_la_segunda_ruta() {
  uint8_t routes[] = {1, 11, 7, 1, 9, 3};
  aux_generar_msg_para_agregar_tablas_de_ruta(routes, sizeof(routes));

  uint8_t msg[] = "DST: 10,NEXT HOP: 10, METRIC: 0\r\n";
  uint8_t msg2[] = "DST: 1,NEXT HOP: 11, METRIC: 8\r\n";
  mesh_conn_send_msg_Ignore();
  mesh_print_Expect(msg);
  mesh_print_Expect(msg2);

  mesh_routing_send_msg(msg_send);
  mesh_routing_handler_time_out();
  mesh_routing_handler_time_out();
  uint8_t routes2[] = {1, 11, 7};
  aux_generar_msg_para_agregar_tablas_de_ruta(routes2, sizeof(routes2));
  mesh_routing_send_msg(msg_send);
  mesh_routing_handler_time_out();
  mesh_routing_handler_time_out();
  mesh_routing_display_routing_table();
}
/* === End of documentation
 * ==================================================================== */
