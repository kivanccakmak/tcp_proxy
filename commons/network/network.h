/*
 * Copyright (C) 2016  Kıvanç Çakmak <kivanccakmak@gmail.com>
 * Author: Kıvanç Çakmak <kivanccakmak@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef NETWORK_HEADERS_H
#define NETWORK_HEADERS_H

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * @brief number of bytes in raw_tcp data
 */
#define BLOCKSIZE 1000 

/**
 * @brief number of connections that 
 * receiver side of proxy allows
 */
#define BACKLOG 10

/**
 * @brief
 *
 * own encapsulation method to structure,
 * to be able to demultiplex packets comming
 * from different TCP connections.
 *
 * [header][raw_data]
 *
 * where, header is joint sequence number
 * of multiple TCO connections.
 *
 */
typedef struct encaps_packet{
    unsigned short seq;
    unsigned char raw_packet[BLOCKSIZE];
} __attribute__((packed)) encaps_packet_t;

#define PACKET_SIZE sizeof(encaps_packet_t)

uint32_t get_in_portnum(struct sockaddr *sa);
void *get_in_ipaddr(struct sockaddr *sa);

#endif
