#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>

#include "include/ioctl_codes.h"
#include "module.h"

char *Device;
int hDevice;
int TimeSync = 0;
int Count = 1;

uint8_t data[2048];
int data_len = 0;

// Структура для управления обработчиком CAN данных
typedef struct {
  int device_fd;
  int running;
  pthread_mutex_t data_mutex;
  int message_count;
  msg_t last_messages[10]; // Буфер для последних сообщений
} can_handler_t;

can_handler_t can_handler = {0};

// Функция обработки полученных CAN данных
void process_can_message(IOCTL_READ_ARG *data) {
  pthread_mutex_lock(&can_handler.data_mutex);

  printf("=== Received CAN message %d ===\n", ++can_handler.message_count);
  printf("ID: 0x%03X\n", data->Id);
  printf("Length: %d bytes\n", data->Len);
  msg_t rx_msg = {0};
  memcpy(&rx_msg, data->Data, sizeof(msg_t));

  // Проверяем, что это сообщение модуля
  // (rx_msg.info.msg_type == 0x01 || rx_msg.info.msg_type == 0x02)
  if (rx_msg.info.msg_type == 0x01) {
    uint8_t *data_ptr = (uint8_t *)&rx_msg;
    data_len = (data_ptr[0] & 0x0f << 8) | data_ptr[1];
    msg_param_t param = {0};
    memcpy(&param, &data_ptr[2], sizeof(msg_param_t));
    printf("data_len: %d\n", data_len);
    printf("module_type: 0x%02X\n", param.module_type);
    printf("slot_number: 0x%02X\n", param.slot_number);
    printf("command: 0x%02X\n", param.command);
    printf("timestamp: 0x%02X\n", param.timestamp);
    memcpy(data, data_ptr + 7, DATA_SIZE);
  } else if (rx_msg.info.msg_type == 0x02) {
    printf("msg_type: 0x%02X\n", rx_msg.info.msg_type);
    printf("module_type: 0x%02X\n", rx_msg.param.module_type);
    printf("slot_number: 0x%02X\n", rx_msg.param.slot_number);
    printf("command: 0x%02X\n", rx_msg.param.command);
    printf("timestamp: 0x%02X\n", rx_msg.param.timestamp);
  }

  // Сохраняем последнее сообщение в буфер
  if (can_handler.message_count <= 10) {
    memcpy(&can_handler.last_messages[can_handler.message_count - 1], &rx_msg,
           sizeof(msg_t));
  }

  // Специальная обработка в зависимости от команды
  switch (rx_msg.param.command) {
  case CMD_MODULE_INFO:
    printf("*** Module Info Response ***\n");
    break;
  case CMD_CONFIG_PARAM:
    printf("*** Config Param Response ***\n");
    break;
  case CMD_DATA_PARAM:
    printf("*** Data Param Response ***\n");
    break;
  default:
    printf("*** Unknown Command Response ***\n");
    break;
  }

  printf("================================\n");

  pthread_mutex_unlock(&can_handler.data_mutex);
}

// Поток для чтения и обработки CAN данных
void *can_reader_thread(void *arg) {
  can_handler_t *handler = (can_handler_t *)arg;
  struct pollfd fd;
  fd.fd = handler->device_fd;
  fd.events = POLLIN;

  printf("CAN reader thread started\n");

  while (handler->running) {
    int ret = poll(&fd, 1, 100); // короткий таймаут для быстрого выхода

    if (ret > 0 && (fd.revents & POLLIN)) {
      // Читаем все доступные сообщения
      while (1) {
        IOCTL_READ_ARG ReadArg = {0};
        int read_result = ioctl(handler->device_fd, IOCTL_READ, &ReadArg);

        if (read_result == 0) {
          // Успешное чтение - обрабатываем сообщение
          process_can_message(&ReadArg);
        } else if (read_result == 0x101) { // ERROR_RX_EMPTY
          // Больше данных нет
          break;
        } else if (read_result < 0) {
          printf("Read error in thread: %d\n", read_result);
          break;
        }
      }
    } else if (ret == -1) {
      printf("Poll error in thread: %d\n", ret);
      break;
    }
    // ret == 0 означает таймаут, продолжаем цикл
  }

  printf("CAN reader thread stopped\n");
  return NULL;
}

