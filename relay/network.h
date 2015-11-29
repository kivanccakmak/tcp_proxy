#ifndef NETWORK_HEADERS_H
#define NETWORK_HEADERS_H

#include <stdio.h>

/**
 * @brief number of bytes in raw_tcp data
 */
#define BLOCKSIZE 6

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

#endif

