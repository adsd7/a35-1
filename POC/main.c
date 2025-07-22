#include "header.h"


void process_can_frame(can_processor_t *processor, can_frame_t *frame) {
    // 1. Извлечение полей из CAN ID
    uint8_t config = (frame->id >> 10) & 0x01;
    uint8_t tx_id = (frame->id >> 5) & 0x1F;
    uint8_t rx_id = frame->id & 0x1F;

    // 2. Извлечение Msg_Info
    uint8_t msg_info = (frame->data[0] >> 4) & 0x0F;

    pthread_mutex_lock(&processor->assembly_table.mutex);

    message_assembly_t *assembly = &processor->assembly_table.assemblies[tx_id];

    switch(msg_info) {
        case 0:  // Короткое сообщение
            handle_short_message(processor, assembly, frame);
            break;

        case 1:  // Первое/единственное сообщение
            handle_first_message(processor, assembly, frame);
            break;

        case 2:  // Продолжение
            handle_continuation(processor, assembly, frame);
            break;
    }

    pthread_mutex_unlock(&processor->assembly_table.mutex);
}



void check_timeouts(can_processor_t *processor) {
    uint32_t now = get_timestamp_ms();

    pthread_mutex_lock(&processor->assembly_table.mutex);

    for (int i = 0; i < 32; i++) {
        message_assembly_t *assembly = &processor->assembly_table.assemblies[i];
        
        if (assembly->data_len > 0 && 
            (now - assembly->timestamp) > processor->assembly_timeout_ms) {
            // Таймаут - сбросить сборку
            if (processor->on_error) {
                processor->on_error(i, "Assembly timeout");
            }
            // Очистка
            memset(assembly, 0, sizeof(message_assembly_t));
        }
    }
    pthread_mutex_unlock(&processor->assembly_table.mutex);
}

/ Поток чтения CAN
void* can_reader_thread(void *arg) {
    can_processor_t *processor = (can_processor_t*)arg;
    can_frame_t frame;

    while (running) {
        if (can_read(&frame) == 0) {
            process_can_frame(processor, &frame);
        }
    }
}

// Поток проверки таймаутов
void* timeout_checker_thread(void *arg) {
    can_processor_t *processor = (can_processor_t*)arg;

    while (running) {
        check_timeouts(processor);
        usleep(100000);  // Проверка каждые 100 мс
    }
}

// Поток обработки готовых сообщений (опционально)
void* message_handler_thread(void *arg) {
    // Обработка через очередь для разгрузки CAN reader
}

int main() {
    return 0;
}