int WaitData() {
  IOCTL_WAIT_ARG WaitArg;
  int ErrorCode;

  WaitArg.TimeoutMks = 1000100; // 1s

  ErrorCode = ioctl(hDevice, IOCTL_WAIT_W, &WaitArg);
  if (ErrorCode < 0) {
    printf("Error in wait ioctl. code: %d\n", ErrorCode);
    close(hDevice);
    exit(ErrorCode);
  }
  return ErrorCode;
}

int WriteData(int id, int len, unsigned char *data) {
  IOCTL_WRITE_ARG WriteArg;
  int ErrorCode;

  WriteArg.Id = id;
  WriteArg.Len = len;
  memcpy(WriteArg.Data, data, len);

  for (int i = 0; i < Count; i++) {
    ErrorCode = ioctl(hDevice, IOCTL_WRITE, &WriteArg);
    if (ErrorCode) {
      printf("Error in write ioctl. code: %d\n", ErrorCode);
      close(hDevice);
      exit(ErrorCode);
    }
    WriteArg.Data[len - 1]++;
  }

  return ErrorCode;
}

// Функция для получения статистики обработанных сообщений
void print_message_stats() {
  pthread_mutex_lock(&can_handler.data_mutex);
  printf("\n=== Message Statistics ===\n");
  printf("Total messages received: %d\n", can_handler.message_count);

  if (can_handler.message_count > 0) {
    printf("Last message details:\n");
    msg_t *last_msg = &can_handler.last_messages[can_handler.message_count - 1];
    printf("  Module type: 0x%02X\n", last_msg->param.module_type);
    printf("  Slot number: 0x%02X\n", last_msg->param.slot_number);
    printf("  Command: 0x%02X\n", last_msg->param.command);
  }
  printf("==========================\n\n");

  pthread_mutex_unlock(&can_handler.data_mutex);
}

int main(void) {
  msg_t msg = {0};

  hDevice = open("/dev/can2", O_RDWR);
  if (hDevice < 0) {
    printf("Can't open device\n");
    return -100;
  }

  // Инициализируем обработчик CAN данных
  can_handler.device_fd = hDevice;
  can_handler.running = 1;
  can_handler.message_count = 0;
  pthread_mutex_init(&can_handler.data_mutex, NULL);

  printf("DATA_SIZE: %ld\n", DATA_SIZE);

  // Создаем поток для чтения CAN данных
  pthread_t reader_thread;
  if (pthread_create(&reader_thread, NULL, can_reader_thread, &can_handler) !=
      0) {
    printf("Failed to create CAN reader thread\n");
    close(hDevice);
    return -1;
  }

  // Даем потоку время на запуск
  usleep(100000); // 100ms

  printf("Main thread: sending module info request...\n");

  // Подготавливаем сообщение запроса
  msg.info.msg_type = 0x00;
  msg.param.module_type = MODULE_DOR12_MODULE;
  msg.param.slot_number = 0x06;
  msg.param.command = CMD_GET_MODULE_INFO;
  msg.param.timestamp = 0x00;

  // Отправляем запрос
  WriteData(0x3f, sizeof(msg), (unsigned char *)&msg);

  // Ждем завершения отправки
  WaitData();

  printf("Request sent, waiting for responses...\n");

  // Ждем некоторое время для получения ответов
  usleep(10000);

  // Выводим статистику
  print_message_stats();

  // Останавливаем поток чтения
  can_handler.running = 0;
  pthread_join(reader_thread, NULL);

  // Очищаем ресурсы
  pthread_mutex_destroy(&can_handler.data_mutex);
  close(hDevice);

  printf("Program finished\n");
  return 0;
}