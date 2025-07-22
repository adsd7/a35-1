#ifndef _POC_H
#define _POC_H

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

// Базовая структура msg_param_t, общая для всех типов сообщений
typedef struct __attribute__((packed)) {
  uint16_t module_type : 5; // Тип модуля
  uint16_t slot_number : 5; // Номер слота
  uint16_t command : 6;     // ID команды
  uint16_t timestamp; // Время постановки в очередь для отправки
} msg_param_t;

// Тип 0: Короткие запросы/ответы (данные не более 3 байт)
typedef struct __attribute__((packed)) {
  // Байт 0
  uint8_t reserved : 4; // NA (не используется)
  uint8_t msg_info : 4; // Всегда = 0 для этого типа

  // Байты 1-4: msg_param_t
  msg_param_t params;

  // Байты 5-7: Данные (до 3 байт)
  uint8_t data[3];
} can_msg_type0_t;

// Тип 1: Первое или единственное сообщение с блоком данных
typedef struct __attribute__((packed)) {
  // Байт 0
  uint8_t data_len_high : 4; // Длина поля данных старшие 4 бита
  uint8_t msg_info : 4; // Всегда = 1 для этого типа

  // Байт 1: Полная длина данных
  uint8_t data_len_low; // Длина поля данных младшие 8 бита

  // Байты 2-5: msg_param_t
  msg_param_t params;

  // Байты 6-N: Данные (максимум 58 байт для CAN FD)
  uint8_t data[58]; // Flexible array member
} can_msg_type1_t;

// Тип 2: Продолжение блока данных
typedef struct __attribute__((packed)) {
  // Байт 0
  uint8_t inc_counter : 4; // Инкрементный счетчик (0-15)
  uint8_t msg_info : 4;    // Всегда = 2 для этого типа

  // Байты 1-4: msg_param_t
  msg_param_t params;

  // Байты 5-N: Продолжение данных
  uint8_t data[59]; // Flexible array member
} can_msg_type2_t;

// Объединение для удобной работы со всеми типами
typedef union {
  struct {
    uint8_t extra : 4;
    uint8_t msg_info : 4;
  } header;
  can_msg_type0_t type0;
  can_msg_type1_t type1;
  can_msg_type2_t type2;
  uint8_t raw[64]; // Максимальный размер CAN FD фрейма
} can_message_t;

// Вспомогательные макросы для работы с типами сообщений
#define MSG_TYPE(msg) ((msg)->header.msg_info & 0x0F)
#define IS_SHORT_MSG(msg) (MSG_TYPE(msg) == 0)
#define IS_FIRST_MSG(msg) (MSG_TYPE(msg) == 1)
#define IS_CONTINUATION_MSG(msg) (MSG_TYPE(msg) == 2)

// Максимальные размеры
#define MAX_SHORT_MSG_DATA 3
#define MAX_SINGLE_MSG_DATA 57
#define MAX_TOTAL_DATA_SIZE 2048

// Структура для парсинга CAN ID
typedef struct {
  uint16_t config : 1;         // Флаг конфигурации
  uint16_t transmitter_id : 5; // ID передатчика
  uint16_t receiver_id : 5;    // ID приемника
} can_id_fields_t;

typedef struct {
  uint16_t expected; // ожидаемая общая длина (из поля data_len)
  uint16_t received; // сколько уже набрали
  uint8_t buf[MAX_TOTAL_DATA_SIZE];
  uint8_t inc_next; // ожидаемый inc_counter (0,1,2…)
  bool busy;
} asm_ctx_t;

#endif // _POC_H