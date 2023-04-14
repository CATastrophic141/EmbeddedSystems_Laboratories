#include <msp430.h>
#define FLAGS UCA1IFG // Contains the transmit & receive flags
#define RXFLAG UCRXIFG // Receive flag
#define TXFLAG UCTXIFG // Transmit flag
#define TXBUFFER UCA1TXBUF // Transmit buffer
#define RXBUFFER UCA1RXBUF // Receive buffer
#define redLED BIT0 // Red LED at P1.0
#define BUT1 BIT1 // P1.1
#define BUT2 BIT2 // P1.2

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

unsigned int ascii[10] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57};

//Test file that swaps colors of multicolor LED using pulse-width modulation
//According to slau599b:
//J4.37 -> Blue -> P3.6 -> TB0.2
//J4.38 -> Green -> P3.3 -> TA1.1
//J.39 -> Red -> P2.6 -> TB0.5

//ACLK -> 32KHz
//PWN -> ~1000 Hz
//Cycles -> ~34

char newline[] = "\n\r";

unsigned int thresh1;
unsigned int thresh2;

unsigned int hours = 0;
unsigned int minutes = 0;
unsigned int seconds = 0;
unsigned int secondsInStage0;
unsigned int secondsInStage1;
unsigned int secondsInStage2;

int rgb_r[] = {10,0,0};
int rgb_o[] = {10,1,0};
int rgb_y[] = {10,4,0};
int rgb_g[] = {0,10,0};
int rgb_b[] = {0,0,10};
int rgb_p[] = {3,0,2};
int rgb_w[] = {5,5,5};
int rgb_off[] = {0,0,0};

int* stage0;
int* stage1;
int* stage2;

