/*#include <msp430fr6989.h>
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7
#define BUT1 BIT1 // Button S1 at P1.1
#define BUT2 BIT2 //P1.2
void main(void) {
	WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
	PM5CTL0 &= ~LOCKLPM5; // Enable the GPIO pins
	P1DIR |= redLED; // Direct pin as output
	P1OUT &= ~redLED; // Turn LED Off
	P1DIR &= ~(BUT1); // Direct Pin as input
	P1REN |= (BIT1); // Enable built-in resistor
	P1OUT |= (BIT1); // Set resistor as pull-up
	// Polling the button in an infinite loop
	for (;;) {
		if ((P1IN & BUT1) == 0) { //Check if red LED button is active
			P1OUT |= redLED;
		}
		else { //Otherwise disable the LED
			P1OUT &= ~redLED;
		}
	}
}*/