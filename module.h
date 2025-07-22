#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>

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
  uint32_t open_crc;
  uint32_t crypt_crc;
  uint8_t module_type;
  compile_param_t compile_param;
  uint8_t reserved[9]; //  x32
} appl_module_info_t;

typedef struct __attribute__((packed)) {
  // boot section
  char serial_number_chars[16];
  uint8_t module_type;       //  тип модуля.
  uint32_t manufacture_data; //  дата производства
  uint32_t hardware_version; //  версия печатной платы == 0xFFFFFFFF  железо не
                             //  поддерживает ревизию
  uint32_t boot_firmware_version; //  версия бута
  uint8_t reserved[3];            //  для выравнивания x16
} boot_module_info_t;

typedef struct __attribute__((packed)) {
  boot_module_info_t boot_info; //  информация о модуле и загрузчике
  appl_module_info_t appl_info; //  информация о основном ПО
  uint8_t slot_number;
  uint8_t boot_mode; //  == 0x55 == normal mode, != 0x55 == boot mode
} global_module_info_t;

#endif