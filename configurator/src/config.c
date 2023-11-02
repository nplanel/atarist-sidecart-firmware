#include "include/config.h"

static __uint16_t is_delay_option = FALSE;

static ConfigData *configData = NULL;

ConfigEntry *get_config_entry(char *key)
{
    for (size_t i = 0; i < configData->count; i++)
    {
        if (strcmp(configData->entries[i].key, key) == 0)
        {
            return &configData->entries[i];
        }
    }
    return NULL;
}

static void print_table(ConfigData *configData)
{
    printf("+----+----------------------+------------------------------------------+-------+\r\n");
    printf("|    |       Key            |                Value                     | Type  |\r\n");
    printf("+----+----------------------+------------------------------------------+-------+\r\n");

    for (size_t i = 0; i < configData->count; i++)
    {
        char valueStr[40]; // Buffer to format the value

        switch (configData->entries[i].dataType)
        {
        case TYPE_INT:
        case TYPE_STRING:
        case TYPE_BOOL:
            snprintf(valueStr, sizeof(valueStr), "%s", configData->entries[i].value);
            break;
        default:
            snprintf(valueStr, sizeof(valueStr), "Unknown");
            break;
        }

        char *typeStr;
        switch (configData->entries[i].dataType)
        {
        case TYPE_INT:
            typeStr = "INT";
            break;
        case TYPE_STRING:
            typeStr = "STRING";
            break;
        case TYPE_BOOL:
            typeStr = "BOOL";
            break;
        default:
            typeStr = "???";
            break;
        }

        printf("| %-2d | %-20s | %-40s | %-8s |\r\n", i + 1, configData->entries[i].key, valueStr, typeStr);
    }

    printf("+----+----------------------+------------------------------------------+----------+\r\n");
}

void init_config()
{
    // Dynamically allocate memory for ConfigData
    configData = malloc(sizeof(ConfigData));
    if (configData != NULL)
    {
        configData->magic = 0;
        configData->count = 0;
    }
}

void load_all_entries()
{
    __uint16_t count = 0;
    __uint32_t currentAddress = (__uint32_t)(CONFIG_START_ADDRESS + sizeof(__uint32_t));
    // Dynamically allocate memory for ConfigData
    configData->magic = *((volatile __uint32_t *)currentAddress);
    configData->count = 0;
    currentAddress += sizeof(__uint32_t);
    while (1)
    {
        ConfigEntry entry;
        memcpy(&entry, (void *)currentAddress, sizeof(ConfigEntry));

        currentAddress += sizeof(ConfigEntry);

        // Check for the end of the config entries
        if (entry.key[0] == '\0')
        {
            break; // Exit the loop if we encounter a key length of 0 (end of entries)
        }
        configData->entries[configData->count] = entry;
        configData->count++;
    }
}

static char *read_input(__uint16_t type)
{
    char buffer[MAX_STRING_VALUE_LENGTH + 2]; // +1 for '\0' and +1 to check overflow
    char *result = (char *)malloc(MAX_STRING_VALUE_LENGTH + 1);
    int ch;
    int index = 0;

    if (!result)
    {
        printf("Memory allocation error.\r\n");
        exit(1);
    }

    // Displaying a prompt based on type
    switch (type)
    {
    case TYPE_INT:
        printf("Please enter an integer value: ");
        break;
    case TYPE_STRING:
        printf("Please enter a string (max %d characters): ", MAX_STRING_VALUE_LENGTH);
        break;
    case TYPE_BOOL:
        printf("Please enter a boolean value (true or false): ");
        break;
    }

    while (1)
    {
        // Repeat until valid input
        while (1)
        {
            ch = getchar();
            if (ch == 13)
            {
                // User pressed Enter
                break;
            }
            else if (ch == 127 || ch == '\b')
            {
                // User pressed backspace
                if (index > 0)
                {
                    // Move back one position in the buffer
                    index--;
                    // Optionally, move cursor back one position and overwrite with space
                    printf(" \b");
                    fflush(stdout);
                }
            }
            else if (ch >= ' ')
            {
                // User entered a printable character
                if (index < sizeof(buffer) - 1)
                {
                    buffer[index++] = (char)ch;
                    fflush(stdout);
                }
            }
        }
        for (int i = index; i < sizeof(buffer); i++)
        {
            buffer[i] = '\0';
        }

        if (type == TYPE_INT)
        {
            int valid = 1;
            for (size_t i = 0; buffer[i]; i++)
            {
                if (buffer[i] < '0' || buffer[i] > '9')
                {
                    valid = 0;
                    break;
                }
            }

            if (valid)
            {
                strncpy(result, buffer, MAX_STRING_VALUE_LENGTH);
                break;
            }
            else
            {
                printf("Invalid integer. Please enter a valid integer: ");
            }
        }
        else if (type == TYPE_STRING)
        {
            if (strlen(buffer) <= MAX_STRING_VALUE_LENGTH)
            {
                strncpy(result, buffer, MAX_STRING_VALUE_LENGTH);
                break;
            }
            else
            {
                printf("String too long. Please enter a string of maximum %d chars: ", MAX_STRING_VALUE_LENGTH);
            }
        }
        else if (type == TYPE_BOOL)
        {
            if (strcasecmp(buffer, "true") == 0 || strcasecmp(buffer, "false") == 0)
            {
                strncpy(result, buffer, MAX_STRING_VALUE_LENGTH);
                break;
            }
            else
            {
                printf("Invalid boolean. Please enter a boolean value (true or false): ");
            }
        }
    }

    return result;
}

