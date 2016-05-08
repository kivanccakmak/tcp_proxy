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


#ifndef STREAM_H
#define STREAM_H

#include <stdlib.h>
#include <stdio.h>
#include <libconfig.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#include "file_read/file_read.h"
#include "../commons/network/network.h"
#include "../commons/logger/logger.h"
#include "../commons/argv_reader/argv_read.h"

#define PATH_MAX 2048
#define IP_CHAR_MAX 512 
#define PORT_MAX_CHAR 50

#define STREAM_ARGV_NUM 4 

#define STREAM_LOG "stream.log"

#endif
