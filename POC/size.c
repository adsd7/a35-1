#include <stdint.h>
#include <stdio.h>

#include "module.h"

/**
 * @brief Выводит структуру compile_param_t
 */
void print_compile_param_structure(void) {
  printf("compile_param_t structure:\n");
  printf("  uint8_t day;     // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t month;   // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t year;    // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t hour;    // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t minute;  // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t sec;     // %lu bytes\n", sizeof(uint8_t));
  printf("  Total size: %lu bytes\n", sizeof(compile_param_t));
  printf("\n");
}

/**
 * @brief Выводит структуру appl_module_info_t
 */
void print_appl_module_info_structure(void) {
  printf("appl_module_info_t structure:\n");
  printf("  uint32_t firmware_ver;    // %lu bytes\n", sizeof(uint32_t));
  printf("  uint32_t fw_size;         // %lu bytes\n", sizeof(uint32_t));
  printf("  uint32_t open_crc;        // %lu bytes\n", sizeof(uint32_t));
  printf("  uint32_t crypt_crc;       // %lu bytes\n", sizeof(uint32_t));
  printf("  uint8_t module_type;      // %lu bytes\n", sizeof(uint8_t));
  printf("  compile_param_t compile_param; // %lu bytes\n", sizeof(compile_param_t));
  printf("  uint8_t reserved[9];      // %lu bytes\n", sizeof(uint8_t) * 9);
  printf("  Total size: %lu bytes\n", sizeof(appl_module_info_t));
  printf("\n");
}

/**
 * @brief Выводит структуру boot_module_info_t
 */
void print_boot_module_info_structure(void) {
  printf("boot_module_info_t structure:\n");
  printf("  char serial_number_chars[16];     // %lu bytes\n", sizeof(char) * 16);
  printf("  uint8_t module_type;              // %lu bytes\n", sizeof(uint8_t));
  printf("  uint32_t manufacture_data;        // %lu bytes\n", sizeof(uint32_t));
  printf("  uint32_t hardware_version;        // %lu bytes\n", sizeof(uint32_t));
  printf("  uint32_t boot_firmware_version;   // %lu bytes\n", sizeof(uint32_t));
  printf("  uint8_t reserved[3];              // %lu bytes\n", sizeof(uint8_t) * 3);
  printf("  Total size: %lu bytes\n", sizeof(boot_module_info_t));
  printf("\n");
}

/**
 * @brief Выводит структуру global_module_info_t
 */
void print_global_module_info_structure(void) {
  printf("global_module_info_t structure:\n");
  printf("  boot_module_info_t boot_info; // %lu bytes\n", sizeof(boot_module_info_t));
  printf("  appl_module_info_t appl_info; // %lu bytes\n", sizeof(appl_module_info_t));
  printf("  uint8_t slot_number;          // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t boot_mode;            // %lu bytes\n", sizeof(uint8_t));
  printf("  Total size: %lu bytes\n", sizeof(global_module_info_t));
  printf("\n");
}

/**
 * @brief Выводит структуру module_config_param_t
 */
void print_module_config_param_structure(void) {
  printf("module_config_param_t structure:\n");
  printf("  uint8_t module_type;     // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t module_mode;     // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t module_priority; // %lu bytes\n", sizeof(uint8_t));
  printf("  uint8_t can_num;         // %lu bytes\n", sizeof(uint8_t));
  printf("  Total size: %lu bytes\n", sizeof(module_config_param_t));
  printf("\n");
}

/**
 * @brief Выводит структуру psu_module_config_t
 */
void print_psu_module_config_structure(void) {
  printf("psu_module_config_t structure:\n");
  printf("  module_config_param_t param; // %lu bytes\n", sizeof(module_config_param_t));
  printf("  uint8_t mode;                // %lu bytes\n", sizeof(uint8_t));
  printf("  uint32_t interval;           // %lu bytes\n", sizeof(uint32_t));
  printf("  Total size: %lu bytes\n", sizeof(psu_module_config_t));
  printf("\n");
}

/**
 * @brief Выводит структуру desc_input_param_t
 */
void print_desc_input_param_structure(void) {
  printf("desc_input_param_t structure:\n");
  printf("  uint8_t antibounce_time : 6;  // 6 bits\n");
  printf("  uint8_t in_active_level : 1;  // 1 bit\n");
  printf("  uint8_t in_survey_enable : 1; // 1 bit\n");
  printf("  Total size: %lu bytes\n", sizeof(desc_input_param_t));
  printf("\n");
}

/**
 * @brief Выводит структуру di_module_config_t
 */
void print_di_module_config_structure(void) {
  printf("di_module_config_t structure:\n");
  printf("  module_config_param_t param;           // %lu bytes\n", sizeof(module_config_param_t));
  printf("  uint8_t mode;                          // %lu bytes\n", sizeof(uint8_t));
  printf("  uint32_t interval;                     // %lu bytes\n", sizeof(uint32_t));
  printf("  uint16_t send_delay;                   // %lu bytes\n", sizeof(uint16_t));
  printf("  desc_input_param_t input_param[32];    // %lu bytes\n",
         sizeof(desc_input_param_t) * 32);
  printf("  Total size: %lu bytes\n", sizeof(di_module_config_t));
  printf("\n");
}

/**
 * @brief Выводит структуру do_chanel_param_t
 */
void print_do_chanel_param_structure(void) {
  printf("do_chanel_param_t structure:\n");
  printf("  uint8_t work_enable : 1;   // 1 bit\n");
  printf("  uint8_t off_time_out : 7;  // 7 bits\n");
  printf("  Total size: %lu bytes\n", sizeof(do_chanel_param_t));
  printf("\n");
}

