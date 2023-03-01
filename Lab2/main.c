/*#include <msp430fr6989.h>
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
// CORRESPONDING INPUT VALUE IS NOT JUST 1/0. IS MULTI-BIT INPUT FOR EACH CHANNEL
P1DIR &= ~(BUT1 | BUT2); // Direct Pin as input, uses both buttons
P1REN |= (BIT1 | BUT2); // Enable built-in resistor for both buttons
P1OUT |= (BIT1 | BUT2); // Set resistor as pull-up for both buttons
// Polling the button in an infinite loop
for(;;) {
//Start check for red LED
if ((P1IN & BUT1) == 0) {
//Check if button 2 active
if((P1IN & BUT2) == 0) {
// Disable both LEDs
P1OUT &= ~redLED;
P9OUT &= ~greenLED;
// Loop until both buttons are inactive
while(((P1IN & BUT1) == 0) || ((P1IN & BUT2) == 0)) {}
}
// Enable red LED since button 2 was not activated
else {P1OUT |= redLED;}
}
// Disable red LED in base case
else P1OUT &= ~redLED;
// Start check for green LED
if ((P1IN & BUT2) == 0) {
//Check if button 1 is active
if((P1IN & BUT1) == 0) {
// Disable both LEDs
P1OUT &= ~redLED;
P9OUT &= ~greenLED;
// Loop until both buttons are inactive
while(((P1IN & BUT1) == 0) || ((P1IN & BUT2) == 0)) {}
}
// Enable green LED since button 1 was not activated
else {P9OUT |= greenLED;}
}
// Disable green LED in base case
else P9OUT &= ~greenLED;
}
}
*/