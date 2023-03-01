/*#include <msp430fr6989.h>
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7
#define BUT1 BIT1 // Button S1 at P1.1
#define BUT2 BIT2 //P1.2

// Configures ACLK to 32 KHz crystal
void config_ACLK_to_32KHz_crystal() {
// By default, ACLK runs on LFMODCLK at 5MHz/128 = 39 KHz
// Reroute pins to LFXIN/LFXOUT functionality
PJSEL1 &= ~BIT4;
PJSEL0 |= BIT4;
// Wait until the oscillator fault flags remain cleared
CSCTL0 = CSKEY; // Unlock CS registers
do {
CSCTL5 &= ~LFXTOFFG; // Local fault flag
SFRIFG1 &= ~OFIFG; // Global fault flag
} while((CSCTL5 & LFXTOFFG) != 0);
CSCTL0_H = 0; // Lock CS registers
return;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Enable the GPIO pins
    // Configure and initialize LEDs
    P1DIR |= redLED; // Direct pin as output
    P9DIR |= greenLED; // Direct pin as output
    P1OUT &= ~redLED; // Turn LED Off
    P9OUT &= ~greenLED; // Turn LED Off
    // Configure buttons
    P1DIR &= ~(BUT1 | BUT2); // Direct Pin as input, uses both buttons   >>> CONFIGURATIONS NEED TO INCLUDE ALL INPUTS RELATED
    P1REN |= (BIT1 | BUT2); // Enable built-in resistor for both buttons
    P1OUT |= (BIT1 | BUT2); // Set resistor as pull-up for both buttons
    // Configure ACLK to the 32 KHz crystal (function call)
    config_ACLK_to_32KHz_crystal();
    //Configure CCR0 value
    TA0CCR0 = 32767; //32 * 2^10 -> K => 2^10
    // Configure Timer_A
    // Use ACLK, divide by 1, continuous mode, clear TAR
    TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
    // Ensure flag is cleared at the start
    TA0CTL &=  ~TAIFG;
    // Infinite loop
    for(;;) {
    // Wait in this empty loop for the flag to raise
    while((TA0CTL & TAIFG) != TAIFG) {}
    // Do the action here
    P1OUT ^= redLED;
    TA0CTL &=  ~TAIFG;
    }
}
*/
