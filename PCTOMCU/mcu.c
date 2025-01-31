#include <mega32.h>
#include <delay.h>

// Define constants
#define BAUD_RATE 2400
#define EEPROM_SIZE 1024
#define END_MARKER '\n'

unsigned int eeprom_address = 0;

// Initialize UART
void UART_init(void) {
    // USART initialization
    // Communication Parameters: 8 Data bits, 1 Stop bit, No Parity
    // USART Receiver: On
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud Rate: 2400
    UCSRA=0x00;
    UCSRB=0x18;
    UCSRC=0x86;
    UBRRH=0x00;
    UBRRL=0xCF;  // For 2400 baud at 16MHz
}

// Send a single byte via UART
void UART_transmitByte(unsigned char data) {
    while(!(UCSRA & (1<<UDRE))); // Wait until transmit buffer is empty
    UDR = data;
}

// Send a string via UART
void UART_transmitString(char* str) {
    while(*str) {
        UART_transmitByte(*str++);
    }
}

// Receive a single byte via UART
unsigned char UART_receiveByte(void) {
    while(!(UCSRA & (1<<RXC))); // Wait until data is received
    return UDR;
}

// Store byte in EEPROM
void EEPROM_writeByte(unsigned int address, unsigned char data) {
    while(EECR & (1<<EEWE)); // Wait for completion of previous write
    EEAR = address;
    EEDR = data;
    EECR |= (1<<EEMWE);
    EECR |= (1<<EEWE);
    delay_ms(5);
}

// Read byte from EEPROM
unsigned char EEPROM_readByte(unsigned int address) {
    while(EECR & (1<<EEWE)); // Wait for completion of previous write
    EEAR = address;
    EECR |= (1<<EERE);
    return EEDR;
}

// Send stored data from EEPROM to PC
void sendStoredData(void) {
    unsigned int i;
    UART_transmitString("Stored data: ");
    
    for(i = 0; i < eeprom_address; i++) {
        UART_transmitByte(EEPROM_readByte(i));
        delay_ms(5);
    }
    UART_transmitString("\r\n");
}

void main(void) {
    unsigned char received_byte;
    
    // Initialize UART
    UART_init();
    
    // Send initial message
    UART_transmitString("MCU ready to receive data\r\n");
    
    while(1) {
        // Check if data is available
        if(UCSRA & (1<<RXC)) {
            received_byte = UART_receiveByte();
            
            // Store in EEPROM if space available
            if(eeprom_address < EEPROM_SIZE) {
                EEPROM_writeByte(eeprom_address, received_byte);
                UART_transmitByte(received_byte); // Echo back
                eeprom_address++;
                
                // If end marker received or EEPROM full, send all stored data
                if(received_byte == END_MARKER || eeprom_address >= EEPROM_SIZE) {
                    delay_ms(100);
                    sendStoredData();
                    eeprom_address = 0; // Reset for next transmission
                }
            }
        }
    }
}
