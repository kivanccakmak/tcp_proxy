#ifndef BOSS_SERVER_H
#define BOSS_SERVER_H

#define MAX_SOCKETS 10

struct dictate{
    int max_sockets;
    int socket_map[MAX_SOCKETS][2];
} rx_boss;

void execute_recvs(int);
void execute_sec(struct dictate val);
void execute_third(struct dictate *val);
#endif
