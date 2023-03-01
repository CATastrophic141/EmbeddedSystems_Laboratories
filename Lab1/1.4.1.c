/*#include <msp430.h>
#include <stdint.h>
#define redLED BIT0
#define grnLED BIT7
void main(void) {
volatile int i;
volatile int j;
volatile int k;
WDTCTL = WDTPW | WDTHOLD;
PM5CTL0 &= ~LOCKLPM5;
P1DIR |= redLED;
P9DIR |= grnLED;
P1OUT &= ~redLED;
P9OUT &= ~grnLED;
for(;;){
P1OUT &= ~BIT0;
P9OUT &= ~BIT7;
for (i = 3; i > 0; i--){
P9OUT &= ~BIT7; //Ensure green LED is off
for (j = 10; j > 0; j--){
_delay_cycles(10000);
P1OUT ^= redLED;
}
P1OUT &= ~BIT0; //Ensure red LED is off
for (k = 10; k > 0; k--){
_delay_cycles(10000);
P9OUT ^= grnLED;
}
}
for (i = 3; i > 0; i--){
P9OUT &= ~BIT7;
for (j = 5; j > 0; j--){
_delay_cycles(100000);
P1OUT ^= redLED;
}
P1OUT &= ~BIT0;
for (k = 5; k > 0; k--){
_delay_cycles(100000);
P9OUT ^= grnLED;
}
}
*/
