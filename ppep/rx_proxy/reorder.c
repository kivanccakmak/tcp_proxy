#include "reorder.h"

static void add2queue(queue_t *queue, char *raw_data);

 /**
  * @brief 
  *
  * Independent thread function to push packets to queue.
  * Sleeps until any link_receptor wakes this thread up.
  * If necessary, this thread wakes queue thread up.
  *
  * @param args

  * @return 
  */
void *nudge_queue(void *args) 
{
    bool packet_exist = true;
    int index = 0;
    struct packet_pool *pool = NULL;
    queue_t *queue = NULL;
    cb_reord_args_t *reord_args = NULL; 
    reord_args = (cb_reord_args_t *) args;
    queue = reord_args->queue;
    pool = reord_args->pool;
    printf("queue: %p\n", queue);
    printf("queue->byte_capacity: %d\n", queue->byte_capacity);
    printf("queue->byte_count: %d\n", queue->byte_count);

    pthread_mutex_lock(&pool->lock);

    // wait until link receptor wakes up
    while (1) {
        printf("* reorder cond_wait() *\n");
        pthread_cond_wait(&pool->cond, &pool->lock);
        printf("* reorder get signal *\n");
        printf("* reorder mutex_lock *\n");
        pthread_mutex_lock(&queue->lock);
        printf("* reorder locked queue mutex *\n");
        index = pool->queue_seq + 1;
        printf("* index: %d\n*", index);
        packet_exist = (pool->sequential_nodes[index] != NULL);
        printf("packet_exist: %d\n", packet_exist ? 1 : 0);
        while (packet_exist) {
            printf("* towards add2queue() \n");
            add2queue(queue, pool->sequential_nodes[index]);
            pool->queue_seq++;
            printf("* passed add2queue() \n");
            index++;
            packet_exist = (pool->sequential_nodes[index] != NULL);
        }
        printf("index: %d\n", index);
        printf("| unlock(&queue->lock) |\n");
        pthread_mutex_unlock(&queue->lock);
        printf("| signal(&queue->cond) |\n");
        pthread_cond_signal(&queue->cond);
    }
    return NULL;
}

/**
 * @brief 
 *
 * Add packet to queue, if necessary increase
 * size of queue. 
 * 
 * @param[out] queue
 * @param[in] raw_data
 *
 */
void add2queue(queue_t *queue, char *raw_data) 
{
    printf("in add2queue() \n");
    int capacity_diff = 0;

    capacity_diff = (int) queue->byte_capacity 
        - (int) queue->byte_count;

    printf("| queue->index: %d |\n", queue->index);
    if (capacity_diff < 2 * (int) sizeof(char*)) 
        queue->byte_capacity = 
            2 * queue->byte_capacity + 1;
    
    queue->index += 1;
    printf("| queue->index: %d |\n", queue->index);
    queue->buffer[queue->index] = raw_data;
    queue->byte_count += sizeof(char *);
}



/**
 * @brief 
 * initialize pool struct, which keeps 
 * non sequentially ordered yet packets
 *
 * @return 
 */
struct packet_pool* packet_pool_init() 
{
    struct packet_pool *pool = (struct packet_pool*) 
        malloc(sizeof(struct packet_pool));
    pool->queue_seq = NO_SEQUENCE;
    pool->capacity = INIT_POOL_SIZE;
    pool->sequential_nodes = 
        (char **) malloc(sizeof(char*) * pool->capacity);
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond, NULL);

    return pool;
}