/**
 * @brief Выводит структуру do_module_config_t
 */
void print_do_module_config_structure(void) {
  printf("do_module_config_t structure:\n");
  printf("  module_config_param_t param;                    // %lu bytes\n",
         sizeof(module_config_param_t));
  printf("  do_chanel_param_t output_param[MAX_DOR12_OUTPUTS]; // %lu bytes\n",
         sizeof(do_chanel_param_t) * MAX_DOR12_OUTPUTS);
  printf("  Total size: %lu bytes\n", sizeof(do_module_config_t));
  printf("\n");
}

/**
 * @brief Выводит структуру rs485_config_t
 */
void print_rs485_config_structure(void) {
  printf("rs485_config_t structure:\n");
  printf("  uint32_t baudrate  : 26;  // 26 bits\n");
  printf("  uint32_t data_bits : 1;   // 1 bit\n");
  printf("  uint32_t stop_bits : 2;   // 2 bits\n");
  printf("  uint32_t party : 2;       // 2 bits\n");
  printf("  uint32_t enable : 1;      // 1 bit\n");
  printf("  Total size: %lu bytes\n", sizeof(rs485_config_t));
  printf("\n");
}

/**
 * @brief Выводит структуру com4_module_config_t
 */
void print_com4_module_config_structure(void) {
  printf("com4_module_config_t structure:\n");
  printf("  module_config_param_t param;        // %lu bytes\n", sizeof(module_config_param_t));
  printf("  uint8_t mode;                       // %lu bytes\n", sizeof(uint8_t));
  printf("  rs485_config_t rs485_interface[4];  // %lu bytes\n", sizeof(rs485_config_t) * 4);
  printf("  Total size: %lu bytes\n", sizeof(com4_module_config_t));
  printf("\n");
}

/**
 * @brief Выводит структуру ai_chanel_param_t
 */
void print_ai_chanel_param_structure(void) {
  printf("ai_chanel_param_t structure:\n");
  printf("  uint8_t ch_data_out_type : 6; // 6 bits\n");
  printf("  uint8_t in_survey_enable : 1; // 1 bit\n");
  printf("  uint8_t na : 1;               // 1 bit\n");
  printf("  uint8_t value_range;          // %lu bytes\n", sizeof(uint8_t));
  printf("  float min_value;              // %lu bytes\n", sizeof(float));
  printf("  float max_value;              // %lu bytes\n", sizeof(float));
  printf("  Total size: %lu bytes\n", sizeof(ai_chanel_param_t));
  printf("\n");
}

/**
 * @brief Выводит структуру ai_module_config_t
 */
void print_ai_module_config_structure(void) {
  printf("ai_module_config_t structure:\n");
  printf("  module_config_param_t param;              // %lu bytes\n",
         sizeof(module_config_param_t));
  printf("  uint8_t mode;                             // %lu bytes\n", sizeof(uint8_t));
  printf("  uint32_t interval;                        // %lu bytes\n", sizeof(uint32_t));
  printf("  uint16_t send_delay;                      // %lu bytes\n", sizeof(uint16_t));
  printf("  ai_chanel_param_t ai_chanel_param[MAX_AI8_INPUTS]; // %lu bytes\n",
         sizeof(ai_chanel_param_t) * MAX_AI8_INPUTS);
  printf("  Total size: %lu bytes\n", sizeof(ai_module_config_t));
  printf("\n");
}

/**
 * @brief Выводит структуру cv_module_config_t
 */
void print_cv_module_config_structure(void) {
  printf("cv_module_config_t structure:\n");
  printf("  module_config_param_t param; // %lu bytes\n", sizeof(module_config_param_t));
  printf("  uint8_t mode;                // %lu bytes\n", sizeof(uint8_t));
  printf("  uint32_t interval;           // %lu bytes\n", sizeof(uint32_t));
  printf("  uint16_t send_delay;         // %lu bytes\n", sizeof(uint16_t));
  printf("  Total size: %lu bytes\n", sizeof(cv_module_config_t));
  printf("\n");
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  printf("=== Structure Sizes ===\n");
  printf("sizeof(global_module_info_t): %lu\n", sizeof(global_module_info_t));
  printf("sizeof(module_config_param_t): %lu\n", sizeof(module_config_param_t));
  printf("sizeof(di_module_config_t): %lu\n", sizeof(di_module_config_t));
  printf("sizeof(psu_module_config_t): %lu\n", sizeof(psu_module_config_t));
  printf("sizeof(do_module_config_t): %lu\n", sizeof(do_module_config_t));
  printf("sizeof(com4_module_config_t): %lu\n", sizeof(com4_module_config_t));
  printf("sizeof(ai_module_config_t): %lu\n", sizeof(ai_module_config_t));
  printf("sizeof(cv_module_config_t): %lu\n", sizeof(cv_module_config_t));
  printf("\n");

  //   printf("=== Structure Details ===\n");
  //   print_compile_param_structure();
  //   print_appl_module_info_structure();
  //   print_boot_module_info_structure();
  //   print_global_module_info_structure();
  //   print_module_config_param_structure();
  //   print_psu_module_config_structure();
  //   print_desc_input_param_structure();
  //   print_di_module_config_structure();
  //   print_do_chanel_param_structure();
  //   print_do_module_config_structure();
  //   print_rs485_config_structure();
  //   print_com4_module_config_structure();
  //   print_ai_chanel_param_structure();
  //   print_ai_module_config_structure();
  //   print_cv_module_config_structure();

  return 0;
}