int currentStage;
int isEditing;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	PM5CTL0 &=  ~LOCKLPM5; // Enable the GPIO pins

	//Active LED indicator
	P1DIR |= redLED; // Direct pin as output
	    P1OUT &=  ~redLED; // Turn LED Off

	// Configure the buttons for interrupts
	P1DIR &=  ~(BUT1|BUT2); // 0: input
	P1REN |= (BUT1|BUT2); // 1: enable built-in resistors
	P1OUT |= (BUT1|BUT2); // 1: built-in resistor is pulled up to Vcc
	P1IES |= (BUT1|BUT2); // 1: interrupt on falling edge (0 for rising edge)
	P1IFG &=  ~(BUT1|BUT2); // 0: clear the interrupt flags
	P1IE |= (BUT1|BUT2); // 1: enable the interrupts
	_enable_interrupts();

	//Initializers
    config_ACLK_to_32KHz_crystal();
    //P3SEL1 &=  ~(BIT4|BIT5);
    //P3SEL0 |= (BIT4|BIT5);
    Initialize_UART();
    //P4SEL1 |= (BIT1|BIT0);
    //P4SEL0 &=  ~(BIT1|BIT0);
    Initialize_I2C();

    //Red LED settings
    //J.39 -> Red -> P2.6 -> TB0.5
    P2DIR |= BIT6;
    P2SEL1 &= ~BIT6;
    P2SEL0 |= BIT6;

    //Blue LED settings
    //J4.37 -> Blue -> P3.6 -> TB0.2
    P3DIR |= BIT6;
    P3SEL1 |= BIT6;
    P3SEL0 &= ~BIT6;

    //Green LED settings
    //J4.38 -> Green -> P3.3 -> TA1.1
    P3DIR |= BIT3;
    P3SEL1 |= BIT3;
    P3SEL0 &= ~BIT3;

    //OUTMOD settings
        //Red -> TB0.5
    TB0CCTL5 |= OUTMOD_7;   // Reset/Set
        //Blue -> TB0.2
    TB0CCTL2 |= OUTMOD_7;
        //Green
    TA1CCTL1 |= OUTMOD_7;

    //Write config data to OPT3001. Device ID = 0x7F. Config Reg = 0x01. Data = 0x7604
    i2c_write_word(0b1000100, 0x01, 0x7604);

    uart_write_string("*** CUSTOM LIGHT MONITOR AND DISPLAY ***");
    uart_write_string(newline);
    uart_write_string("Default colors:\n\rStage 0 -> Red\n\rStage 1 -> Greed\n\rStage 2 -> Blue");
    uart_write_string(newline);
    uart_write_string("Default lux thresholds:\n\rStage 0 -> 0\n\rStage 1 -> 100\n\rStage 2 -> 200");
    uart_write_string(newline);
    uart_write_string("Set time: t\tSet thresholds: c");
    uart_write_string(newline);

    //First measurement
    unsigned int data;
    i2c_read_word(0b1000100, 0x00, &data);
    data = data * 1.28;
    uart_write_string("Initial measurement: ");
    uart_write_uint16(data);
    uart_write_string(newline);

    stage0 = rgb_r;
    stage1 = rgb_g;
    stage2 = rgb_b;

    thresh1 = 100;
    thresh2 = 200;

    //Timer & PWM settings
    TA1CCR0 = 34;  //Overall cycles for TA1 1
    TB0CCR0 = 34; //Overall cycles for TB0 2 and 5

    //Initial stage markers
    if(data < thresh1) {
        TB0CCR5 = stage0[0];    //RED
        TA1CCR1 = stage0[1];   // GREEN
        TB0CCR2 = stage0[2];  //BLUE
        currentStage = 0;
    }
    else if (data < thresh2 && data >= thresh1){
        TB0CCR5 = stage1[0];    //RED
        TA1CCR1 = stage1[1];   // GREEN
        TB0CCR2 = stage1[2];  //BLUE
        currentStage = 1;
    }
    else {
        TB0CCR5 = stage2[0];    //RED
        TA1CCR1 = stage2[1];   // GREEN
        TB0CCR2 = stage2[2];  //BLUE
        currentStage = 2;
    }

    //Start PWM
    TA1CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
    TB0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;

    // Configure Channel 0 for up mode with interrupts
    TA0CCR0 = 32767; // 1 second @ 32 KHz
    // Timer_A: ACLK, div by 1, up mode, clear TAR
    TA0CTL = TASSEL_1 | ID_0 | MC_1 | TACLR;
    TA0CCTL0 |= CCIE; // Enable Channel 0 CCIE bit
    TA0CCTL0 &= ~CCIFG; // Clear Channel 0 CCIFG bit

    secondsInStage0 = 0;
    secondsInStage1 = 0;
    secondsInStage2 = 0;

    char changeInput;
    isEditing = 0;

    while(1) {
        _delay_cycles(10000);

        changeInput = uart_read_char(); //Read input

        if (changeInput == 116){
               isEditing = 1;

               unsigned char data;
               unsigned char input;
               unsigned int currentIndex = 0;
               //Array for parsing input
               unsigned int timeDigits[4] = {0,0,0,0}; //Hours: X0, X1 // Minutes: X2, X3

               uart_write_string("EDITING TIME: Enter 3 or 4 numbers and then press enter\n\r");

               //While enter button was not pressed
               do {
                   input = uart_read_char(); //Read input
                   if (input != 0){
                       data = input; //If input was not null, set data <Allows for null uses>
                       //It data is placeable in index and is a ASCII number, save as actual number value
                       if (data != 0 && data != 13 && data > 47 && data < 58 && currentIndex < 5){ timeDigits[currentIndex] = data - 48;
                       currentIndex++;
                       }
                   }
               } while (data != '\r');  //End entry upon 'enter' key
               if (currentIndex < 3){
                   uart_write_string("Failed to set time. Time not changed"); //Do not save input
                   uart_write_string(newline);
               }
               else {  //Save input
                   if (currentIndex == 3){
                       hours = timeDigits[0];  //Entered hour is only one number
                       minutes = timeDigits[1] * 10 + timeDigits[2];
                   }
                   else {
                       hours = timeDigits[0] * 10 + timeDigits[1]; //Entered hour is two numbers
                       minutes = timeDigits[2] * 10 + timeDigits[3];
                   }

                   uart_write_string("Time set to: ");
                   printTime();
                   uart_write_string(newline);
               }
        }

        else if (changeInput == 99){
            isEditing = 1;

            unsigned char input;
            unsigned int currentIndex = 0;
            unsigned int temp1[4] = {0,0,0,0};

                    uart_write_string("Setting threshold values: [Threshold 2 must be greater than threshold 1]\n\r");
                    uart_write_string("Threshold 1:\n\r");
                    do {
                       input = uart_read_char(); //Read input
                        if (input != 0){
                            //It data is placeable in index and is a ASCII number, save as actual number value
                            if (input != 0 && input != 13 && input > 47 && input < 58 && currentIndex < 5){ temp1[currentIndex] = input - 48;
                            currentIndex++;
                            }
                        }
                    } while (input != '\r');  //End entry upon 'enter' key
                    if (currentIndex >= 4){
                        thresh1 = temp1[0]*1000 + temp1[1]*100 + temp1[2]*10 + temp1[3];
                    }
                    else if (currentIndex == 3){
                        thresh1 = temp1[0]*100 + temp1[1]*10 + temp1[2];
                    }
                    else if (currentIndex == 2){
                        thresh1 = temp1[0]*10 + temp1[1];
                    }
                    else thresh1 = temp1[0];
                    uart_write_string("Threshold 1 set to: ");
                    uart_write_uint16(thresh1);
                    uart_write_string(newline);

                    input = 0;
                    currentIndex = 0;
                    unsigned int temp2[] = {0,0,0,0};

                    uart_write_string("Threshold 2:\n\r");
                    do {
                        input = uart_read_char(); //Read input
                        if (input != 0){
                           //It data is placeable in index and is a ASCII number, save as actual number value
                           if (input != 13 && input > 47 && input < 58 && currentIndex < 5){ temp2[currentIndex] = input - 48;
                           currentIndex++;
                           }
                        }
                   } while (input != '\r');  //End entry upon 'enter' key
                   if (currentIndex >= 4){
                        thresh2 = temp2[0]*1000 + temp2[1]*100 + temp2[2]*10 + temp2[3];
                   }
                   else if (currentIndex == 3){
                        thresh2 = temp2[0]*100 + temp2[1]*10 + temp2[2];
                   }
                   else if (currentIndex == 2){
                        thresh2 = temp2[0]*10 + temp2[1];
                   }
                   else thresh2 = temp2[0];
                   if (thresh2 <= thresh1){
                       uart_write_string("Failed to set second threshold to value above first threshold. Defaulting threshold value");
                       uart_write_string(newline);
                       thresh2 = thresh1 + 1;
                   }
                   uart_write_string("Threshold 2 set to: ");
                   uart_write_uint16(thresh2);
                   uart_write_string(newline);

                   uart_write_string("EDITING STAGE COLORS: type corresponding letter (lowercase only)\n\rRed = r ; Orange = o ; Yellow = y ; Green = g ; Blue = b ; Purple = p ; White = w ; OFF = x");
                   uart_write_string(newline);
                   int flag = 0;
                   input = 0;
                   uart_write_string("Setting stage 0 color:\n\r");
                   do {
                        input = uart_read_char(); //Read input
                        if (input != 0){
                            //r o y g b w x
                            if (input == 114 || input == 111 || input == 121 || input == 103 || input == 98 || input == 112|| input == 119 || input == 120) {
                                flag = 1;
                                switch(input){
                                    case 114: stage0 = rgb_r; break;
                                    case 111: stage0 = rgb_o; break;
                                    case 121: stage0 = rgb_y; break;
                                    case 103: stage0 = rgb_g; break;
                                    case 98: stage0 = rgb_b; break;
                                    case 112: stage0 = rgb_p; break;
                                    case 119: stage0 = rgb_w; break;
                                    case 120: stage0 = rgb_off; break;
                                }
                                uart_write_string("Set stage color to: ");
                                uart_write_char( (char)input );
                                uart_write_string(newline);
                            }
                        }
                  } while (flag != 1);  //End entry upon correct input
                  flag = 0;
                  input = 0;
                  uart_write_string("Setting stage 1 color:\n\r");
                  do {
                       input = uart_read_char(); //Read input
                       if (input != 0){
                           //r o y g b w x
                           if (input == 114 || input == 111 || input == 121 || input == 103 || input == 98 || input == 112|| input == 119 || input == 120) {
                               flag = 1;
                               switch(input){
                                   case 114: stage1 = rgb_r; break;
                                   case 111: stage1 = rgb_o; break;
                                   case 121: stage1 = rgb_y; break;
                                   case 103: stage1 = rgb_g; break;
                                   case 98: stage1 = rgb_b; break;
                                   case 112: stage1 = rgb_p; break;
                                   case 119: stage1 = rgb_w; break;
                                   case 120: stage1 = rgb_off; break;
                               }
                               uart_write_string("Set stage color to: ");
                               uart_write_char( (char)input );
                               uart_write_string(newline);
                           }
                       }
                  } while (flag != 1);  //End entry upon correct input
                  flag = 0;
                  uart_write_string("Setting stage 2 color:\n\r");
                  do {
                       input = uart_read_char(); //Read input
                       if (input != 0){
                       //It data is placeable in index and is a ASCII number, save as actual number value
                           //r o y g b w x
                           if (input == 114 || input == 111 || input == 121 || input == 103 || input == 98 || input == 112|| input == 119 || input == 120) {
                               flag = 1;
                               switch(input){
                                   case 114: stage2 = rgb_r; break;
                                   case 111: stage2 = rgb_o; break;
                                   case 121: stage2 = rgb_y; break;
                                   case 103: stage2 = rgb_g; break;
                                   case 98: stage2 = rgb_b; break;
                                   case 112: stage2 = rgb_p; break;
                                   case 119: stage2 = rgb_w; break;
                                   case 120: stage2 = rgb_off; break;
                               }
                               uart_write_string("Set stage color to: ");
                               uart_write_char( (char)input );
                               uart_write_string(newline);
                           }
                       }
                 } while (flag != 1);  //End entry upon corect input
                 flag = 0;
                 input = 0;
        }

        else if (changeInput == 45) {
            secondsInStage0 = 0;
            secondsInStage1 = 0;
            secondsInStage2 = 0;
            uart_write_string("Cleared stage time recordings");
            uart_write_string(newline);
        }

        changeInput = 0;
        isEditing = 0;
    }
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

