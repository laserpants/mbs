/*
 * Copyright (c) 2017 Johannes Hildén <hildenjohannes@gmail.com>
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 * 
 *     * Neither the name of copyright holder nor the names of other
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef MBS_H
#define MBS_H

#include <stdint.h>
#include <ncurses.h>

struct stats 
{
    uint64_t rx_bytes;           /* Bytes received */
    uint64_t tx_bytes;           /* Bytes trasmitted */
};

#define FLAG_VERBOSE   (1 << 0)  /* 00000001 */
#define FLAG_COUNTDOWN (1 << 1)  /* 00000010 */
#define FLAG_ASCII     (1 << 2)  /* 00000100 */
#define FLAG_NO_EXIT   (1 << 3)  /* 00001000 */

struct mbs
{
    struct stats snapshot;       /* Last TX RX value pair read */
    struct stats used;           /* Data used since the command was launched */
    uint64_t     balance;        /* Current data balance estimate (in bytes) */
    uint8_t      flags;          /* Bit flags */
    char        *ifa_name;       /* Network interface name */
    WINDOW      *win;            /* ncurses window */
};

char *to_human_readable (double bytes, char *buf);
int parse_bytes (const char *str, uint64_t *result);
void mbs_getopt (int argc, char *argv[], struct mbs *s);
int mbs_poll_interfaces (struct mbs *s, struct stats *stats);

#endif 
