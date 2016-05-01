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
            exit(EXIT_FAILURE);                            \
    } } while(0);

#endif /* _LOGGER_H_ */
