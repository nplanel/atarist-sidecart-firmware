#include "include/network.h"

ConnectionStatus connection_status = DISCONNECTED;
WifiScanData *wifiScanData = NULL;
ConnectionData *connection_data = CONNECTION_STATUS_START_ADDRESS;
__uint16_t previous_connection_status = NOT_SUPPORTED; // Assuming first status is no hardware found for networking
;

static void read_networks_from_memory(char *ssids, WifiNetworkInfo networks[], __uint16_t total_size)
{
    char *current_ssid_position = ssids;

    for (__uint16_t i = 0; i < total_size; i++)
    {
        size_t ssid_len = strlen(networks[i].ssid);
        memcpy(current_ssid_position, networks[i].ssid, ssid_len);
        current_ssid_position[ssid_len] = '\0'; // Null terminate the copied string
        current_ssid_position += ssid_len + 1;  // Move to the next position, taking into account the null terminator
    }
    current_ssid_position[0] = '\0'; // Null terminate the list of ssids
}

static __uint8_t get_network_count(char *file_array)
{
    __uint8_t count = 0;
    char *current_ptr = file_array;

    while (*current_ptr)
    { // as long as we don't hit the double null terminator
        count++;

        // skip past the current filename to the next
        while (*current_ptr)
            current_ptr++;
        current_ptr++; // skip the null terminator for the current filename
    }

    return count;
}

__uint16_t get_connection_status(bool show_bar)
{
    if (connection_data != NULL)
    {
        previous_connection_status = connection_data->status;
    }
    send_command(GET_IP_DATA, NULL, (__uint16_t)0);

    locate(0, 24);
    please_wait_silent(1);
    locate(0, 24);

    if (show_bar && (connection_data->status != NOT_SUPPORTED))
    {
        char buffer[128];
        char *status_str = "Disconnected";
        switch (connection_data->status)
        {
        case DISCONNECTED:
            status_str = "Disconnected";
            break;
        case CONNECTING:
            status_str = "Connecting...";
            break;
        case CONNECTED_WIFI:
            status_str = "Wi-Fi only, no IP";
            break;
        case CONNECTED_WIFI_NO_IP:
            status_str = "Wi-Fi only, no IP";
            break;
        case CONNECTED_WIFI_IP:
            status_str = "Connected";
            break;
        case TIMEOUT_ERROR:
            status_str = "Timeout!";
            break;
        case GENERIC_ERROR:
            status_str = "Error!";
            break;
        case NO_DATA_ERROR:
            status_str = "No data!";
            break;
        case NOT_PERMITTED_ERROR:
            status_str = "Not permitted!";
            break;
        case INVALID_ARG_ERROR:
            status_str = "Invalid args!";
            break;
        case IO_ERROR:
            status_str = "IO error!";
            break;
        case BADAUTH_ERROR:
            status_str = "Bad auth!";
            break;
        case CONNECT_FAILED_ERROR:
            status_str = "Connect failed!";
            break;
        case INSUFFICIENT_RESOURCES_ERROR:
            status_str = "No resources!";
            break;
        case NOT_SUPPORTED:
            status_str = "Networking not supported!";
            break;
        }
        bool conn_state_changed = !(previous_connection_status == connection_data->status);
        if (conn_state_changed)
        {
            sprintf(buffer, "IP: %s | SSID: %s | Status: %s",
                    connection_data->ipv4_address, connection_data->ssid,
                    status_str);
            printf("\033p");
            for (int i = 0; i < (80 - strlen(buffer)) / 2; i++)
            {
                printf(" ");
            }
            printf(buffer);
            for (int i = 0; i < (80 - strlen(buffer)) / 2; i++)
            {
                printf(" ");
            }
            printf("\033q");
        }
    }
    return connection_data->status;
}

__uint8_t network_selector()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    send_command(LAUNCH_SCAN_NETWORKS, NULL, (__uint16_t)0);

    please_wait("Scanning the network...", NETWORK_WAIT_TIME);

    send_command(GET_SCANNED_NETWORKS, NULL, (__uint16_t)0);

    please_wait("\n\033KRetrieving networks...", WAIT_TIME);

    printf("\r\n");

    int num_networks = -1;
    __uint32_t network_list_mem = NETWORK_START_ADDRESS;

