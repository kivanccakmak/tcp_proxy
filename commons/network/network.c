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
