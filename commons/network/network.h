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
