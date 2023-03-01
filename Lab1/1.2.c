/*#include <msp430fr6989.h>
#define redLED BIT0
#define grnLED BIT7
void main(void) {
volatile unsigned int i;
WDTCTL = WDTPW | WDTHOLD;
PM5CTL0 &= ~LOCKLPM5;
P1DIR |= redLED;
P9DIR |= grnLED;
P1OUT &= ~redLED;
P9OUT &= ~grnLED;
for(;;) {
//FOR IN-SYNC
for(i=0; i<20000; i++) {}
P1OUT ^= redLED;
P9OUT ^= grnLED;
//FOR OUT-OF-SYNC
/*for(i=0; i<20000; i++) {}
P1OUT ^= redLED;
for(i=0; i<10000; i++) {}
P9OUT ^= grnLED;*/
}
*/
