#include <msp430.h>
#include <stdio.h>
#define FLAGS UCA1IFG // Contains the transmit & receive flags
#define RXFLAG UCRXIFG // Receive flag
#define TXFLAG UCTXIFG // Transmit flag
#define TXBUFFER UCA1TXBUF // Transmit buffer
#define RXBUFFER UCA1RXBUF // Receive buffer
#define redLED BIT0 // Red LED at P1.0
#define greenLED BIT7 // Green LED at P9.7
#define BUT1 BIT1 // P1.1
#define BUT2 BIT2 //P1.2

void config_ACLK_to_32KHz_crystal();
void Initialize_UART();
void Initialize_UART_2();
void uart_write_char(unsigned char ch);
unsigned char uart_read_char();
unsigned int get_numSize(unsigned int num);
void parse_int_into_array(unsigned int num, unsigned int numSize, unsigned int* digits);
void uart_write_uint16(unsigned int num);
void uart_write_string(char string[]);

unsigned int ascii[10] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57};
int runway1State = 0; //0 = unused (off) /// 1 = requested (on) /// 2 = using (flashing)
int runway2State = 0; //0 = unused (off) /// 1 = requested (on) /// 2 = using (flashing)


char string1[] = "Runway 1 controls:   Request = 1   Forfeit = 9";
char string2[] = "Runway 2 controls:   Request = 2   Forfeit = 0";
char newline[] = "\n\r";
char outline[] = "____________________________________________________________________________";
char runways[] = "\t\tRUNWAY 1\t\t\t\t\tRUNWAY 2";
char request1[] = "\t\tREQUESTED   ";
char request2[] = "\t\t\t\t\t\t\t\tREQUESTED";
char used1[] = "\t\tIN USE";
char used2[] = "\t\t\t\t\t\t\t\tIN USE";
char inquiry1[] = "\t\t**INQUIRY**";
char inquiry2[] = "\t\t\t\t\t\t\t\t**INQUIRY**";
char CLEARLINE[] = "\033[2K\r";
char GOTO_LINE9[] = "\033[9;0H";
char GOTO_LINE10[] = "\033[10;0H";
char GOTO_LINE11[] = "\033[11;0H";

int main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &=  ~LOCKLPM5; // Enable the GPIO pins
    // Configure and initialize LEDs
    P1DIR |= redLED; // Direct pin as output
    P9DIR |= greenLED; // Direct pin as output
    P1OUT &=  ~redLED; // Turn LED Off
    P9OUT &=  ~greenLED; // Turn LED Off

    config_ACLK_to_32KHz_crystal();

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

    // Configure pins to backchannel UART
    // Pins: (UCA1TXD / P3.4) (UCA1RXD / P3.5)
    // (P3SEL1=00, P3SEL0=11) (P2DIR=xx)
    P3SEL1 &=  ~(BIT4|BIT5);
    P3SEL0 |= (BIT4|BIT5);
    Initialize_UART();

    unsigned char input;
    unsigned char data;

    //Write headings
    uart_write_string(string1); //Line1
    uart_write_string(newline);
    uart_write_string(string2); //Line2
    uart_write_string(newline); //Line3
    uart_write_string(newline); //Line4
    uart_write_string(outline); //Line5
    uart_write_string(newline);
    uart_write_string(runways); //Line6
    uart_write_string(newline);
    uart_write_string(outline); //Line7
    uart_write_string(newline); //Line8

        //REQUEST LINE = LINE 9
        //USE LINE = LINE 10
        //INQUIRY LINE = LINE 11

    TA0CTL |= MC_1; //Start timer for interrupts

    while(1){
        _delay_cycles(100000); //Delay

        input = uart_read_char(); //Read input
        if (input != 0){
            data = input; //If input was not null, set data <Allows for null uses>
        }

        if (data == 49){ //1 received
            if (runway1State == 0){
                runway1State = 1;
                uart_write_string(GOTO_LINE9);
                uart_write_string(request1);
            }
        }
        if (data == 50){ //2 received
            if (runway2State == 0){
                runway2State = 1;
                uart_write_string(GOTO_LINE9);
                uart_write_string(request2);
           }
        }
        if (data == 57) { //9 received
            if (runway1State == 1){
            uart_write_string(GOTO_LINE9);
            uart_write_string(CLEARLINE);
            if (runway2State == 1) uart_write_string(request2);
            }
            else if (runway1State == 2) {
                uart_write_string(GOTO_LINE10);
                uart_write_string(CLEARLINE);
                uart_write_string(GOTO_LINE11);
                uart_write_string(CLEARLINE);
            }
            runway1State = 0;
        }
        if (data == 48) { //0 received
            if (runway2State == 1){
            uart_write_string(GOTO_LINE9);
            uart_write_string(CLEARLINE);
            if (runway1State == 1) uart_write_string(request1);
            }
            else if (runway2State == 2) {
                uart_write_string(GOTO_LINE10);
                uart_write_string(CLEARLINE);
                uart_write_string(GOTO_LINE11);
                uart_write_string(CLEARLINE);
            }
            runway2State = 0;
        }
        data = 0; //Data has been consumed
    }
}

