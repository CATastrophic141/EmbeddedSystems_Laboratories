#include <msp430.h>
#define FLAGS UCA1IFG // Contains the transmit & receive flags
#define RXFLAG UCRXIFG // Receive flag
#define TXFLAG UCTXIFG // Transmit flag
#define TXBUFFER UCA1TXBUF // Transmit buffer
#define RXBUFFER UCA1RXBUF // Receive buffer
#define BUT1 BIT1 // P1.1
#define BUT2 BIT2 // P1.2

unsigned int ascii[10] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57};

/* TODO: FIX NEGATIVE ROLLOVER INTEGERS */

void Initialize_UART();
void uart_write_char(unsigned char ch);
unsigned char uart_read_char();
void uart_write_string(char string[]);
unsigned int get_numSize(unsigned int num);
void uart_write_uint16(unsigned int num);
void parse_int_into_array(unsigned int num, unsigned int numSize, unsigned int* digits);
void Initialize_I2C();
int i2c_read_word(unsigned char i2c_address, unsigned char i2c_reg, unsigned int * data);
int i2c_write_word(unsigned char i2c_address, unsigned char i2c_reg, unsigned int data);
void config_ACLK_to_32KHz_crystal();
void printTime();

char newline[] = "\n\r";
char header[] = "*** LUX MONITOR ***";
char prompt[] = "EDITING TIME: Enter 3 or 4 numbers and then press enter\n\r";
char current[] = "Current lux: ";
char timeSet[] = "Time set to: ";
char timeFail[] = "Failed to set time. Time not changed";
char up[] = " <UP>";
char down[] = " <DOWN>";

unsigned int bench = 0;
unsigned int hours = 0;
unsigned int minutes = 0;
unsigned int seconds = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &=  ~LOCKLPM5; // Enable the GPIO pins

    // Configure the buttons for interrupts
    P1DIR &=  ~(BUT1|BUT2); // 0: input
    P1REN |= (BUT1|BUT2); // 1: enable built-in resistors
    P1OUT |= (BUT1|BUT2); // 1: built-in resistor is pulled up to Vcc
    P1IES |= (BUT1|BUT2); // 1: interrupt on falling edge (0 for rising edge)
    P1IFG &=  ~(BUT1|BUT2); // 0: clear the interrupt flags
    P1IE |= (BUT1|BUT2); // 1: enable the interrupts

    void config_ACLK_to_32KHz_crystal();

    // Configure Channel 0 for up mode with interrupts
    TA0CCR0 = 32767; // 1 second @ 32 KHz
    // Timer_A: ACLK, div by 1, up mode, clear TAR
    TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
    TA0CCTL0 |= CCIE; // Enable Channel 0 CCIE bit
    TA0CCTL0 &= ~CCIFG; // Clear Channel 0 CCIFG bit

    // Enable the global interrupt bit
    _enable_interrupts();

    // Configure pins to backchannel UART
    // Pins: (UCA1TXD / P3.4) (UCA1RXD / P3.5)
    // (P3SEL1=00, P3SEL0=11) (P2DIR=xx)
    P3SEL1 &=  ~(BIT4|BIT5);
    P3SEL0 |= (BIT4|BIT5);
    Initialize_UART();

    // Configure pins to I2C functionality
    // (UCB1SDA same as P4.0) (UCB1SCL same as P4.1)
    // (P4SEL1=11, P4SEL0=00) (P4DIR=xx)
    P4SEL1 |= (BIT1|BIT0);
    P4SEL0 &=  ~(BIT1|BIT0);
    Initialize_I2C();

    //Write config data to OPT3001. Device ID = 0x7F. Config Reg = 0x01. Data = 0x7604
    i2c_write_word(0b1000100, 0x01, 0x7604);

    uart_write_string(header);
    uart_write_string(newline);

    unsigned int data;
    i2c_read_word(0b1000100, 0x00, &data);
    data = data * 1.28;
    bench = data;
    uart_write_string("Initial measurement: ");
    uart_write_uint16(data);
    uart_write_string(newline);

    while(1){}
}

