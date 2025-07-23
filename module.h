#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>

//------------------------------------------------------------------------------------------------
// Глобальная информация о модуле
//------------------------------------------------------------------------------------------------

typedef struct __attribute__((packed)) {
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t hour;
  uint8_t minute;
  uint8_t sec;
} compile_param_t;

typedef struct __attribute__((packed)) {
  // application section
  uint32_t firmware_ver; //  версия ПО
  uint32_t fw_size; //  следующие поля необходимы для обновления ПО
  uint32_t        open_crc;
  uint32_t        crypt_crc;
  uint8_t         module_type;
  compile_param_t compile_param;
  uint8_t         reserved[9]; //  x32
} appl_module_info_t;

typedef struct __attribute__((packed)) {
  // boot section
  char     serial_number_chars[16];
  uint8_t  module_type;      //  тип модуля.
  uint32_t manufacture_data; //  дата производства
  uint32_t hardware_version; //  версия печатной платы == 0xFFFFFFFF  железо не
                             //  поддерживает ревизию
  uint32_t boot_firmware_version; //  версия бута
  uint8_t  reserved[3];           //  для выравнивания x16
} boot_module_info_t;

typedef struct __attribute__((packed)) {
  boot_module_info_t boot_info; //  информация о модуле и загрузчике
  appl_module_info_t appl_info; //  информация о основном ПО
  uint8_t            slot_number;
  uint8_t            boot_mode; //  == 0x55 == normal mode, != 0x55 == boot mode
} global_module_info_t;

//------------------------------------------------------------------------------------------------
// Конфигурация модулей
//------------------------------------------------------------------------------------------------

typedef struct __attribute__((packed)) {
  uint8_t module_type;     //  тип модуля
  uint8_t module_mode;     //  режим работы модуля
  uint8_t module_priority; //  аппаратный приоритет модуля
  uint8_t can_num;         //  номер интерфейса
} module_config_param_t;

//------------------------------------------------------------------------------------------------
//  Модуль питания POWER_UNIT_MODULE
//------------------------------------------------------------------------------------------------

typedef struct __attribute__((packed)) {
  module_config_param_t param; //  обязательные параметры конфигурации
  uint8_t mode;                //  режим отправки данных модуля.
                //  MODE_LEVEL_CHANGE(0x00) - отклонение напряжений более 5%
                //  или изменилось сотояние реле или светодиодов
                //  MODE_POLLING(0x01)      - только по запросу
                //  MODE_SILENCE(0x02)      - режим тишины
                //  MODE_CYCLIC(0x03)       - отправка данных с заданым интервалом
  uint32_t interval; //  интервал в цикличном режиме в мс.
} psu_module_config_t;

//------------------------------------------------------------------------------------------------
//  Модули дискретных входов (DIA20, DI32, DIU20, DIR12).
//------------------------------------------------------------------------------------------------

typedef struct __attribute__((packed)) {
  uint8_t antibounce_time : 6; //  Время антидребезга, 63мс максимум.
  uint8_t in_active_level : 1; //  Активный уровень по входу, 1 -  высокий
                               //  уровень(по умолчанию), 0 – низкий уровень.
  uint8_t in_survey_enable : 1; //  Разрешение опроса входа. 1 – опрос разрешен
                                //  (по умолчанию), 0 – запрещен.
} desc_input_param_t;

typedef struct __attribute__((packed)) {
  module_config_param_t param; //
  uint8_t mode; //  режим отправки состояния входов модуля в ЦПУ.
  uint32_t interval; //  интервал в цикличном режиме, в мс.
  uint16_t send_delay; //  пауза перед отправкой, val * 100us. 0 – (по умолчанию).
  desc_input_param_t input_param[32]; //  настройки входов
} di_module_config_t;

//------------------------------------------------------------------------------------------------
//  Модуль дискретного вывода (DOR12)
//------------------------------------------------------------------------------------------------
#define MAX_DOR12_OUTPUTS 12

