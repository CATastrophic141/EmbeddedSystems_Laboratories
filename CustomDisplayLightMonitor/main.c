#include <msp430.h>
#define FLAGS UCA1IFG // Contains the transmit & receive flags
#define RXFLAG UCRXIFG // Receive flag
#define TXFLAG UCTXIFG // Transmit flag
#define TXBUFFER UCA1TXBUF // Transmit buffer
#define RXBUFFER UCA1RXBUF // Receive buffer
#define redLED BIT0 // Red LED at P1.0
#define BUT1 BIT1 // P1.1

void config_ACLK_to_32KHz_crystal();
void Initialize_UART();
void uart_write_char(unsigned char ch);
unsigned char uart_read_char();
unsigned int get_numSize(unsigned int num);
void parse_int_into_array(unsigned int num, unsigned int numSize, unsigned int* digits);
void uart_write_uint16(unsigned int num);
void uart_write_string(char string[]);

unsigned int ascii[10] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57};

//Test file that swaps colors of multicolor LED using pulse-width modulation
//According to slau599b:
//J4.37 -> Blue -> P3.6 -> TB0.2
//J4.38 -> Green -> P3.3 -> TA1.1
//J.39 -> Red -> P2.6 -> TB0.5

//ACLK -> 32KHz
//PWN -> 100 Hz
//Cycles -> ~ 320

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &=  ~LOCKLPM5; // Enable the GPIO pins

    config_ACLK_to_32KHz_crystal();

    //Green LED settings
    //J4.38 -> Green -> P3.3 -> TA1.1
    P3DIR |= BIT3;
    P3SEL0 |= BIT3;
    P3SEL1 &= ~BIT3;

    TA1CCTL1 = OUTMOD_7;   // Reset/Set

    TA1CCR0 = 33;  //Overall cycles
    TA1CCR1 = 1;   //Active duty cycles

    TA1CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

    TA1CCTL1 &= ~CCIFG;
    TA1CCTL1 |= CCIE;

    _enable_interrupts();

    while(1) {}
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

void uart_write_uint16(unsigned int num){
    //Get number of digits of number
    unsigned int numSize = get_numSize(num);
    unsigned int digits[5] = {99,99,99,99,99}; //Number will not have more than 5 digits
    //Parse the numbers digit, place into declared array
    parse_int_into_array(num, numSize, digits);
    //uart_write_char(ascii[0]);
    //unsigned int test[5] = {0,1,2,3,4};
    int i;
    for (i = 0; i < numSize; i++){
        if (digits[i] != 99) uart_write_char(ascii[digits[i]]);
    }
    uart_write_char(10); //Write /n
    uart_write_char(13); //Write /r
    return;
}
void uart_write_string(char string[]){
    int i = 0;
    while(string[i] != '\0'){
        uart_write_char(string[i]);
        i++;
    }
    uart_write_char(string[i]);
    uart_write_char(10); //Write /n
    uart_write_char(13); //Write /r
}
//Get the number of digits in a certain number
unsigned int get_numSize(unsigned int num){
    //count number of digits
    unsigned int size=1;
    while (num/=10) size++;
    return size;
}

void parse_int_into_array(unsigned int num, unsigned int numSize, unsigned int* digits){
    int size = numSize;
    while(size--){
        digits[size] = num % 10;
        num = num/10;
    }
    return;
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void T1A1_ISR() {
    if (TA1CCR1 > 160) TA1CCR1 = 0;
    else TA1CCR1 += 20;
    TA1CTL &= ~CCIFG;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR() {
    if (TA1CCR1 > 160) TA1CCR1 = 0;
    else TA1CCR1 += 20;
    TA1CTL &= ~CCIFG;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void T0A1_ISR() {
    if (TA1CCR1 > 160) TA1CCR1 = 0;
    else TA1CCR1 += 20;
    TA1CTL &= ~CCIFG;
}

/*#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
    if (TA1CCR1 > 160) TA1CCR1 = 0;
    else TA1CCR1 += 20;
    TA1CCTL1 &= ~CCIFG;
}*/
