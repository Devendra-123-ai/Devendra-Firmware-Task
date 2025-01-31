#include <stdio.h>
#include <windows.h>

#define COM_PORT "COM4"  // Change based on your setup
#define BAUD_RATE CBR_2400
#define END '\0'

void send_data(HANDLE hSerial, const char *data) {
    DWORD bytes_written;
    char end_char = END;
    WriteFile(hSerial, data, strlen(data), &bytes_written, NULL);
    WriteFile(hSerial, &end_char, 1, &bytes_written, NULL);
    printf("Sent: %s\n", data);
}

void receive_data(HANDLE hSerial) {
    char receivedChar;
    DWORD bytes_read;
    printf("Received: ");
    
    while (1) {
        ReadFile(hSerial, &receivedChar, 1, &bytes_read, NULL);
        if (bytes_read > 0) {
            if (receivedChar == END)
                break;
            printf("%c", receivedChar);
        }
    }
    printf("\n");
}

int main() {
    HANDLE hSerial = CreateFile(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening COM port!\n");
        return 1;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    dcbSerialParams.BaudRate = BAUD_RATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    char input[100];
    while (1) {
        printf("Enter text to send (or 'exit' to quit): ");
        gets(input);
        
        if (strcmp(input, "exit") == 0) {
            break;
        }

        send_data(hSerial, input);
        receive_data(hSerial);
    }

    CloseHandle(hSerial);
    printf("Serial connection closed.\n");
    return 0;
}
