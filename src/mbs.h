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

/**
 * @file mbs.h
 * @brief File containing example of doxygen usage for quick reference.
 *
 * Here typically goes a more extensive explanation of what the header
 * defines. 
 *
 * @author Johannes Hildén <hildenjohannes@gmail.com>
 */
#ifndef MBS_H
#define MBS_H

#include <stdint.h>
#include <ncurses.h>

/**
 * @brief Example showing how to document a function with Doxygen.
 */
struct stats 
{
    /** 
     * @brief Bytes received 
     */
    uint64_t rx_bytes;           

    /**
     * @brief Bytes transmitted 
     */
    uint64_t tx_bytes;           
};

/**
 * @brief Example showing how to document a function with Doxygen.
 */
enum 
{
    /**
     * @brief Example showing how to document a function with Doxygen.
     */
    FLAG_VERBOSE   = 1 << 0,

    /**
     * @brief Example showing how to document a function with Doxygen.
     */
    FLAG_COUNTDOWN = 1 << 1,

    /**
     * @brief Example showing how to document a function with Doxygen.
     */
    FLAG_ASCII     = 1 << 2,

    /**
     * @brief Example showing how to document a function with Doxygen.
     */
    FLAG_NO_EXIT   = 1 << 3  
};

/**
 * @brief Example showing how to document a function with Doxygen.
 */
struct mbs
{
    /**
     * @brief Last TX RX value pair read.
     */
    struct stats snapshot;       

    /**
     * @brief Amount of data used since the command was launched.
     */
    struct stats used;           

    /**
     * @brief Current data balance estimate (in bytes). 
     */
    uint64_t balance;        

    /**
     * @brief Bit flags 
     */
    uint8_t flags;          

    /**
     * @brief Network interface name. 
     */
    char *ifa_name;       

    /**
     * @brief ncurses window.
     */
    WINDOW *win;            
};

/**
 * @brief Example showing how to document a function with Doxygen.
 *
 * Description
 *
 * @param  bytes This is an argument.
 * @param  buf This is an argument.
 * @return something
 */
char *to_human_readable (double bytes, char *buf);

/**
 * @brief Example showing how to document a function with Doxygen.
 *
 * Description
 *
 * @param  str This is an argument.
 * @param  result This is an argument.
 * @return something
 */
int parse_bytes (const char *str, uint64_t *result);

/**
 * @brief Example showing how to document a function with Doxygen.
 *
 * Description
 *
 * @param  argc Argument one
 * @param  argv Argument two
 * @param  s Argument three
 * @return void 
 */
void mbs_getopt (int argc, char *argv[], struct mbs *s);

/**
 * @brief Example showing how to document a function with Doxygen.
 *
 * Description
 *
 * @param  s This is an argument.
 * @param  stats This is an argument.
 * @return something
 */
int mbs_poll_interfaces (struct mbs *s, struct stats *stats);

#endif 
