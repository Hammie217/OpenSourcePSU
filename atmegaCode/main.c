#ifndef F_CPU					// if F_CPU was not defined in Project -> Properties
#define F_CPU 1000000UL			// define it now as 1 MHz unsigned long
#endif

//Define SPI Pins and functions
#define SPI_DDR DDRB
#define CS      PINB1
#define MOSI    PINB3
#define MISO    PINB4
#define SCK     PINB5
#define SPI_SLAVE_SELECTED PORTB &= ~(1 << CS)
#define SPI_SLAVE_DESELECTED PORTB |= (1 << CS)

#include <avr/io.h>				// this is always included in AVR programs
#include <util/delay.h>			// add this to use the delay function

void SPI_init()
{
	SPI_DDR |= (1 << PINB2) | (1 << CS) | (1 << MOSI) | (1 << SCK);// set CS, MOSI and SCK to output
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);	// enable SPI, set as master, and clock to fosc/128
}

void TWIInit(void)
{
	//set SCL to 400kHz
	TWSR = 0x00;
	TWBR = 0x0C;
	//enable TWI
	TWCR = (1<<TWEN);
}

void TWIStart(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}
//send stop signal
void TWIStop(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void TWIWrite(uint8_t u8data)
{
	TWDR = u8data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

uint8_t TWIReadACK(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}
//read byte with NACK
uint8_t TWIReadNACK(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}

uint8_t TWIGetStatus(void)
{
	uint8_t status;
	//mask status
	status = TWSR & 0xF8;
	return status;
}


void SPI_MasterTransmit(uint8_t cData)
{
	SPDR = cData;//Start data send
	while(!(SPSR & (1<<SPIF)));//wait for completion flag
}

void outputVoltage(uint16_t outputVal){
	outputVal &= 0x0FFF;
	outputVal |= 0x3000;
	uint8_t msbTransfer = outputVal >> 8;
	uint8_t LSBTransfer = outputVal;
	SPI_SLAVE_SELECTED;
	SPI_MasterTransmit(msbTransfer);
	SPI_MasterTransmit(LSBTransfer);
	SPI_SLAVE_DESELECTED;
}

void outputCurrent(uint16_t outputVal){
	outputVal &= 0x0FFF;
	outputVal |= 0xB000;
	uint8_t msbTransfer = outputVal >> 8;
	uint8_t LSBTransfer = outputVal;
	SPI_SLAVE_SELECTED;
	SPI_MasterTransmit(msbTransfer);
	SPI_MasterTransmit(LSBTransfer);
	SPI_SLAVE_DESELECTED;
}

void initINA219(void){
	TWIStart();
		TWIWrite(0B10000000);
		TWIWrite(0B00000000);
		TWIWrite(0B00111001);
		TWIWrite(0B10011101);
	TWIStop();
	_delay_ms(10);
	TWIStart();
		TWIWrite(0B10000000);
		TWIWrite(0B00000001);
	TWIStop();
	//
}

uint16_t currentCurrent(void){
	uint16_t readCurrent =0;
	TWIStart();
	TWIWrite(0B10000001);
	readCurrent = (TWIReadACK()<<8);
	readCurrent |= TWIReadNACK();//current is readval/10 mA
	TWIStop();
	return readCurrent;
}

void turnLEDON(){
	PORTC |= (1 << PC0);	// LED ON
}

void invertLEDState(){
	PORTC ^= (1 << PC0);	// LED ON
}

int main(void) {
	
	DDRC |= (1 << PC0);			// set Port C pin PC0 for output
	SPI_init();
	TWIInit();
	uint16_t outputValue=200*5;//4095 max
	uint16_t outputValue2=0x07FF;//4095 max
	uint16_t readCurrent =0;
	
	initINA219();
	_delay_ms(10);
	
	while (1) {
		outputVoltage(outputValue);
		outputCurrent(outputValue2);
		readCurrent = currentCurrent();
		if(readCurrent>500){
			turnLEDON();
		}
		if(readCurrent<500){
			invertLEDState();
		}
		_delay_ms(50);
	}
	return(0);					
}

//current LSB = 1/2^15=0.00003051757
//Config bytes 0011100110011101


/*

*/