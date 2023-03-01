/*#include <msp430fr6989.h>
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7

#include <msp430fr6989.h>
#define redLED BIT0
void main(void) {
volatile unsigned int i;
WDTCTL = WDTPW | WDTHOLD;
PM5CTL0 &= ~LOCKLPM5;
P1DIR |= redLED;
P1OUT &= ~redLED;
for(;;) {
for(i=0; i<20000; i++) {}
P1OUT ^= redLED;
}
}
*/
