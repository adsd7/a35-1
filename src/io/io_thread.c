// src/io/io_thread.c
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "a35/ringbuf.h"
#include "a35/stats.h"
#include "mcp/mcp2518fd_ioctl.h"   /* содержит IOCTL_READ и IOCTL_READ_ARG */

#define POLL_TIMEOUT_MS   100      /* «сон» при отсутствии кадров       */
#define DEV_PATH_DEFAULT  "/dev/can2"

/* --- публичный API ---------------------------------------------------- */
typedef struct io_ctx {
    int              fd;       /* дескриптор /dev/canX            */
    ringbuf_t       *rb;       /* указатель на SPSC-очередь       */
    io_stats_t      *stats;    /* общая структура счётчиков       */
    volatile bool    stop;     /* сигнал завершения потока        */
} io_ctx_t;

static void *io_thread_fn(void *arg);

int io_thread_start(pthread_t *th, ringbuf_t *rb,
                    io_stats_t *stats, const char *dev)
{
    io_ctx_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return -1;

    ctx->rb    = rb;
    ctx->stats = stats;
    ctx->fd    = open(dev ? dev : DEV_PATH_DEFAULT,
                      O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (ctx->fd < 0) {
        perror("open /dev/canX");
        free(ctx);
        return -1;
    }

    /* заблокируем память и зададим RT-приоритет —
       критично для <1 мс end-to-end */
    mlockall(MCL_CURRENT | MCL_FUTURE);
    struct sched_param sp = { .sched_priority = 80 };
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp))
        perror("setschedparam");  /* не смертельно, но логируем */

    cpu_set_t cpumask;
    CPU_ZERO(&cpumask);
    CPU_SET(0, &cpumask);                    /* I/O-поток на CPU0 */
    pthread_setaffinity_np(pthread_self(), sizeof(cpumask), &cpumask);

    return pthread_create(th, NULL, io_thread_fn, ctx);
}

/* --- «горячий» приём сообщений --------------------------------------- */
static void *io_thread_fn(void *arg)
{
    io_ctx_t *ctx = arg;
    struct pollfd pfd = { .fd = ctx->fd, .events = POLLIN };

    while (!ctx->stop) {
        int pr = poll(&pfd, 1, POLL_TIMEOUT_MS);
        if (pr < 0 && errno == EINTR)          /* сигнал — проверить ctx->stop */
            continue;
        if (pr <= 0)                           /* тайм-аут или ошибка */
            continue;

        /* один вызов ioctl == один принятый кадр */
        IOCTL_READ_ARG fr;                     /* Id, Len, Data[64] */
        if (ioctl(ctx->fd, IOCTL_READ, &fr) < 0) {
            ctx->stats->drv_errors++;          /* счётчик ошибок ioctl */
            continue;
        }

        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);

        /* подготовим компактный кадр для ringbuf */
        canfd_frame_t cf = {
            .id  = (uint32_t)fr.Id,
            .len = (uint8_t)fr.Len,
            .tsc = (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec
        };
        memcpy(cf.data, fr.Data, cf.len);

        if (!ringbuf_push(ctx->rb, &cf)) {     /* переполнение SPSC */
            ctx->stats->rb_overflow++;
            /* Опционально: drop oldest вместо newest */
        }

        /* статистика приёма */
        ctx->stats->rx_packets++;
        ctx->stats->rx_bytes += cf.len + 5;    /* 5-байт заголовок CAN-FD */
    }

    close(ctx->fd);
    free(ctx);
    return NULL;
}
