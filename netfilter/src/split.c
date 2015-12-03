#include "split.h"

static pthread_t queuer;
static pthread_t listener;
static pthread_t poller;

#define QUEUE_NUM 0 

static struct cb_args_syn* init_syn_args(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa,
        void *data); 

static int nfqueue_get_syn(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa,
        void *data);


int main(int argc, char *argv[])
{
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    struct nfnl_handle *nh;

    int fd, ret;
    int rc = 1;
    char buf[QUEUER_BUF_SIZE];

    printf("Opening NFQ library handle");
    h = nfq_open();

    nfq_unbind_pf(h, AF_INET);
    ret = nfq_bind_pf(h, AF_INET);
    if (ret < 0) {
        printf("failed to bind\n");
    }

    qh = nfq_create_queue(h, QUEUE_NUM, &nfqueue_get_syn, NULL);

    ret = nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);
    if (ret < 0) {
        printf("Failed to setup NFQ packet_copy mode\n");
    }

    nh = nfq_nfnlh(h);
    fd = nfnl_fd(nh);

    while (rc > 0) {
        rc = recv(fd, buf, QUEUER_BUF_SIZE, 0);
        printf("received packet size : %d\n", rc);
        nfq_handle_packet(h, buf, rc);
    }

    printf("hello \n");
    return 0;
}

static struct cb_args_syn* init_syn_args(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa,
        void *data) {

    struct cb_args_syn *syn_args = (struct cb_args_syn *)
        malloc(sizeof(struct cb_args_syn));
    syn_args->qh = qh;
    syn_args->nfmsg = nfmsg;
    syn_args->nfa = nfa;
    syn_args->data = data;
    return syn_args;
}



static int nfqueue_get_syn(struct nfq_q_handle *qh,
        struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data) 
{
    unsigned char *buffer;
    struct ipv4_packet *ip4;
	int id = 0, ret, added = 0;
	struct nfqnl_msg_packet_hdr *ph;
    int addr;

    ph = nfq_get_msg_packet_hdr(nfa);
    if (!ph) {
        perror("unable to get packet header");
    }

    id = ntohl(ph->packet_id);
    ret = nfq_get_payload(nfa, &buffer);

    ip4 = (struct ipv4_packet *) buffer;

    printf("src: %d\n", ntohl(ip4->iph.saddr));

    return 0;
}
