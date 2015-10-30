#include "boss_server_headers.h"
#include "boss_server.h"

int main(int argc, char * argv[]){
    if (argc != 1){
        printf("usage: %s \n", argv[0]);
    }else{
        struct dictate rx_boss;
        rx_boss.max_sockets = 10;
        struct dictate * r;
        r = &rx_boss;
        r->max_sockets = 3;
        execute_recvs(r->max_sockets);
        execute_sec(rx_boss);
        execute_third(&rx_boss);
    }
    return 0;
}

void execute_recvs(int max_val){
    printf("max_sockets: %d\n", max_val);
}

void execute_sec(struct dictate val){
    printf("max_sockets: %d\n", val.max_sockets);
}

void execute_third(struct dictate *p){
    printf("max_sockets: %d\n", p->max_sockets);
}