//  Структура конфигурации одного выхода
typedef struct __attribute__((packed)) {
  uint8_t work_enable : 1;  // Разрешение использования выхода.
  uint8_t off_time_out : 7; // таймаут на деактивацию выхода в мс, 0 – 127.  0 –по умолчанию.
} do_chanel_param_t;

//  Структура конфигурации модуля
typedef struct __attribute__((packed)) {
  module_config_param_t param;
  do_chanel_param_t     output_param[MAX_DOR12_OUTPUTS];
} do_module_config_t;

//------------------------------------------------------------------------------------------------
//  Модуль последовательных интерфейсов (COM4).
//------------------------------------------------------------------------------------------------

//  Структура конфигурации одного интерфейса
typedef struct __attribute__((packed)) {
  uint32_t baudrate  : 26;
  uint32_t data_bits : 1; // 0 – 8 бит, 1 – 9 бит.
  uint32_t stop_bits : 2; // 0 – 1 стоп бит, 1 – 0,5 стоп бит, 2 – 2 стоп бит, 3 – 1,5 стоп бит.
  uint32_t party : 2;     // 0 – нет контроля, 2 – четно, 3 – нечетно
  uint32_t enable : 1; // 0 – интерфейс выключен, 1 – интерфейс включен
} rs485_config_t;

//  Структура конфигурации модуля
typedef struct __attribute__((packed)) {
  module_config_param_t param;
  uint8_t mode; //  режим отправки принятых данных в ЦПУ .
                //  MODE_COM4_ECHO(0x04) - мгновенная отаправка приянтых данных
                //  MODE_WAKE(0x03)    - по приходу данных отправка информации о приемном буфере
                //  интерфейса MODE_RX_DATA(0x00) - отправка буфера по приему MODE_POLLING(0x01) -
                //  данные только по запросу

  rs485_config_t rs485_interface[4];
} com4_module_config_t;

//------------------------------------------------------------------------------------------------
//  Модуль аналогового ввода (AI8).
//------------------------------------------------------------------------------------------------
#define MAX_AI8_INPUTS 8

//  конфигурация одного канала
typedef struct __attribute__((packed)) {
  uint8_t ch_data_out_type : 6; //  режим работы канала.
  uint8_t in_survey_enable : 1; //  Разрешение опроса входа. 1 – опрос разрешен (по умолчанию), 0 –
                                //  запрещен.
  //  sw
  uint8_t na : 1;

  uint8_t value_range;
  float   min_value;
  float   max_value;
} ai_chanel_param_t;

//  структура конфигурации модуля
typedef struct __attribute__((packed)) {
  module_config_param_t param; //  обязательные параметры конфигурации
  uint8_t mode; //  режим отправки состояния входов модуля в ЦПУ .
  uint32_t interval; //  интервал в цикличном режиме в мс.
  uint16_t send_delay; //  пауза перед отправкой, val * 100us. 0 – (по умолчанию).
  ai_chanel_param_t ai_chanel_param[MAX_AI8_INPUTS];
} ai_module_config_t;

//------------------------------------------------------------------------------------------------
//  Модули телеизмерений (C4V4.5, C4V4.1).
//------------------------------------------------------------------------------------------------

//  Структура конфигурации модуля:
typedef struct __attribute__((packed)) {
  module_config_param_t param;
  uint8_t mode; //  режим отправки состояния входов модуля в ЦПУ .
  uint32_t interval; //  интервал в цикличном режиме в мс.
  uint16_t send_delay; //  пауза перед отправкой, val * 100us. 0 – (по умолчанию).
} cv_module_config_t;

//------------------------------------------------------------------------------------------------
//  Объединение конфигураций модулей
//------------------------------------------------------------------------------------------------

typedef union {
  uint8_t byte[256 - 4 - 2 - 4 - 4]; // 2byte - start flag, 4byte - write_counter - reset counter
  di_module_config_t    di_config;
  psu_module_config_t   psu_module_config;
  module_config_param_t config_param;
  do_module_config_t    dor12_config;
  com4_module_config_t  com4_config;
  ai_module_config_t    ai_config;
  cv_module_config_t    cv_config;
} config_u;

#endif