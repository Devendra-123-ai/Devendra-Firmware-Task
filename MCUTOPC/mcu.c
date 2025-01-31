#include <mega32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <delay.h>

#define F_CPU 8000000UL  // Define CPU frequency (8 MHz)

volatile unsigned int bits_transmitted = 0;  // Count bits transmitted
volatile unsigned long transmissionTime = 0;  // Store transmission time (in clock ticks)
char data[] = "Finance Minister Arun Jaitley Tuesday hit out at former RBI governor Raghuram Rajan for predicting that the next banking crisis would be triggered by MSME lending, saying postmortem is easier than taking action when it was required. Rajan, who had as the chief economist at IMF warned of impending financial crisis of 2008, in a note to a parliamentary committee warned against ambitious credit targets and loan waivers, saying that they could be the sources of next banking crisis. Government should focus on sources of the next crisis, not just the last one. In particular, government should refrain from setting ambitious credit targets or waiving loans. Credit targets are sometimes achieved by abandoning appropriate due diligence, creating the environment for future NPAs, \"Rajan said in the note.\" Both MUDRA loans as well as the Kisan Credit Card, while popular, have to be examined more closely for potential credit risk. Rajan, who was RBI governor for three years till September 2016, is currently.";

// UART Initialization
void UART_Init(unsigned int baudrate) {
    unsigned int ubrr = F_CPU / 16 / baudrate - 1; // Calculate UBRR value
    UBRRH = (ubrr >> 8);  // Set the higher byte
    UBRRL = ubrr;         // Set the lower byte
    UCSRB = (1 << TXEN) | (1 << RXEN);  // Enable TX, RX (no interrupt)
    UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // 8-bit data, no parity, 1 stop bit
}

// Timer1 Initialization (No Prescaler, for accurate measurement)
void Timer1_Init(void) {
    TCCR1B = (1 << CS10); // No prescaler, clock = 8 MHz
    TCNT1 = 0;  // Reset timer
}

// UART Transmit Single Character
void UART_Transmit(char data) {
    while (!(UCSRA & (1 << UDRE)));  // Wait until buffer is empty
    UDR = data;  // Send data
}

// UART Send String
void UART_SendString(const char* str) {
    while (*str) {
        UART_Transmit(*str++);
    }
}

// Poll the TXC flag to count bits transmitted
void Check_Transmit_Complete(void) {
    if (UCSRA & (1 << TXC)) {  // Check if transmission is complete
        bits_transmitted += 8;  // Every byte is 8 bits
        UCSRA |= (1 << TXC);     // Clear the TXC flag
    }
}

// Calculate and display the transmission speed (bps)
void Measure_Transmission_Speed(void) {
    // We measure how many bits are transmitted in a given period
    char speedMsg[50];
    
    unsigned long speed_bps = (unsigned long)bits_transmitted * F_CPU / transmissionTime;
    sprintf(speedMsg, "Speed: %lu bps\r\n", speed_bps);
    UART_SendString(speedMsg);
    
    // Reset the counters for the next measurement period
    bits_transmitted = 0;
    transmissionTime = 0;
}

// Main Program
void main(void) {
    UART_Init(2400); // Initialize UART with 2400 baud rate
    Timer1_Init();   // Initialize Timer1 for accurate time measurement

    // Transmit Data and measure transmission speed
    while (1) {
        transmissionTime = TCNT1;  // Start the timer before transmission
        UART_SendString(data);  // Send data to UART
        transmissionTime = TCNT1 - transmissionTime;  // Stop the timer after transmission

        Measure_Transmission_Speed();  // Calculate and print speed
        
        // Poll the TXC flag to count bits transmitted
        Check_Transmit_Complete();

        delay_ms(1000);  // Wait for 1 second before measuring again
    }
}
