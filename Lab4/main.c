// Timer_A continuous mode, with interrupts, flashes LEDs
#include <msp430fr6989.h>
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7
#define BUT1 BIT1 // P1.1
#define BUT2 BIT2 //P1.2

int STATE = 0;

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
// Configure Channel 0 for up mode with interrupts
TA0CCR0 = 32767; // 1 second @ 32 KHz
TA0CCTL0 |= CCIE; // Enable Channel 0 CCIE bit
TA0CCTL0 &= ~CCIFG; // Clear Channel 0 CCIFG bit
// Timer_A: ACLK, div by 1, up mode, clear TAR
TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
// Enable the global interrupt bit (call an intrinsic function)
_enable_interrupts();

    for(;;) {
    switch(STATE){
        case -3:
            TA0CCR0 = 16383;
            P9OUT &=  ~greenLED; // Turn LED Off
            break;
        case -2:
            TA0CCR0 = 32767;
            P9OUT &=  ~greenLED;
            break;
        case -1:
            TA0CCR0 = 65535;
            P9OUT &=  ~greenLED;
            break;
        case 0:
            TA0CCR0 = 32767;
            break;
        case 3:
            TA0CCR0 = 16383;
            P1OUT &=  ~redLED; // Turn LED Off
            break;
        case 2:
            TA0CCR0 = 32767;
            P1OUT &=  ~redLED;
            break;
        case 1:
            TA0CCR0 = 65535;
            P1OUT &=  ~redLED;
            break;
        }
    }
}

//*******************************
#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR() {
// Action goes here
    //Detect button1
    if((P1IFG & BUT1) == BUT1){
        if (STATE > -3){
        STATE--;
        if (STATE == 0){
            P1OUT &= ~redLED;
            P9OUT &= ~greenLED;
        }
        }
        _delay_cycles (100000);
        P1IFG &=  ~(BUT1);
    }
    //Detect button2
    if((P1IFG & BUT2) == BUT2){
        if (STATE < 3){
        STATE++;
        if (STATE == 0){
             P1OUT &= ~redLED;
             P9OUT &= ~greenLED;
        }
        }
        _delay_cycles (100000);
        P1IFG &= ~(BUT2);
    }

}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
if (STATE == 0){
    P1OUT ^= redLED;
    P9OUT ^= greenLED;
}
if (STATE < 0){
    P1OUT ^= redLED;
}
if (STATE > 0){
    P9OUT ^= greenLED;
}
}

