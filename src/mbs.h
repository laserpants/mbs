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
 * @brief This header file defines the methods and data structures that provide 
 *        the core functionality of the command.
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
 * @brief A rx-tx pair which represents the amount of data received and 
 *        transmitted over a network interface.
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
 * @brief Configuration flags controlling the behavior of the application.
 */
enum cmd_flags
{
    /**
     * If this flag is set, verbose output is enabled.
     */
    FLAG_VERBOSE = 1 << 0,

    /**
     * Is the application running in countdown mode?
     */
    FLAG_COUNTDOWN = 1 << 1,

    /**
     * Setting this flag disables non-ascii Unicode characters in the terminal
     * interface.
     */
    FLAG_ASCII = 1 << 2,

    /**
     * This flag tells the command not to exit when the data limit is exceeded 
     * or the connection is lost.
     */
    FLAG_NO_EXIT = 1 << 3  
};

/**
 * @brief This struct encapsulates various application state and configuration
 *        settings. The fields of the struct roughly correspond to what one 
 *        normally would have defined as data members in a class-based object 
 *        oriented language setting. However, since a C struct cannot have 
 *        methods, we allocate `mbs` on the stack in \ref main, and pass it 
 *        explicitly, as an argument, to the various functions that require 
 *        access to application state.
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
     *
     * @see cmd_flags
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
 * @brief Translate \a bytes to human-readable form.
 *
 * The input is written (as a string) to \a buf, expressed as a (rounded) 
 * multiplicative factor of the nearest lower power of 1024. For instance 1442 
 * is translated to the string `1.4K`. Note that \a buf must be large enough to 
 * contain the result, including the terminating null character.
 *
 * The function returns a pointer to the character buffer, so that it can be 
 * used in function composition, as in the following example:
 *
 * @code
 * char buf[10];
 * printf ("Available data: %s\n", to_human_readable (bytes, buf)); 
 * @endcode
 *
 * @param  bytes The number of bytes to translate.
 * @param  buf   A character buffer to write the result to.
 * @return       A pointer identical to \a buf.
 */
char *to_human_readable (double bytes, char *buf);

/**
 * @brief Convert the input string, which may include a size suffix, to an
 *        unsigned integer.
 *
 * This is a utility function, used to parse user provided byte amounts (e.g.,
 * `10K`) to numeric form. If no suffix is given, the behavior of this function
 * should be similar, if not identical, to that of `strtoull()`. 
 *
 * The following suffixes are recognized:
 *
 * | Suffix                     | Unit                   | Size             |
 * |----------------------------|------------------------|------------------|
 * | **B**, **b**, or no suffix | Bytes                  | 1                |
 * | **kB**, or **k**           | Kilobyte               | 1000             |
 * | **KB**, **K**, or **KiB**  | Kibibyte<sup>†</sup>   | 2<sup>10</sup>   |
 * | **mB**, or **m**           | Megabyte               | 1000<sup>2</sup> |
 * | **MB**, **M**, or **MiB**  | Mebibyte<sup>†</sup>   | 2<sup>20</sup>   |
 * | **gB**, or **g**           | Gigabyte               | 1000<sup>3</sup> |
 * | **GB**, **G**, or **GiB**  | Gibibyte<sup>†</sup>   | 2<sup>30</sup>   |
 *
 * †) Defined by the International Electrotechnical Commission (IEC).
 *
 * @param  str    The input string, which must be terminated by a null 
 *                character.
 * @param  result A pointer to a 64-bit integer, which will contain the result
 *                if the function is successful (i.e., the return code is 0).
 * @return        0 on success, or -1 if an error occured.
 */
int parse_bytes (const char *str, uint64_t *result);

/**
 * @brief Parse command-line arguments and initialize the \ref mbs struct.
 *
 * The implementation uses the [Argtable3](http://www.argtable.org/) library.
 *
 * @param  argc Passed on from \ref main.
 * @param  argv Passed on from \ref main.
 * @param  s    An \ref mbs struct, to which configuration settings will be 
 *              written.
 * @return      Nothing
 */
void mbs_getopt (int argc, char *argv[], struct mbs *s);

/**
 * @brief Example showing how to document a function with Doxygen.
 *
 * Description
 *
 * @param  s An \ref mbs struct holding application state and configuration
 *         settings.
 * @param  stats This is an argument.
 * @return something
 */
int mbs_poll_interfaces (struct mbs *s, struct stats *stats);

#endif 
