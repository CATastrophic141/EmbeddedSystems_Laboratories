/*// Sample code that prints 430 on the LCD monitor
#include <msp430fr6989.h>
#define redLED BIT0 // Red at P1.0
#define greenLED BIT7 // Green at P9.7
#define BUT1 BIT1 // P1.1
#define BUT2 BIT2 //P1.2

void config_ACLK_to_32KHz_crystal();
void Initialize_LCD();
void print_uint16();

//8-Segment display numbers
const unsigned char LCD_Shapes[10] = {0xFC, 0x60, 0xDB, 0xF3, 0x67, 0xB7, 0xBF, 0xE0, 0xFF, 0xE7};
unsigned int num = 0;

int main(void) {
volatile unsigned int n;
WDTCTL = WDTPW | WDTHOLD; // Stop WDT
PM5CTL0 &=  ~LOCKLPM5; // Enable GPIO pins

//LED setup
P1DIR |= redLED; // Pins as output
P9DIR |= greenLED;
P1OUT |= redLED; // Red on
P9OUT &=  ~greenLED; // Green off

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


// Initializes the LCD_C module
Initialize_LCD();
// The line below can be used to clear all the segments
//LCDCMEMCTL = LCDCLRM; // Clears all the segments

// Display 0;
print_uint16(num);

// Flash the red LED
for(;;) {}
}

void print_uint16(unsigned int num){
    //LCDM10 = LCD_Shapes[digit]; //Num 1, unused
    int digit;
    digit = num % 10;
    LCDM8 = LCD_Shapes[digit]; //Num 6
    num = num / 10;
    digit = num % 10;
    LCDM15 = LCD_Shapes[digit]; //Num 5
    num = num / 10;
    digit = num % 10;
    LCDM19 = LCD_Shapes[digit]; //Num 4
    num = num / 10;
    digit = num % 10;
    LCDM4 = LCD_Shapes[digit]; //Num 3
    num = num / 10;
    digit = num % 10;
    LCDM6 = LCD_Shapes[digit]; //Num 2
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
CSCTL4 &=  ~LFXTOFF; // Enable LFXT
do {
CSCTL5 &=  ~LFXTOFFG; // Clear LFXT fault flag
SFRIFG1 &=  ~OFIFG;
}while (SFRIFG1 & OFIFG); // Test oscillator fault flag
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
} while((CSCTL5 & LFXTOFFG) != 0);
CSCTL0_H = 0; // Lock CS registers
return;
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR() {
    //Detect button1
    if((P1IFG & BUT1) == BUT1){
        num = 0;
        print_uint16(num);
        _delay_cycles (100000);
        P1IFG &=  ~(BUT1);
    }
    //Detect button2
    if((P1IFG & BUT2) == BUT2){
        num += 1000;
        print_uint16(num);
        _delay_cycles (100000);
        P1IFG &=  ~(BUT2);
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
    //Increment num
    num++;
    //Print num
    print_uint16(num);
    P1OUT ^= redLED;
}
*/
