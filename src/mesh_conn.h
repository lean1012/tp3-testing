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

#ifndef __mesh_conn_H
#define __mesh_conn_H

/** @file
 ** @brief
 */

/* === Headers files inclusions =============================================================== */

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

/* === Public macros definitions =============================================================== */

/* === Public data type declarations =========================================================== */

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */
/**
 * @brief Inicializa el modulo conn sin ninguna conexión
 *
 */
void mesh_conn_init();

/**
 * @brief Agrega una conexión
 *
 * @param conn id de la conexión ble
 * @return int
 */
int mesh_conn_add_per(uint8_t * conn);

/**
 * @brief Elimina una conexíon
 *
 * @param conn id de la conexión ble
 * @return int
 */
int mesh_conn_delete_per(uint8_t * conn);

/**
 * @brief Recive un msg desde la capa ble
 *
 * @param p_conn id de la conexión ble
 * @param msg msg a procesar
 */
void mesh_conn_rcv_ble_msg(uint8_t * p_conn, uint8_t * msg);

/**
 * @brief envia un msg a la capa conn
 *
 * @param id_mesh id del nodo
 * @param msg msg a procesar
 */
void mesh_conn_send_msg(uint8_t id_mesh, uint8_t * msg);

/* === End of documentation ==================================================================== */

#endif
