
#include <msp430.h>
#include <stdint.h>
#define redLED BIT0
#define grnLED BIT7
void main(void){
volatile int i;
volatile int j;
volatile unsigned int delay;
WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
PM5CTL0 &= ~LOCKLPM5; // Disable GPIO power-on default high-impedance mode
P1DIR |= redLED; // Direct pin as output
P9DIR |= grnLED;
P1OUT &= ~redLED; // Turn LED Off
P9OUT &= ~grnLED;
for(;;){
P1OUT &= BIT0;
P9OUT &= ~BIT7;
for (delay = 60000; delay > 1000; delay = delay - 1000){
P1OUT ^= redLED;
P9OUT ^= grnLED;
for(i = 0; i < delay; i++){}
}
P1OUT |= BIT0;
P9OUT |= BIT7;
for (j = 0; j < 10; j++){
_delay_cycles(100000);
P1OUT ^= redLED;
P9OUT ^= grnLED;
}
}9OUT ^= grnLED;
}

