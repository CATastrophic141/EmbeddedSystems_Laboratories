/*

// Timer_A continuous mode, with interrupts, flashes LEDs
#include <msp430fr6989.h>
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7
#define BUT1 BIT1 // P1.1
#define BUT2 BIT2 //P1.2

void main(void) {
WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
PM5CTL0 &=  ~LOCKLPM5; // Enable the GPIO pins
P1DIR |= redLED; // Configure pin as output
P9DIR |= greenLED; // Configure pin as output
P1OUT &=  ~redLED; // Turn LED Off
P9OUT &=  ~greenLED; // Turn LED Off
// Configure the buttons for interrupts
P1DIR &=  ~(BUT1|BUT2); // 0: input
P1REN |= (BUT1|BUT2); // 1: enable built-in resistors
P1OUT |= (BUT1|BUT2); // 1: built-in resistor is pulled up to Vcc
P1IES |= (BUT1|BUT2); // 1: interrupt on falling edge (0 for rising edge)
P1IFG &=  ~(BUT1|BUT2); // 0: clear the interrupt flags
P1IE |= (BUT1|BUT2); // 1: enable the interrupts
// Enable the global interrupt bit (call an intrinsic function)
_enable_interrupts();

for(;;) {}
}

//*******************************
#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR() {
// Action goes here
    //Detect button1
    if((P1IFG & BUT1) == BUT1){
        P1OUT ^= redLED;
        _delay_cycles (100000);
        P1IFG &=  ~(BUT1);
    }
    //Detect button2
    if((P1IFG & BUT2) == BUT2){
        P9OUT ^= greenLED;
        _delay_cycles (100000);
        P1IFG &= ~(BUT2);
    }
}

*/
