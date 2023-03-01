#include <msp430fr6989.h>
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7
#define BUT1 BIT1 // Button S1 at P1.1
#define BUT2 BIT2 //P1.2

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
    int butPressFlag = 0;
    WDTCTL = WDTPW | WDTHOLD; // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Enable the GPIO pins
    // Configure and initialize LEDs
    P1DIR |= redLED; // Direct pin as output
    P9DIR |= greenLED; // Direct pin as output
    P1OUT &= ~redLED; // Turn LED Off
    P9OUT &= ~greenLED; // Turn LED Off
    // Configure buttons
    P1DIR &= ~(BUT1 | BUT2); // Direct Pin as input, uses both buttons   >>> CONFIGURATIONS NEED TO INCLUDE ALL INPUTS RELATED
    P1REN |= (BIT1 | BUT2); // Enable built-in resistor for both buttons
    P1OUT |= (BIT1 | BUT2); // Set resistor as pull-up for both buttons
    // Configure ACLK to the 32 KHz crystal (function call)
    config_ACLK_to_32KHz_crystal();
    //Configure CCR0 value
    //TA0CCR0 = 32767; //32 * 2^10 -> K => 2^10
    // Configure Timer_A
    // Use ACLK, divide by 1, continuous mode, clear TAR
    TA0CTL = TASSEL_1 | ID_0 | MC_0 | TACLR;
    // Ensure flag is cleared at the start
    TA0CTL &=  ~TAIFG;
    // Infinite loop
    for(;;) {
        TA0CTL &= ~MC_3
        TA0CTL |= MC_0; //Start timer in off state
        TA0CTL |= TACLR; //Ensure timer register is reset
        TA0CCR0 = 0; //Ensure TA0CCR0 is set to a 0 state
        butPressFlag = 0; //Ensure backup flag is reset
        if ( ((P1IN & BUT1) == 0) ) { //If button 1 has been pressed
            TA0CTL |= MC_2; //Start continuous timer
            while ( ((P1IN & BUT1) == 0) && ((TA0CTL & TAIFG) != TAIFG) ){} //While button is pressed and flag is not up
            TA0CTL &= ~MC_3; //Stop timer, keep TA0R

            if((TA0CTL & TAIFG) == TAIFG){ //If continuous timer raised flag
                P9OUT |= greenLED; //Turn on error-flag LED
                while ((P1IN & BUT2) != 0){} //Wait until button 2 is pressed
                P9OUT &= ~greenLED; //Turn off green LED
                TA0CTL &=  ~TAIFG; //Reset flag
                TA0CTL &= ~MC_3; //Stop timer
                TA0CTL |= TACLR; //Clear TAR
                continue; //Exit parent if statement without running remaining code
            }
            //else {
                butPressFlag = 1; //Button was pressed and error was not raised, set backup flag
                TA0CCR0 = TA0R; //Copy timer to TA0CCR0
                TA0CTL |= TACLR; //Clear TAR
                TA0CTL &= ~MC_3;
            //}
        }
        if ((butPressFlag == 1 || TA0CCR0 != 0) && (TA0CTL & TAIFG) != TAIFG) { //IF TA0CCR0 has been altered
            TA0CTL |= MC_1 | TACLR; //Start timer in up-mode & ensure timer is cleared
            P1OUT |= redLED; //Turn on LED
            while((TA0CTL & TAIFG) != TAIFG) {} //Wait until flag is raised
            P1OUT &= ~redLED; //Turn off LED
            TA0CTL &=  ~TAIFG; //Clear Flag
            TA0CTL &= ~MC_3; //Stop timer
            butPressFlag = 0; //Clear backup flag
            TA0CCR0 = 0; //Clear TA0CCR0
            TA0CTL |= TACLR; //Clear TAR
        }
    }
}