__uint16_t configuration()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    printf("Loading configuration...");

    send_sync_command(GET_CONFIG, NULL, 0, 10, TRUE);

    printf("\r\n");

    load_all_entries();

    flush_kbd();

    while (1)
    {
        print_table(configData);

        char *prompt;
        asprintf(&prompt, "Enter the parameter to modify (1 to %d) or [C]ancel: ", configData->count);

        int param_index = get_number_within_range(prompt, configData->count, 1, 'C');

        if (param_index <= 0)
        {
            // Back to main menu
            return 0; // 0 means back to main menu
        }

        param_index = param_index - 1;

        printf("\r\n%s = %s\r\r\n", configData->entries[param_index].key, configData->entries[param_index].value);
        char *input = read_input(configData->entries[param_index].dataType);
        printf("\r\n");

        printf("The input is: %s\r\n", input);
        strncpy(configData->entries[param_index].value, input, MAX_STRING_VALUE_LENGTH);

        printf("Saving configuration...");

        switch (configData->entries[param_index].dataType)
        {
        case TYPE_INT:
            send_sync_command(PUT_CONFIG_INTEGER, &configData->entries[param_index], sizeof(ConfigEntry), 10, FALSE);
            break;
        case TYPE_BOOL:
            send_sync_command(PUT_CONFIG_BOOL, &configData->entries[param_index], sizeof(ConfigEntry), 10, FALSE);
            break;
        case TYPE_STRING:
            send_sync_command(PUT_CONFIG_STRING, &configData->entries[param_index], sizeof(ConfigEntry), 10, FALSE);
            break;
        default:
            printf("Unknown data type.\r\n");
            break;
        }

        send_sync_command(SAVE_CONFIG, NULL, 0, 10, TRUE);
        printf("\r\n");

        free(input);

        return 0; // 0 means back to main menu, do not reboot
    }
}

__uint16_t read_config()
{

#ifndef _DEBUG
    int err = send_sync_command(GET_CONFIG, NULL, 0, 10, FALSE);

    if (err != 0)
    {
        printf("Cannot read configuration. Is the SidecarT connected?\r\nTry to reset the SidecarT and try again.\r\n");
        return 1;
    }

    load_all_entries();

    for (size_t i = 0; i < configData->count; i++)
    {
        if (strcmp(configData->entries[i].key, "DELAY_ROM_EMULATION") == 0)
        {
            is_delay_option = strcmp(configData->entries[i].value, "true") == 0;
        }
    }
#endif
    return 0;
}

__uint16_t is_delay_option_enabled(void)
{
    return is_delay_option;
}

__uint16_t toggle_delay_option(void)
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    is_delay_option = !is_delay_option;
    ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, "DELAY_ROM_EMULATION", MAX_STRING_VALUE_LENGTH);
    strncpy(entry->value, is_delay_option ? "true" : "false", MAX_STRING_VALUE_LENGTH);
    entry->dataType = TYPE_BOOL;

    send_sync_command(PUT_CONFIG_BOOL, entry, sizeof(ConfigEntry), 10, FALSE);

    free(entry);

    flush_kbd();

    return 0; // Do not reset computer after toggling delay option
}