void printTime(){
    uart_write_uint16(hours);
    uart_write_char(':');
    uart_write_uint16(minutes);
    uart_write_char('\t');
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

void uart_write_string(char string[]){
    int i = 0;
    while(string[i] != '\0'){
        uart_write_char(string[i]);
        i++;
    }
    uart_write_char(string[i]);
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
    return;
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

void Initialize_I2C() {
// Configure the MCU in Master mode
// Configure pins to I2C functionality
// (UCB1SDA same as P4.0) (UCB1SCL same as P4.1)
// (P4SEL1=11, P4SEL0=00) (P4DIR=xx)
P4SEL1 |= (BIT1|BIT0);
P4SEL0 &=  ~(BIT1|BIT0);
// Enter reset state and set all fields in this register to zero
UCB1CTLW0 = UCSWRST;
// Fields that should be nonzero are changed below
// (Master Mode: UCMST) (I2C mode: UCMODE_3) (Synchronous mode: UCSYNC)
// (UCSSEL 1:ACLK, 2,3:SMCLK)
UCB1CTLW0 |= UCMST | UCMODE_3 | UCSYNC | UCSSEL_3;
// Clock frequency: SMCLK/8 = 1 MHz/8 = 125 KHz
UCB1BRW = 8;
// Chip Data Sheet p. 53 (Should be 400 KHz max)
// Exit the reset mode at the end of the configuration
UCB1CTLW0 &=  ~UCSWRST;
}

// Read a word (2 bytes) from I2C (address, register
int i2c_read_word(unsigned char i2c_address, unsigned char i2c_reg, unsigned int * data) {

unsigned char byte1, byte2;

// Initialize the bytes to make sure data is received every time
byte1 = 111;
byte2 = 111;

// Write Frame #1
UCB1I2CSA = i2c_address; // Set I2C address
UCB1IFG &=  ~UCTXIFG0;
UCB1CTLW0 |= UCTR; // Master writes (R/W bit = Write)
UCB1CTLW0 |= UCTXSTT; // Initiate the Start Signal

while ((UCB1IFG & UCTXIFG0) ==0) {}
UCB1TXBUF = i2c_reg; // Byte = register address
while((UCB1CTLW0 & UCTXSTT)!=0) {}
if(( UCB1IFG & UCNACKIFG )!=0) return -1;
UCB1CTLW0 &=  ~UCTR; // Master reads (R/W bit = Read)
UCB1CTLW0 |= UCTXSTT; // Initiate a repeated Start Signal

//Read Frame #1
while ( (UCB1IFG & UCRXIFG0) == 0) {}
byte1 = UCB1RXBUF;

// Read Frame #2
while((UCB1CTLW0 & UCTXSTT)!=0) {}
UCB1CTLW0 |= UCTXSTP; // Setup the Stop Signal
while ( (UCB1IFG & UCRXIFG0) == 0) {}
byte2 = UCB1RXBUF;
while ( (UCB1CTLW0 & UCTXSTP) != 0) {}

// Merge the two received bytes
*data = ( (byte1 << 8) | (byte2 & 0xFF) );

return 0;
}

// Write a word (2 bytes) to I2C (address, register)
int i2c_write_word(unsigned char i2c_address, unsigned char i2c_reg, unsigned int data) {

unsigned char byte1, byte2;

byte1 = (data >> 8) & 0xFF; // MSByte
byte2 = data & 0xFF; // LSByte

UCB1I2CSA = i2c_address; // Set I2C address
UCB1CTLW0 |= UCTR; // Master writes (R/W bit = Write)
UCB1CTLW0 |= UCTXSTT; // Initiate the Start Signal
while ((UCB1IFG & UCTXIFG0) ==0) {}
UCB1TXBUF = i2c_reg; // Byte = register address
while((UCB1CTLW0 & UCTXSTT)!=0) {}

// Write Byte #1
UCB1TXBUF = byte1;
while ( (UCB1IFG & UCTXIFG0) == 0) {}

// Write Byte #2
UCB1TXBUF = byte2;
while ( (UCB1IFG & UCTXIFG0) == 0) {}
UCB1CTLW0 |= UCTXSTP;
while ( (UCB1CTLW0 & UCTXSTP) != 0) {}

return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR() {
    //Detect button1
    if((P1IFG & BUT1) == BUT1){
        _delay_cycles (100000);

        unsigned char data;
        unsigned char input;
        int currentIndex = 0;
        unsigned int timeDigits[4] = {0,0,0,0}; //Hours: X0, X1 // Minutes: X2, X3

        uart_write_string(prompt);

        //While enter button was not pressed
        do {
            input = uart_read_char(); //Read input
            if (input != 0){
                data = input; //If input was not null, set data <Allows for null uses>
                if (data != 0 && data != 13 && data > 47 && data < 58 && currentIndex < 5) timeDigits[currentIndex] = data - 48;
                currentIndex++;
            }
        } while (data != '\r');
        if (currentIndex < 3){
            uart_write_string(timeFail);
            uart_write_string(newline);
        }
        else {
            if (currentIndex == 3){
                hours = timeDigits[0];
                minutes = timeDigits[1] * 10 + timeDigits[2];
            }
            else {
                hours = timeDigits[0] * 10 + timeDigits[1];
                minutes = timeDigits[2] * 10 + timeDigits[3];
            }

            uart_write_string(timeSet);
            printTime();
            uart_write_string(newline);
        }

        P1IFG &=  ~(BUT1);
        TA0CTL &= ~TAIFG;
    }
    //Detect button2
    if((P1IFG & BUT2) == BUT2){
        _delay_cycles (100000);

        unsigned int data;
        i2c_read_word(0b1000100, 0x00, &data);
        data = data * 1.28;
        uart_write_string(current);
        uart_write_uint16(data);
        uart_write_string(newline);

        P1IFG &=  ~(BUT2);
        P1IFG &=  ~(BUT1);
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
    seconds++;
    if (seconds == 6){
        seconds = 0;
        minutes++;

        if (minutes == 60){
            minutes = 0;
            hours++;
        }
        if (hours == 24){
            hours = 0;
        }

        printTime();
        unsigned int data;
        int benchOverflow = 0
        i2c_read_word(0b1000100, 0x00, &data);
        data = data * 1.28;
        uart_write_uint16(data);
        if (bench < bench - 10) benchOverflow = -1;
        else if (bench > bench + 10) benchOverflow = 1;
        if (data > bench + 10){
            if (benchOverflow = 1) bench = 65535;
            else bench = data;
            uart_write_string(up);
        }
        if (data < bench - 10){
            if (benchOverflow = -1) bench = 0;
            else bench = data;
             uart_write_string(down);
       }
       uart_write_string(newline);
    }
    TA0CTL &= ~TAIFG;
}
