/*#include <msp430.h>
#include <stdint.h>
#define redLED BIT0
#define grnLED BIT7
void main(void) {
volatile int i;
volatile unsigned long delay = 100000;
WDTCTL = WDTPW | WDTHOLD;
PM5CTL0 &= ~LOCKLPM5;
P1DIR |= redLED;
P9DIR |= grnLED;
P1OUT &= ~redLED;
P9OUT &= ~grnLED;
for(;;){
//Delay loop <Unsigned long loop limit>
for (i = 0; i < delay; i++) {}
P1OUT ^= redLED;
P9OUT ^= grnLED;
//Delay loop <Cycle delay>
//delay_cycles(1000000);
//P1OUT ^= redLED;
//P9OUT ^= grnLED;
}
*/
