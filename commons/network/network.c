#include "network.h"
/**
 * @brief 
 *
 * Get port number of transmitter side,
 * consider whether IPv4 or IPv6 used.
 *
 * @param[in] sa
 *
 * @return 
 */
uint32_t get_in_portnum(struct sockaddr *sa)
{
    struct sockaddr_in* leg_temp;
    struct sockaddr_in6* cur_temp;
    uint32_t port;
    if(sa->sa_family == AF_INET){
        leg_temp = ((struct sockaddr_in*)sa);
        port = leg_temp->sin_port;
        return port;
    }else if(sa->sa_family == AF_INET6){
        cur_temp = ((struct sockaddr_in6*)sa);
        port = cur_temp->sin6_port;
        return port;
    }else{
        perror("cant get port number of tx");
        return 0;
    }
}

/**
 * @brief 
 *
 * Get IP address of transmitter side,
 * consider whether IPv4 or IPv6 used.
 *
 * @param[in] sa
 *
 * @return 
 */
void *get_in_ipaddr(struct sockaddr *sa)
{
    struct sockaddr_in* leg_temp;
    struct sockaddr_in6* cur_temp;
    if(sa->sa_family == AF_INET){
        leg_temp = ((struct sockaddr_in*)sa);
        return &(leg_temp->sin_addr);
    }else if(sa->sa_family == AF_INET6){
        cur_temp = ((struct sockaddr_in6*)sa);
        return &(cur_temp->sin6_addr);
    }else{
        perror("can't get ip addr of tx");
    }
}
