#ifndef HELPER_H_
#define HELPER_H_

#include <sys/types.h>
#include <stdio.h>
#include <osbind.h>

#include "config.h"

#define EXCHANGE_BUFFER_SIZE 4096
#ifdef _DEBUG
static char file_list_example[] = {
    'd', 'o', 'c', 'u', 'm', 'e', 'n', 't', '.', 't', 'x', 't', 0,
    'i', 'm', 'a', 'g', 'e', '.', 'j', 'p', 'g', 0,
    'n', 'o', 't', 'e', 's', '.', 'd', 'o', 'c', 'x', 0,
    'm', 'u', 's', 'i', 'c', '.', 'm', 'p', '3', 0,
    'p', 'r', 'e', 's', 'e', 'n', 't', 'a', 't', 'i', 'o', 'n', '.', 'p', 'p', 't', 'x', 0,
    's', 'p', 'r', 'e', 'a', 'd', 's', 'h', 'e', 'e', 't', '.', 'x', 'l', 's', 'x', 0,
    0, 0, 0, 0, 0, 0 // Two consecutive zeroes at the end
};

static char network_file_list_example[] = {
    'D', 'o', 'c', 'u', 'm', 'e', 'n', 't', ' ', '1', 0,
    'i', 'm', 'a', 'g', 'e', 'F', 'i', 'l', 'e', 0,
    'N', 'o', 't', 'e', 's', ' ', 'A', 'B', 'C', 0,
    'M', 'y', ' ', 'M', 'u', 's', 'i', 'c', 0,
    'P', 'o', 'w', 'e', 'r', ' ', 'P', 'r', 'e', 's', 'e', 'n', 't', 'a', 't', 'i', 'o', 'n', 0,
    'E', 'x', 'c', 'e', 'l', ' ', 'S', 'h', 'e', 'e', 't', 0,
    0, 0 // A double zero at the end to indicate the end of the list
};

static __uint16_t protocol_example[] = {
    0x1234,
    0x5678,
    0x9ABC,
    0xDEF0,
};
#define ROM4_MEMORY_START &protocol_example[0]
#define ROM3_MEMORY_START &protocol_example[0]
#define FILE_LIST_START_ADDRESS &file_list_example[0]
#define NETWORK_FILE_LIST_START_ADDRESS &network_file_list_example[0]
#define CONFIG_START_ADDRESS &config_data_example
#define NETWORK_START_ADDRESS &wifi_scan_data_example
#define CONNECTION_STATUS_START_ADDRESS &connection_data_example
#define PROTOCOL_HEADER 0x0000
#define WAIT_TIME 0
#define NETWORK_WAIT_TIME 2
#define ROMS_JSON_WAIT_TIME 2
#define ELEMENTS_PER_PAGE 10
#else
#define ROM4_MEMORY_START 0xFA0000
#define ROM3_MEMORY_START 0xFB0000
#define FILE_LIST_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define NETWORK_FILE_LIST_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define CONFIG_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define NETWORK_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define CONNECTION_STATUS_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define PROTOCOL_HEADER 0xABCD
#define WAIT_TIME 2
#define NETWORK_WAIT_TIME 10
#define ROMS_JSON_WAIT_TIME 5
#define ELEMENTS_PER_PAGE 17
#endif

#define KEY_UP_ARROW 0x480000
#define KEY_DOWN_ARROW 0x500000
#define KEY_LEFT_ARROW 0x4B0000
#define KEY_RIGHT_ARROW 0x4D0000
#define KEY_ENTER 0x1C000D
#define KEY_ESC 0x1001B

#define PRINT_APP_HEADER(version)                                                                                       \
    do                                                                                                                  \
    {                                                                                                                   \
        clearHome();                                                                                                    \
        locate(0, 0);                                                                                                   \
        printf("\033pATARI ST SIDECART CONFIGURATOR. V%s - (C)2023 Diego Parrilla / @sidecartridge\033q\r\n", version); \
    } while (0)

int get_number_within_range(char *prompt, __uint8_t num_items, __uint8_t first_value, char cancel_char, char save_char);
int send_command(__uint16_t command, void *payload, __uint16_t payload_size);
void please_wait(char *message, __uint8_t seconds);
void please_wait_silent(__uint8_t seconds);
void sleep_seconds(__uint8_t seconds, bool silent);
void spinner(__uint16_t spinner_update_frequency);
char *read_files_from_memory(__uint8_t *memory_location);
__uint8_t get_file_count(char *file_array);
char *print_file_at_index(char *current_ptr, __uint8_t index, int num_columns);
int display_paginated_content(char *file_array, int num_files, int page_size, char *item_name);
void print_centered(const char *str, int screen_width);

#endif /* HELPER_H_ */