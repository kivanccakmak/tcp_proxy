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


#ifndef NFQUEUE_H
#define NFQUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/user.h>
#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include <netdb.h>
#include <stdbool.h>

#include <netinet/in.h>
#include <netinet/ether.h>
#include <linux/netfilter.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <errno.h>
#include <getopt.h>
#include <libconfig.h>

#include <net/if.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../commons/network/network.h"
#include "../commons/logger/logger.h"
#include "../commons/argv_reader/argv_read.h"

#include <libnetfilter_queue/libnetfilter_queue.h>

#define INITIAL_CAPACITY 1000
#define CONN_NUMBER 2

#define CLOSED -1
#define ACTIVE 1
#define PASSIVE 0

#define PATH_MAX 2048
#define PORT_MAX_CHAR 50
#define IP_CHAR_MAX 512

#define TX_PROXY_ARGV_NUM 4

#define TX_PROXY_LOG "tx_proxy.log"

struct ipv4_packet{
    struct iphdr iph;
    struct tcphdr tcph;
};

/**
 * @brief call-back arguments
 * of TCP SYN hijaker function
 */
struct cb_args_syn{
    struct nfq_q_handle *qh;
    struct nfgenmsg *nfmsg;
    struct nfq_data *nfa;
    void *data;
};

/**
 * @brief Splitting function
 * would push data into pointer
 * array of this struct. tx_chain()  
 * would use this struct to get data
 * and update its index and tx_byte.
 */
typedef struct proxy_buff{

    /**
     * @brief total amount of 
     * proxied bytes
     */
    int rx_byte;

    bool fin_flag;

    /**
     * @brief total amount of
     * transmitted bytes
     */
    int tx_byte;

    int set_ind;
    int get_ind;

    /**
     * @brief size of **buffer
     */
    int capacity; 

    /**
     * @brief pointers of 
     * proxied raw packets
     * added in this array
     */
    char **buffer;

    pthread_mutex_t lock;
}proxy_buff;


/**
 * @brief call-back arguments
 * of split thread
 */
struct split_args{
    /**
     * @brief nfqueue redirects
     * packets towards this port
     */
    char *local_port;

    proxy_buff *buff;
};

/**
 * @brief call-back arguments
 * of tcp_controller thread, who
 * opens and closes TCP-connections
 */
struct cb_cntrl_args{
    int conn_number;
    char *dest_ip;
    char *dest_port;
    proxy_buff *buff;
};

/**
 * @brief each tx_chain initiated
 * with link struct, state information
 * would be passed there by controller
 */
struct link{
    int src_port;
    int fd;
    int id;
    int state;
    struct link *next;
    struct link *prev;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

/**
 * @brief controller accesses link
 * list
 */
struct link_control{
    struct link *head;
    struct link *begin;
};

struct cb_tx_args{
    proxy_buff *buff;
    struct link *tx_link;
};

struct cb_nfq_args{
    char *dest_ip;
    char *dest_port;
};

#define QUEUER_BUF_SIZE 10000

#define WAIT_LIMIT 10

#endif 
