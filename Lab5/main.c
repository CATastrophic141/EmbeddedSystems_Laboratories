// Sample code that prints 430 on the LCD monitor
#include <msp430fr6989.h>
#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7
#define BUT1 BIT1 // P1.1
#define BUT2 BIT2 //P1.2

void config_ACLK_to_32KHz_crystal();
void Initialize_LCD();
void print_time();

//8-Segment display numbers
const unsigned char LCD_Shapes[10] = { 0xFC, 0x60, 0xDB, 0xF3, 0x67, 0xB7, 0xBF, 0xE0, 0xFF, 0xE7 };
unsigned long time = 0;
int mode = 0; //0 == off, 1 == on

int main(void) {
    volatile unsigned int n;
    WDTCTL = WDTPW | WDTHOLD; // Stop WDT
    PM5CTL0 &= ~LOCKLPM5; // Enable GPIO pins

    //LED setup
    P1DIR |= redLED; // Pins as output
    P9DIR |= greenLED;
    P1OUT |= redLED; // Red on
    P9OUT &= ~greenLED; // Green off

    // Configure the buttons for interrupts
    P1DIR &= ~(BUT1 | BUT2); // 0: input
    P1REN |= (BUT1 | BUT2); // 1: enable built-in resistors
    P1OUT |= (BUT1 | BUT2); // 1: built-in resistor is pulled up to Vcc
    P1IES |= (BUT1 | BUT2); // 1: interrupt on falling edge (0 for rising edge)
    P1IFG &= ~(BUT1 | BUT2); // 0: clear the interrupt flags
    P1IE |= (BUT1 | BUT2); // 1: enable the interrupts

    // Configure Channel 0 for up mode with interrupts
    TA0CCR0 = 32767; // 1 second @ 32 KHz
    TA0CCTL0 |= CCIE; // Enable Channel 0 CCIE bit
    TA0CCTL0 &= ~CCIFG; // Clear Channel 0 CCIFG bit
    // Timer_A: ACLK, div by 1, up mode, clear TAR
    TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
    TA0CTL &= ~TAIFG;
    TA0CTL &= ~MC_3; //Start with clock off

    // Enable the global interrupt bit (call an intrinsic function)
    _enable_interrupts();

    // Initializes the LCD_C module
    Initialize_LCD();
    // The line below can be used to clear all the segments
    //LCDCMEMCTL = LCDCLRM; // Clears all the segments

    // Display 0, setup decimal point;
    print_time(time);
    LCDM20 = 0x01; //Leftmost bit of M20
    LCDM7 |= BIT2;

    for (;;) {
        if (mode == 0) {
            LCDM3 |= BIT0;
            LCDM3 &= ~BIT3;
        }
        else {
            LCDM3 |= BIT3;
            LCDM3 &= ~BIT0;
        }
        //if ((TA0CTL & TAIFG) != 0) LCDM3 |= BIT1; //Test
    }
}

void print_time(unsigned long time) {

    long hours;
    long minutes;
    long seconds;
    hours = time / 3600;
    minutes = (time / 60) % 60;
    seconds = time % 60;

    long digit;

    digit = hours / 10;
    LCDM10 = LCD_Shapes[digit]; //Num 1
    digit = hours % 10;
    LCDM6 = LCD_Shapes[digit]; //Num 2

    digit = minutes / 10;
    LCDM4 = LCD_Shapes[digit]; //Num 3
    digit = minutes % 10;
    LCDM19 = LCD_Shapes[digit]; //Num 4

    digit = seconds / 10;
    LCDM15 = LCD_Shapes[digit]; //Num 5
    digit = seconds % 10;
    LCDM8 = LCD_Shapes[digit]; //Num 6

    return;
}

void Initialize_LCD() {
    PJSEL0 = BIT4 | BIT5; // For LFXT
    // Initialize LCD segments 0 - 21; 26 - 43
    LCDCPCTL0 = 0xFFFF;
    LCDCPCTL1 = 0xFC3F;
    LCDCPCTL2 = 0x0FFF;
    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8; // Unlock CS registers
    CSCTL4 &= ~LFXTOFF; // Enable LFXT
    do {
        CSCTL5 &= ~LFXTOFFG; // Clear LFXT fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG); // Test oscillator fault flag
    CSCTL0_H = 0; // Lock CS registers
    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;
    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;
    LCDCCPCTL = LCDCPCLKSYNC; // Clock synchronization enabled
    LCDCMEMCTL = LCDCLRM; // Clear LCD memory
    //Turn LCD on
    LCDCCTL0 |= LCDON;
    return;
}

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
    } while ((CSCTL5 & LFXTOFFG) != 0);
    CSCTL0_H = 0; // Lock CS registers
    return;
}

/*******************************/
#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR() {
    //Detect button1
    if ((P1IFG & (BUT1 | BUT2)) == BUT1) {
        _delay_cycles(100000);
        TA0CTL &= ~MC_3;
        TA0CTL |= TACLR; //Start counting how long button is pressed
        TA0CTL &= ~TAIFG;
        TA0CTL |= MC_1;
        while ((P1IN & BUT1) == 0) {} //Let counter detect how long button was pressed for
        TA0CTL &= ~MC_3;

        if ((TA0CTL & TAIFG) != 0) {
            time = 0;
            print_time(0);
            TA0CTL |= TACLR;
            TA0CTL &= ~MC_3;
            mode = 0;
        }

        if ((TA0CTL & TAIFG) == 0) {
            if (mode == 0) {
                mode = 1;
                TA0CTL |= MC_1;
            }
            else {
                mode = 0;
                TA0CTL &= ~MC_3;
            }
            print_time(time);
            P1IFG &= ~(BUT1);
            TA0CTL &= ~TAIFG;
        }
        _delay_cycles(5000);
        P1IFG &= ~(BUT1);
        TA0CTL &= ~TAIFG;
    }
    //Detect button2
    if ((P1IFG & BUT2) == BUT2) {
        _delay_cycles(100000);
        while ((P1IN & BUT2) == 0) {
            _delay_cycles(100000);
            if ((P1IN & BUT1) == 0) {
                if (time == 0) time = 86399;
                else time--;
                print_time(time);
            }
            else {
                if (time == 86399) time = 0;
                else time++;
                print_time(time);
            }
        }
        _delay_cycles(100000);
        P1IFG &= ~(BUT2);
        P1IFG &= ~(BUT1);
    }
    TA0CCTL0 &= ~CCIFG;
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
    TA0CTL &= ~TAIFG;
    //Increment time, toggle colon
    LCDM7 ^= BIT2;
    if (time == 86399) time = 0;
    else time++;
    //Print num
    print_time(time);
    P1OUT ^= redLED;
}