// Setting up a PWM on P1.0 (red LED)
// P1.0 coincides with TA0.1 (Timer0_A Channel 1)
// Configure P1.0 pin to TA0.1 ---> P1DIR=1, P1SEL1=0, P1SEL0=1
// PWM frequency: 1000 Hz -> 0.001 seconds

/*

#include <msp430fr6989.h>
#define PWM_PIN BIT0

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
WDTCTL = WDTPW | WDTHOLD; // Stop WDT
PM5CTL0 &=  ~LOCKLPM5;

P1DIR |= PWM_PIN; // P1DIR bit = 1
P1SEL1 &= ~PWM_PIN; // P1SEL1 bit = 0
P1SEL0 |= PWM_PIN; // P1SEL0 bit = 1

config_ACLK_to_32KHz_crystal();

TA0CCTL1 = OUTMOD_7; // Modify OUTMOD field to 7
TA0CCR1 = 1; // Modify this value between 0 and
TA0CCR0 = 33; // @ 32 KHz --> 0.001 seconds (1000 Hz)
TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

for(;;) {}
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void T0A1_ISR() {
    if (TA0CCR1 > 31) TA0CCR1 = 0;
    else TA0CCR1 += 1;
}

*/
