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

#ifndef __mesh_routing_H
#define __mesh_routing_H

/** @file
 ** @brief
 */

/* === Headers files inclusions =============================================================== */
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
/* === Public macros definitions =============================================================== */
#define MAX_NEIGHBOR 20

/* === Public data type declarations =========================================================== */

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */

/**
 * @brief Función que inicializa y vacía la tabla de rutas.
 *
 */
void mesh_routing_init(void);

/**
 * @brief Función que permite enviar un mensaje a la capa routing
 *
 * @param msg msg a enviar a la capa routing
 */
void mesh_routing_send_msg(uint8_t * msg);

/**
 * @brief Función que muestra la tabla de rutas
 *
 */
void mesh_routing_display_routing_table();

/**
 * @brief Handler que envía los mensajes hello para el descubrimiento de vecinos y verifica que las
 * rutas sigan disponibles. Si se ejecuta esta handler cada X segundos, el mensaje hello se enviará
 * con una frecuencia de 2*X segundos. y se verificará que las rutas estén disponibles cada 3*X
 * segundos.
 *
 */
void mesh_routing_handler_time_out();

/* === End of documentation ==================================================================== */

#endif
