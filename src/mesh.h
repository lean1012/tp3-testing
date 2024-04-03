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

#ifndef __mesh_H
#define __mesh_H

/** @file
 ** @brief
 */

/* === Headers files inclusions =============================================================== */
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

/* === Public macros definitions =============================================================== */
#define SRC                   0
#define DST                   1
#define NEXT_HOP              2
#define OPCODE                3
#define LENGHT                4
#define MSG                   5

#define MAX_SIZE_MSG          20
#define SRC_DIR               10

#define BROADCAST_DIR         0xFD
#define NULL_DIR              0xFE
#define UNREACHABLE_DIR       0xFF

#define OPCODE_CONNECTION_MIN 11
#define OPCODE_CONNECTION_MAX 20
#define OPCODE_ROUTING_MIN    21
#define OPCODE_ROUTING_MAX    30
#define OPCODE_APP_MIN        31
#define OPCODE_APP_MAX        100

/* === Public data type declarations =========================================================== */
struct msg {
  uint8_t src;
  uint8_t dst;
  uint8_t next_hop;
  uint8_t opcode;
  uint8_t lenght;
  uint8_t msg[MAX_SIZE_MSG];
};

/* === Public variable declarations ============================================================ */

/* === Public function declarations ============================================================ */
void mesh_init();

void mesh_add_opcode(uint8_t opcode_number, int (*p_func)(uint8_t * msg, int len));

int mesh_add_per(uint8_t * conn);

int mesh_delete_per(uint8_t * conn);

int mesh_send_msg(uint8_t * msg);
/* === End of documentation ==================================================================== */

#endif
