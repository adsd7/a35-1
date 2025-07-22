#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>

#define MODULE_HEAD_CPU_MODULE 0x00 // модуль центрального процессора
#define MODULE_POWER_UNIT_MODULE 0x01 // PSU A35-PS8401 АНПБ.436644.001
#define MODULE_DIA20_MODULE 0x04      // DIA20 A35-DI9344 АНПБ.426433.001-03
#define MODULE_DI32_MODULE 0x05       // DI32 A35-DI2480 АНПБ.426433.001-02
#define MODULE_DIU20_MODULE 0x06      // DIU20 A35-DI6344 АНПБ.426433.001
#define MODULE_DIR12_MODULE 0x07      // DIR12 A35-DI6142 АНПБ.426433.001-01
#define MODULE_AI8_MODULE 0x11        // AI8 A35-AI1710 АНПБ.426431.001
#define MODULE_DOR12_MODULE 0x14      // DOR12 A35-DO5117 АНПБ.426436.001
#define MODULE_C4V4_5_MODULE 0x18 // C4V4.5 A35-AI7255 АНПБ.426431.001-01
#define MODULE_C4V4_1_MODULE 0x19 // C4V4.1 A35-AI6255 АНПБ.426431.001-02
#define MODULE_COM4_MODULE 0x1C   // COM4

#define CMD_GET_MODULE_INFO 0x01 // запрос информации о модуле
#define CMD_MODULE_INFO 0x02    // информация о модуле
#define CMD_TIMESTAMP_SYNC 0x06 // синхронизация времени
#define CMD_TIMESTAMP_SYNC_OK 0x07 // синхронизация времени успешна
#define CMD_GET_CONFIG_PARAM 0x0D // запрос параметров конфигурации
#define CMD_CONFIG_PARAM 0x0E // параметры конфигурации
#define CMD_SET_CONFIG_MODE 0x0F // установка режима конфигурации
#define CMD_WRITE_CONFIG_PARAM 0x10 // запись параметров конфигурации
#define CMD_SET_DEFAULT_PARAM 0x11 // установка конфигурации по умолчанию
#define CMD_GET_DATA_PARAM 0x12 // запрос параметров данных
#define CMD_DATA_PARAM 0x13     // параметры данных
#define CMD_WRITE_DATA_PARAM 0x14 // запись параметров данных
#define CMD_MCU_RESET 0x20        // сброс микроконтроллера

#define MSG_T_SIZE 64
#define DATA_SIZE (MSG_T_SIZE - (sizeof(msg_info_t) + sizeof(msg_param_t)))

typedef struct __attribute__((packed)) {
  uint8_t reserved : 4;
  uint8_t msg_type : 4;
} msg_info_t;

typedef struct __attribute__((packed)) {
  uint16_t module_type : 5;
  uint16_t slot_number : 5;
  uint16_t command : 6;
  uint16_t timestamp;
} msg_param_t;

typedef struct __attribute__((packed)) {
  msg_info_t info;
  msg_param_t param;
  uint8_t data[DATA_SIZE];
} msg_t;

#endif