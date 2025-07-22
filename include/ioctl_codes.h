#ifndef _IO_CTL_CODES
#define _IO_CTL_CODES

//#include "global.h"

#include <time.h>
#define MSP_MAX_DATA 64

#define IOCTL_CODE 10

struct can_stat {
  int rx_count;
  int rx_bytes;
  int rx_errors;
  int rx_overflow;
  int tx_count;
  int tx_bytes;
  int tx_errors;
  int tx_dropped;
};

struct can_debug {
  unsigned int citrec_reg;
  unsigned int cidiag0_reg;
  unsigned int cidiag1_reg;
};

//Структуры для передачи в IOCTL
typedef struct _IOCTL_TEST_ARG {
  unsigned short val;
} IOCTL_TEST_ARG;
#define IOCTL_TEST _IOWR(IOCTL_CODE, 0, IOCTL_TEST_ARG *)

typedef struct _IOCTL_WAIT_ARG {
  unsigned int TimeoutMks; // mks
} IOCTL_WAIT_ARG;
#define IOCTL_WAIT_R _IOWR(IOCTL_CODE, 1, IOCTL_WAIT_ARG *)
#define IOCTL_WAIT_W _IOWR(IOCTL_CODE, 4, IOCTL_WAIT_ARG *)

typedef struct _IOCTL_READ_ARG {
  int Id;
  int Len;
  unsigned char Data[MSP_MAX_DATA];
} IOCTL_READ_ARG;
#define IOCTL_READ _IOW(IOCTL_CODE, 2, IOCTL_READ_ARG *)

typedef struct _IOCTL_WRITE_ARG {
  int Id;
  int Len;
  unsigned char Data[MSP_MAX_DATA];
} IOCTL_WRITE_ARG;
#define IOCTL_WRITE _IOR(IOCTL_CODE, 3, IOCTL_WRITE_ARG *)
#define IOCTL_WRITE_TIMESTAMP _IOR(IOCTL_CODE, 5, IOCTL_WRITE_ARG *)

typedef struct _IOCTL_GET_STATS_ARG {
  struct can_stat stats;
} IOCTL_GET_STATS_ARG;
#define IOCTL_GET_STATS _IOW(IOCTL_CODE, 10, IOCTL_GET_STATS_ARG *)

typedef struct _IOCTL_GET_DEBUG_ARG {
  struct can_debug debug;
} IOCTL_GET_DEBUG_ARG;
#define IOCTL_GET_DEBUG _IOW(IOCTL_CODE, 11, IOCTL_GET_DEBUG_ARG *)

typedef struct _IOCTL_GET_WRITE_TIMESTAMP_ARG {
  struct timespec Time;
} IOCTL_GET_WRITE_TIMESTAMP_ARG;
#define IOCTL_GET_WRITE_TIMESTAMP                                              \
  _IOW(IOCTL_CODE, 6, IOCTL_GET_WRITE_TIMESTAMP_ARG *)

#endif //_IO_CTL_CODES
