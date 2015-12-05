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

#include <net/if.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../../relay/network.h"

#include <libnetfilter_queue/libnetfilter_queue.h>

#define BUFF_SIZE 10

struct ipv4_packet{
    struct iphdr iph;
    struct tcphdr tcph;
};

struct cb_args_syn{
    struct nfq_q_handle *qh;
    struct nfgenmsg *nfmsg;
    struct nfq_data *nfa;
    void *data;
};

struct listen_args{
    char *dest_ip;
    char *dest_port;
    char *local_port;
};

#define QUEUER_BUF_SIZE 10000

#endif 