void uart_write_string(char string[]){
    int i = 0;
    while(string[i] != '\0'){
        uart_write_char(string[i]);
        i++;
    }
    uart_write_char(string[i]);
}

// Configure UART to the popular configuration
// 9600 baud, 8-bit data, LSB first, no parity bits, 1 stop bit
// no flow control, oversampling reception
// Clock: SMCLK @ 1 MHz (1,000,000 Hz)
void Initialize_UART(){
    // Configure pins to UART functionality
    P3SEL1 &=  ~(BIT4|BIT5);
    P3SEL0 |= (BIT4|BIT5);

    // Main configuration register
    UCA1CTLW0 = UCSWRST; // Engage reset; change all the fields to zero

    // Most fields in this register, when set to zero, correspond to the popular configuration
    UCA1CTLW0 |= UCSSEL_2; // Set clock to SMCLK

    // Configure the clock dividers and modulators (and enable oversampling)
    UCA1BRW = 6; // divider

    // Modulators: UCBRF = 8 = 1000 --> UCBRF3 (bit #3)
    // UCBRS = 0x20 = 0010 0000 = UCBRS5 (bit #5)
    UCA1MCTLW = UCBRF3 | UCBRS5 | UCOS16;

    // Exit the reset state
    UCA1CTLW0 &=  ~UCSWRST;
}

// The function returns the byte; if none received, returns null character
unsigned char uart_read_char(){
// Temporary storage variable
    unsigned char temp;

// Return null character (ASCII=0) if no byte was received
    if( (FLAGS & RXFLAG) == 0) return 0;

// Otherwise, copy the received byte (this clears the flag) and return it
    temp = RXBUFFER;

    return temp;
}

void uart_write_char(unsigned char ch){
// Wait for any ongoing transmission to complete
    while ( (FLAGS & TXFLAG)==0 ) {}
// Copy the byte to the transmit buffer
    TXBUFFER = ch; // Tx flag goes to 0 and Tx begins!
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


//  //  // INTERRUPT VECTORS

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR() {
    //Detect button1
    if((P1IFG & BUT1) == BUT1){

        _delay_cycles(200000); //Debounce delay

        if (runway1State == 1 && runway2State < 2){
            runway1State = 2;
            uart_write_string(GOTO_LINE9);
            uart_write_string(CLEARLINE);
            if (runway2State == 1) uart_write_string(request2);
            uart_write_string(GOTO_LINE10);
            uart_write_string(used1);
        }

        else if (runway1State == 2) {
            uart_write_string(GOTO_LINE11);
            uart_write_string(inquiry1);
        }
        P1IFG &=  ~(BUT1);
        TA0CTL &= ~TAIFG;
    }
    //Detect button2
    if((P1IFG & BUT2) == BUT2){

        _delay_cycles(200000); //Debounce delay

        if (runway2State == 1 && runway1State < 2){
            runway2State = 2;
            uart_write_string(GOTO_LINE9);
            uart_write_string(CLEARLINE);
            if (runway1State == 1) uart_write_string(request1);
            uart_write_string(GOTO_LINE10);
            uart_write_string(used2);
        }

        else if (runway2State == 2) {
            uart_write_string(GOTO_LINE11);
            uart_write_string(inquiry2);
        }
        P1IFG &=  ~(BUT2);
        TA0CTL &= ~TAIFG;
     }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
    TA0CTL &= ~TAIFG;
    //First runway handler
    if (runway1State == 0) P1OUT &= ~redLED;
    else if (runway1State == 1) P1OUT |= redLED;
    else if (runway1State == 2) P1OUT ^= redLED;

    //Second runway handler
    if (runway2State == 0) P9OUT &= ~greenLED;
    else if (runway2State == 1) P9OUT |= greenLED;
    else if (runway2State == 2) P9OUT ^= greenLED;
}

