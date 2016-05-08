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


#include "stream.h"

static struct option long_options[] = {
    {"dest_ip", required_argument, NULL, 'A'},
    {"dest_port", required_argument, NULL, 'B'},
    {"file_name", required_argument, NULL, 'C'}
};

static void stream(
                   char *ip_addr, 
                   char *port, 
                   FILE *fp
                  );

static FILE *log_fp; /* error logger fp */

#ifdef STREAM
int main(int argc, char *argv[]) 
{
    const char *ip_addr, *port, *fname; /*transmit local fname
                                        file to ip_addr:port */
    int i = 0, ret;
    FILE *fp;                           /* to read file */

    log_fp = fopen(STREAM_LOG, "w");
    if (log_fp == NULL) {
        perror("fopen: ");
        exit(1);
    }

    arg_val_t **arg_vals = init_arg_vals(
            (int) STREAM_ARGV_NUM-1, long_options);

    if (argc == STREAM_ARGV_NUM) { // running via command argvs
        ret = argv_reader(arg_vals,
                long_options, argv, (int) STREAM_ARGV_NUM);
        LOG_ASSERT(log_fp, LL_ERROR, ret==0);
    } else if (argc == 1) {       // running via config file
        char *config_file = "../network.conf";
        config_t cfg;
        config_setting_t *setting;
        config_init(&cfg);
        if (!config_read_file(&cfg, config_file)) {
            printf("\n%s:%d - %s", config_error_file(&cfg), 
                    config_error_line(&cfg), config_error_text(&cfg));
            config_destroy(&cfg);
            return -1;
        }
        setting = config_lookup(&cfg, "stream");
        if (setting != NULL) {
            ret = config_reader(arg_vals, setting, STREAM_ARGV_NUM-1);
            LOG_ASSERT(log_fp, LL_ERROR, ret==0);
        }
    } else {printf("wrong usage\n"); return -1; }

    ip_addr = get_argv((char *) "dest_ip", arg_vals, STREAM_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, ip_addr!=NULL);
    port = get_argv((char *) "dest_port", arg_vals, STREAM_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, port!=NULL);
    fname = get_argv((char *) "file_name", arg_vals, STREAM_ARGV_NUM-1);
    LOG_ASSERT(log_fp, LL_ERROR, fname!=NULL);

    printf("dest -> %s:%s, file_name:%s\n", ip_addr, port, fname);

    fp = fopen(fname, "r");
    LOG_ASSERT(log_fp, LL_ERROR, fp!=NULL);

    stream((char*) ip_addr, (char*) port, fp);
}
#endif

/**
 * @brief read file and stream 
 * to ip:port node
 *
 * @param[in] ip_addr
 * @param[in] port
 * @param[in] fp
 */
static void stream(
                   char *ip_addr, 
                   char *port, 
                   FILE *fp
                  )
{ 
    int ret, sockfd;
    int byte_count = 0, temp_count = 0, capacity = 0;
    int file_size = 0, amount = 0, residue = 0;
    char *thr_buff;

    //Initialize sockaddr_in struct to connect to dest
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(ip_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(port) );

    printf("socket initializing ...\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    LOG_ASSERT(log_fp, LL_ERROR, sockfd!=-1);

    printf("connecting ...\n");
    ret = connect(sockfd, (struct sockaddr*)&server,
            sizeof(server));
    LOG_ASSERT(log_fp, LL_ERROR, ret!=-1);

    thr_buff = (char *) malloc(BLOCKSIZE);
    file_size = get_file_size(fp);

    while (byte_count < file_size) {
        if (byte_count + BLOCKSIZE < file_size) {
            read_file(fp, byte_count, byte_count + BLOCKSIZE, thr_buff);
            capacity = BLOCKSIZE;
        } else {
            residue = file_size - byte_count;
            read_file(fp, byte_count, byte_count + residue, thr_buff);
            capacity = residue;
        }
        while (temp_count < capacity) {
            amount = write(sockfd, thr_buff+temp_count, 
                    capacity-temp_count); 
            if (amount > 0) {
                temp_count += amount;
            }
        }
        byte_count += temp_count;
        temp_count = 0;
    }
} 
