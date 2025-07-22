#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>

typedef struct {
    uint8_t  tx_id;           // ID отправителя (приоритет модуля)
    uint8_t  msg_info;        // Тип сообщения (0, 1, 2)
    uint8_t  inc_counter;     // Ожидаемый счетчик для Msg_Info=2
    uint16_t data_len;        // Общая длина данных
    uint16_t received_len;    // Сколько уже получено
    uint8_t  data[2048];      // Буфер для сборки данных
    uint32_t timestamp;       // Время начала сборки (для таймаута)
    uint8_t  module_type;     // Тип модуля
    uint8_t  slot_number;     // Номер слота
    uint16_t command;         // Команда
} message_assembly_t;

typedef struct {
    message_assembly_t assemblies[32];  // Индекс = TX_ID
    pthread_mutex_t mutex;              // Защита для многопоточности
} assembly_table_t;

typedef struct {
    assembly_table_t assembly_table;
    
    // Callback для готовых сообщений
    void (*on_message_complete)(uint8_t tx_id, 
                               uint8_t module_type,
                               uint8_t slot_number, 
                               uint16_t command,
                               uint8_t *data, 
                               uint16_t len);
    
    // Callback для ошибок
    void (*on_error)(uint8_t tx_id, const char *error);
    
    // Настройки
    uint32_t assembly_timeout_ms;  // Таймаут сборки (например, 100 мс)
} can_processor_t;

#endif // HEADER_H