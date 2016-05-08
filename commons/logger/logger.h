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


#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdlib.h>
#include <errno.h>

typedef enum log_level_en {
    LL_NONE = 0,
    LL_FATAL,
    LL_ERROR,
    LL_WARN,
    LL_INFO,
    LL_DEBUG,
    LL_TRACE
} log_level_t;

#define clean_errno()                                 \
        (errno == 0 ? "None" : strerror(errno))       \

#define LOG_ASSERT(fp, ll, expr)                           \
    do {                                                   \
        if(!(expr)) {                                      \
            perror((""));                                  \
            fprintf(fp, "[%d] Assertion ("                 \
                       #expr                               \
                       ") failed"                          \
                       ", [%s] in %s:%s:%d\n",             \
                       ll,                                 \
                       clean_errno(),                      \
                       __FILE__,                           \
                       __func__,                           \
                       __LINE__);                          \
            printf("[%d] Assertion ("                      \
                       #expr                               \
                       ") failed"                          \
                       ", [%s] in %s:%s:%d\n",             \
                       ll,                                 \
                       clean_errno(),                      \
                       __FILE__,                           \
                       __func__,                           \
                       __LINE__);                          \
            exit(EXIT_FAILURE);                            \
    } } while(0);

#endif /* _LOGGER_H_ */