#ifdef _DEBUG
    printf("Reading network list from memory address: 0x%08X\r\n", network_list_mem);
#endif
    WifiScanData *wifiScanDataBuff = network_list_mem;
    char *network_array = malloc(MAX_SSID_LENGTH * wifiScanDataBuff->count + 1);
    read_networks_from_memory(network_array, wifiScanDataBuff->networks, wifiScanDataBuff->count);

    if (!network_array)
    {
        printf("No networks found!\r\n");
        printf("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }

    __int16_t network_number = display_paginated_content(network_array, get_network_count(network_array), ELEMENTS_PER_PAGE, "Networks");

    if (network_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    locate(0, 22);

    printf("\033KSelected network: %s\r\n", wifiScanDataBuff->networks[network_number - 1].ssid);
    printf("\033KPlease enter the password of the network:");

    char password[MAX_SSID_LENGTH] = {0};

    if (wifiScanDataBuff->networks[network_number - 1].auth_mode > 0)
    {
        // Read the password
        int ch;
        int index = 0;
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
                if (index < sizeof(password) - 1)
                {
                    password[index++] = (char)ch;
                    fflush(stdout);
                }
            }
        }
        for (int i = index; i < sizeof(password); i++)
        {
            password[i] = '\0';
        }
        printf("\033KPassword:%s", password);
    }

    // NOT WORKING. NEED TO FIX
    WifiNetworkAuthInfo network_auth_info;
    // Copy ssid and auth_mode from the selected network
    strcpy(network_auth_info.ssid, wifiScanDataBuff->networks[network_number - 1].ssid);
    network_auth_info.auth_mode = wifiScanDataBuff->networks[network_number - 1].auth_mode;
    strcpy(network_auth_info.password, password);

    send_command(CONNECT_NETWORK, &network_auth_info, sizeof(network_auth_info));

    printf("\r\n\033KROM network loaded. ");

    free(network_array);

    return 0; // Return 0 to avoid to force a reset
}

__uint8_t roms_from_network_selector()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    int retries = 5; // for example
    bool command_executed = false;
    int num_files = -1;
    __uint16_t *ptr;
    __uint16_t *network_file_list_mem = (__uint16_t *)NETWORK_FILE_LIST_START_ADDRESS;

    while (!command_executed && retries > 0)
    {
        send_command(GET_ROMS_JSON_FILE, NULL, (__uint16_t)0);
        please_wait("Getting ROMs list...", ROMS_JSON_WAIT_TIME);

        __uint32_t sum = 0;
        ptr = network_file_list_mem;
        for (int i = 0; i < EXCHANGE_BUFFER_SIZE / 2; i++)
        {
            sum += *ptr++;
        }

        command_executed = (sum != 0);
        retries--;
    }

    printf("\r\n");

    if (!command_executed)
    {
        printf("All values are zero. Command not executed!\r\n");
    }
    else if (retries == 0)
    {
        printf("Max retries reached without success.\r\n");
    }

#ifdef _DEBUG
    printf("Reading file list from memory address: 0x%08X\r\n", network_file_list_mem);
#endif
    char *file_array = read_files_from_memory((__uint8_t *)network_file_list_mem);

    if ((!file_array) || (!command_executed) || (retries == 0))
    {
        printf("No files found. Check if your network connection is working!\r\n");
        printf("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }
    __int16_t rom_number = display_paginated_content(file_array, get_file_count(file_array), ELEMENTS_PER_PAGE, "ROM images");

    if (rom_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    locate(0, 22);

    printf("\033KSelected the ROM file: %d. ", rom_number);

    print_file_at_index(file_array, rom_number - 1, 0);

    send_command(DOWNLOAD_ROM, &rom_number, 2);

    printf("\033KROM file downloaded. ");

    return 1; // Positive is OK
}