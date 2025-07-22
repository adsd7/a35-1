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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "include/ioctl_codes.h"
#include "module.h"
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
  asm_ctx_t asm_tab[30]; /* контексты сборки */
} ctx_t;

static ctx_t g; /* zero‑initialised */

/* ------------------------------------------------------------------ */
static void fatal(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

/* simple CRC-32 calculation (polynomial 0xEDB88320) */
static uint32_t crc32_calc(const void *data, size_t len) {
  static uint32_t tbl[256];
  static int ready = 0;
  if (!ready) {
    for (uint32_t i = 0; i < 256; i++) {
      uint32_t r = i;
      for (int j = 0; j < 8; j++)
        r = (r >> 1) ^ (0xEDB88320u & (-(int)(r & 1)));
      tbl[i] = r;
    }
    ready = 1;
  }
  uint32_t crc = 0xFFFFFFFFu;
  const uint8_t *p = data;
  for (size_t i = 0; i < len; i++)
    crc = tbl[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
  return ~crc;
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

  uint8_t slot;
  switch (m.header.msg_info) {
  case 0:
    slot = m.type0.params.slot_number;
    break;
  case 1:
    slot = m.type1.params.slot_number;
    break;
  case 2:
    slot = m.type2.params.slot_number;
    break;
  default:
    printf("msg_type=0x%X (unhandled or no ctx)\n", m.header.msg_info);
    return;
  }

  asm_ctx_t *ac = &g.asm_tab[slot];

  if (m.header.msg_info == 1) { /* FIRST / SINGLE */
    uint16_t dlen =
        ((m.type1.data_len_high & 0x0F) << 8) | m.type1.data_len_low;
    printf("dlen: %u\n", dlen);

    /* сброс любого предыдущего незаконченного */
    ac->busy = false;

    size_t payload = sizeof(m.type1.data);
    if (payload > sizeof(ac->buf))
      payload = sizeof(ac->buf); /* перестраховка */

    memcpy(ac->buf, m.type1.data, payload);
    ac->expected = dlen;
    ac->received = payload;
    ac->inc_next = 0;
    ac->busy = true;

    if (ac->received >= ac->expected) { /* «короткий» один кадр */
      printf(">>> COMPLETE (single) %u bytes\n", ac->expected);
      /* здесь вызывайте вашу бизнес-логику… */
      ac->busy = false;
    }
  } else if (m.header.msg_info == 2 && ac->busy) { /* CONT / LAST */
    if (m.type2.inc_counter != ac->inc_next) {
      printf("inc_counter mismatch (exp %u, got %u) – reset\n", ac->inc_next,
             m.type2.inc_counter);
      ac->busy = false;
      pthread_mutex_unlock(&g.lock);
      return;
    }
    size_t hdr = 5; /* 0 + 1-4 msg_param_t */
    size_t payload = raw->len - hdr;
    if (ac->received + payload > sizeof(ac->buf))
      payload = sizeof(ac->buf) - ac->received;

    memcpy(ac->buf + ac->received, m.type2.data, payload);

    ac->received += payload;
    ac->inc_next = (ac->inc_next + 1) & 0x0F;

    if (ac->received >= ac->expected) {
      printf(">>> COMPLETE: %u bytes expected, %u received (slot %u)\n",
             ac->expected, ac->received, slot);
      /* обработать ac->buf / ac->expected … */
      ac->busy = false;
      global_module_info_t info;
      memcpy(&info, ac->buf, sizeof(info));
      uint32_t crc_received;
      memcpy(&crc_received, ac->buf + sizeof(info), sizeof(crc_received));
      printf("crc_received: %08X\n", crc_received);
      uint32_t crc_calculated = crc32_calc(ac->buf, sizeof(info));
      printf("crc_calculated: %08X\n", crc_calculated);

      printf("boot_firmware_version: %u\n",
             info.boot_info.boot_firmware_version);
      printf("hardware_version: %u\n", info.boot_info.hardware_version);
      printf("manufacture_data: %u\n", info.boot_info.manufacture_data);
      printf("module_type: %u\n", info.boot_info.module_type);
      printf("serial_number: %.*s\n",
             (int)sizeof(info.boot_info.serial_number_chars),
             info.boot_info.serial_number_chars);
      printf("firmware_ver: %u\n", info.appl_info.firmware_ver);
      printf("fw_size: %u\n", info.appl_info.fw_size);
      printf("open_crc: %08X\n", info.appl_info.open_crc);
      printf("crypt_crc: %08X\n", info.appl_info.crypt_crc);
      printf(
          "compile date: %02u.%02u.%02u %02u:%02u:%02u\n",
          info.appl_info.compile_param.day, info.appl_info.compile_param.month,
          info.appl_info.compile_param.year, info.appl_info.compile_param.hour,
          info.appl_info.compile_param.minute,
          info.appl_info.compile_param.sec);
      printf("slot_number: %u\n", info.slot_number);
      printf("boot_mode: %02x\n", info.boot_mode);
    }
  } else {
    printf("msg_type=0x%X (unhandled or no ctx)\n", m.header.msg_info);
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

  printf("sizeof(global_module_info_t): %lu\n", sizeof(global_module_info_t));

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
