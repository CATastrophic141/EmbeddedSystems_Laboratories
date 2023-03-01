/*
#include <msp430fr6989.h>
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7
#define BUT1 BIT1 // Button S1 at P1.1
#define BUT2 BIT2 //P1.2
void main(void) {
WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
PM5CTL0 &= ~LOCKLPM5; // Enable the GPIO pins
// Configure and initialize LEDs
P1DIR |= redLED; // Direct pin as output
P9DIR |= greenLED; // Direct pin as output
P1OUT &= ~redLED; // Turn LED Off
P9OUT &= ~greenLED; // Turn LED Off
// Configure buttons
P1DIR &= ~(BUT1 | BUT2); // Direct Pin as input, uses both buttons
P1REN |= (BIT1 | BUT2); // Enable built-in resistor for both buttons
P1OUT |= (BIT1 | BUT2); // Set resistor as pull-up for both buttons
// Polling the buttons in an infinite loop
for(;;) {
if ((P1IN & BUT1) == 0){ //Check if red LED button is active, handle accordingly
P1OUT |= redLED;
}
else {
P1OUT &= ~redLED;
}
if ((P1IN & BUT2) == 0){ //Check if green LED button is active, handle accordingly
P9OUT |= greenLED;
}
else {
P9OUT &= ~greenLED;
}
}
}
*/