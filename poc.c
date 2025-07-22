#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "include/ioctl_codes.h"
#include "poc.h"

#define DEVICE_DEFAULT "/dev/can2"
#define MAX_LAST_MSG 16

uint8_t raw_data[2048];

/* "Raw" frame coming from driver */
typedef struct {
  uint32_t id;
  uint8_t len;
  uint8_t data[64];
  uint64_t hw_ts;
} canfd_raw_t;

/* Global (for demo) context */
typedef struct {
  int fd;
  atomic_bool running;
  pthread_t tid;

  pthread_mutex_t lock;
  unsigned int msg_cnt;
  can_message_t last[MAX_LAST_MSG];
} ctx_t;

static ctx_t g; /* zero‑initialised */

/* ------------------------------------------------------------------ */
static void fatal(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

/* Send one CAN‑FD frame via proprietary IOCTL_WRITE */
static void can_write(uint32_t id, const void *buf, size_t len) {
  IOCTL_WRITE_ARG a = {.Id = id, .Len = (uint8_t)len};
  if (len > sizeof(a.Data))
    fatal("CAN write len>64");
  memcpy(a.Data, buf, len);
  if (ioctl(g.fd, IOCTL_WRITE, &a) != 0)
    fatal("IOCTL_WRITE");
}

/* -------------------- message parser + app logic ------------------- */
static void handle_msg(const canfd_raw_t *raw) {
  can_message_t m;
  memcpy(&m, raw->data, sizeof(m));

  pthread_mutex_lock(&g.lock);

  g.msg_cnt++;
  g.last[g.msg_cnt % MAX_LAST_MSG] = m;

  printf("=== CAN msg #%u ===\n", g.msg_cnt);
  printf("ID=0x%03X len=%u\n", raw->id, raw->len);

  switch (m.header.msg_info) {
  case 0x01: { /* First fragment (or Single w/len) */
    uint16_t dl = ((m.type1.data_len_high & 0x0F) << 8) | m.type1.data_len_low;
    printf("FIRST: data_len=%u mt=0x%02X slot=%u cmd=0x%02X "
           "ts=0x%04X\n",
           dl, m.type1.params.module_type, m.type1.params.slot_number,
           m.type1.params.command, m.type1.params.timestamp);
    printf("data size: %lu\n", sizeof(m.type1.data));
    break;
  }
  case 0x02: /* Last fragment / single */
    printf("LAST:  mt=0x%02X slot=%u cmd=0x%02X ts=0x%04X inc_count=%u\n",
           m.type2.params.module_type, m.type2.params.slot_number,
           m.type2.params.command, m.type2.params.timestamp,
           m.type2.inc_counter);
    break;
  default:
    printf("msg_type=0x%X (unhandled)\n", m.header.msg_info);
    break;
  }

  pthread_mutex_unlock(&g.lock);
}

/* ---------------------- reader thread ------------------------------ */
static void *reader(void *arg) {
  (void)arg;
  struct pollfd pfd = {.fd = g.fd, .events = POLLIN};

  /* RT settings – ignore errors if not privileged */
  struct sched_param sp = {.sched_priority = 70};
  pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
  cpu_set_t cs;
  CPU_ZERO(&cs);
  CPU_SET(0, &cs);
  pthread_setaffinity_np(pthread_self(), sizeof(cs), &cs);

  while (atomic_load(&g.running)) {
    int pr = poll(&pfd, 1, 100);
    if (pr <= 0)
      continue; /* timeout or EINTR */

    for (;;) {
      IOCTL_READ_ARG tmp;
      int rc = ioctl(g.fd, IOCTL_READ, &tmp);
      if (rc == 0) {
        canfd_raw_t r = {.id = tmp.Id, .len = tmp.Len};
        memcpy(r.data, tmp.Data, tmp.Len);
        handle_msg(&r);
      } else if (rc == 0x101) {
        break; /* RX empty */
      } else {
        fprintf(stderr, "IOCTL_READ err=%d\n", rc);
        break;
      }
    }
  }
  return NULL;
}

/* ---------------------- graceful shutdown -------------------------- */
static void on_sig(int s) {
  (void)s;
  atomic_store(&g.running, false);
}

static void dump_stats(void) {
  pthread_mutex_lock(&g.lock);
  printf("\n=== stats ===\n"
         "frames : %u\n",
         g.msg_cnt);
  if (g.msg_cnt) {
    can_message_t *lm = &g.last[g.msg_cnt % MAX_LAST_MSG];
    printf("last cmd=0x%02X slot=%u\n", lm->type2.params.command,
           lm->type2.params.slot_number);
  }
  pthread_mutex_unlock(&g.lock);
}

/* ------------------------------ main ------------------------------- */
int main(int argc, char **argv) {
  const char *dev = (argc > 1) ? argv[1] : DEVICE_DEFAULT;

  g.fd = open(dev, O_RDWR | O_CLOEXEC | O_NONBLOCK);
  if (g.fd < 0)
    fatal("open");

  pthread_mutex_init(&g.lock, NULL);
  atomic_init(&g.running, true);

  if (pthread_create(&g.tid, NULL, reader, NULL) != 0)
    fatal("pthread_create");

  signal(SIGINT, on_sig);
  signal(SIGTERM, on_sig);

  /* ---- send GET_MODULE_INFO ---- */
  can_message_t rq = {0};
  rq.header.msg_info = 0; /* master single */
  rq.type0.params.module_type = MODULE_DOR12_MODULE;
  rq.type0.params.slot_number = 6;
  rq.type0.params.command = CMD_GET_MODULE_INFO;

  can_write(0x3F, &rq, sizeof(rq));

  puts("Press Ctrl‑C to stop …");
  while (atomic_load(&g.running))
    pause();

  pthread_join(g.tid, NULL);
  dump_stats();
  close(g.fd);
  return 0;
}