void printTime(){
    uart_write_uint16(hours);
    uart_write_char(':');
    uart_write_uint16(minutes);
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

void checkThreshChange(unsigned int data, unsigned int currStage, unsigned int t1, unsigned int t2) {
    //Depending on the current stage, check if the reading passed either of the threshold values. If so, update LED PWM settings and stage marker
    if (currStage == 0){
        if (data >= t1 && data < t2) {
            TB0CCR5 = stage1[0];    //RED
            TA1CCR1 = stage1[1];   // GREEN
            TB0CCR2 = stage1[2];  //BLUE
            currentStage = 1;
            uart_write_string("Reading crossed threshold 1 at ");
            printTime();
            uart_write_string(" to ");
            uart_write_uint16(data);
            uart_write_string(newline);
        }
        else if (data >= t2){
            TB0CCR5 = stage2[0];    //RED
            TA1CCR1 = stage2[1];   // GREEN
            TB0CCR2 = stage2[2];  //BLUE
            currentStage = 2;
            uart_write_string("Reading crossed threshold 2 at ");
            printTime();
            uart_write_string(" to ");
            uart_write_uint16(data);
            uart_write_string(newline);
        }
    }
    else if (currStage == 1){
        if (data < t1) {
            TB0CCR5 = stage0[0];    //RED
            TA1CCR1 = stage0[1];   // GREEN
            TB0CCR2 = stage0[2];  //BLUE
            currentStage = 0;
            uart_write_string("Reading crossed threshold 0 at ");
            printTime();
            uart_write_string(" to ");
            uart_write_uint16(data);
            uart_write_string(newline);
        }
        else if (data >= t2){
            TB0CCR5 = stage2[0];    //RED
            TA1CCR1 = stage2[1];   // GREEN
            TB0CCR2 = stage2[2];  //BLUE
            currentStage = 2;
            uart_write_string("Reading crossed threshold 2 at ");
            printTime();
            uart_write_string(" to ");
            uart_write_uint16(data);
            uart_write_string(newline);
        }
    }
    else if (currStage == 2){
        if (data < t1) {
            TB0CCR5 = stage0[0];    //RED
            TA1CCR1 = stage0[1];   // GREEN
            TB0CCR2 = stage0[2];  //BLUE
            currentStage = 0;
            uart_write_string("Reading crossed threshold 0 at ");
            printTime();
            uart_write_string(" to ");
            uart_write_uint16(data);
            uart_write_string(newline);
        }
        if (data >= t1 && data < t2) {
            TB0CCR5 = stage1[0];    //RED
            TA1CCR1 = stage1[1];   // GREEN
            TB0CCR2 = stage1[2];  //BLUE
            currentStage = 1;
            uart_write_string("Reading crossed threshold 1 at ");
            printTime();
            uart_write_string(" to ");
            uart_write_uint16(data);
            uart_write_string(newline);
        }
    }
    else uart_write_string("ERROR: DEBUG PRINT\n\r");
}

#pragma vector = PORT1_VECTOR
__interrupt void Port1_ISR() {
    //Detect button1
    if((P1IFG & BUT1) == BUT1){
        _delay_cycles (100000); //Debounce

        //Print amount of seconds spent in each stage
        uart_write_string("Time in seconds spent in stage 0: ");
        uart_write_uint16(secondsInStage0);
        uart_write_string(newline);
        uart_write_string("Time in seconds spent in stage 1: ");
        uart_write_uint16(secondsInStage1);
        uart_write_string(newline);
        uart_write_string("Time in seconds spent in stage 2: ");
        uart_write_uint16(secondsInStage2);
        uart_write_string(newline);

        P1IFG &=  ~(BUT1);
        //TA0CTL &= ~TAIFG;
    }
    //Detect button2
    if((P1IFG & BUT2) == BUT2){
        _delay_cycles (100000); //Debounce

        //Print current reading
        unsigned int data;
        i2c_read_word(0b1000100, 0x00, &data);
        data = data * 1.28;
        uart_write_string("Current lux: ");
        uart_write_uint16(data);
        uart_write_string(newline);

        P1IFG &=  ~(BUT2);
        //TA0CTL &= ~TAIFG;
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T0A0_ISR() {
    //Increment seconds
    seconds++;

    //Increment second counter of current stage
    if (currentStage == 0) secondsInStage0++;
    else if (currentStage == 1) secondsInStage1++;
    else if (currentStage == 1) secondsInStage2++;

    //Active program marker
    P1OUT ^= redLED;

    //Get data
    unsigned int data;
    i2c_read_word(0b1000100, 0x00, &data);
    data = data * 1.28;

    //Check threshold passage if now currently editing settings
    if (isEditing == 0) checkThreshChange(data, currentStage, thresh1, thresh2);

    if (seconds == 60){ //Count to 60 seconds
        seconds = 0;
        minutes++;

        if (minutes == 60){ //Count to one hour
            minutes = 0;
            hours++;
        }

        if (hours == 24){   //Count to one day
            hours = 0;
        }

        //Print current status if not currently editing
        if(isEditing == 0){
        printTime();    //Print current hour and minute measurement
        uart_write_string(" ");
        uart_write_uint16(data);
        uart_write_string(" (Current stage: ");
        uart_write_uint16(currentStage);
        uart_write_char(')');
        uart_write_string(newline);
        }
    }
    TA0CTL &= ~TAIFG;
}
