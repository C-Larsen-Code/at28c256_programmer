#include "EEPROM_Comm.h"

/*************************************************************************
*	Function Definitions
**************************************************************************
*	-EEPROM:		Constructor for instance of this class
*	-setAddress:	Output an address between 0x0000 and 0x7fff to the
*					EEPROM
*	-startWrite:	Puts all eight data lines into output mode (starting at
*					LOW)
*	-startRead:		Puts all eight data lines into input mode
*	-readData:		Prints "howManyAddresses" number of addresses/data
*					stored there, starting from the reference address
*					(which is parameter one)
*	-writeData:		Writes specified data to an address on the EEPROM
*	-writeDataSD:	
*	-autoSetup:		Gets SD card, serial port, and address/data lines ready
*					for use
**************************************************************************/

/*************************************************************************/
EEPROM::EEPROM(char fileName[], const byte clockTime,int chipSelectPin,
			   const byte *DATA, const byte *ADDR, const byte writePin,
			   const byte OEPin) {
	_myFile = SD.open(fileName);
	_clockTime = clockTime;
	_SDCardReady = false;
	
	_chipSelectPin = chipSelectPin;
	_DATA = DATA;
	_ADDR = ADDR;
	_writePin = writePin;
	_OEPin = OEPin;
}
/*************************************************************************/
void EEPROM::setAddress(unsigned int twoByte) {
	//Lowest to highest order bits (pins 53 to 39)
	for(int i=0; i<15; i++){
		digitalWrite(_ADDR[i], (twoByte >> i) & 1);
	}
	return;
}
/************************************************************************/
void EEPROM::startWrite(){
	for(int i=0; i<8; i++){
		pinMode(_DATA[i], OUTPUT);
		digitalWrite(_DATA[i], LOW);
	}
	Serial.println("Beginning write:");
	return;
}
/************************************************************************/
void EEPROM::startRead(){
	for(int i=0; i<8; i++){
		pinMode(_DATA[i], INPUT);
	}
	digitalWrite(_OEPin, LOW);
	Serial.println("Ready to read values");
	return;
}
/************************************************************************/
void EEPROM::readData(int startAddress, int howManyAddresses){
	for(int i = startAddress; i < howManyAddresses; i++){
    unsigned char dataValue;
    char printString[40];
    
    for(int j = 0; j < 8; j++){
      setAddress(i);
      delayMicroseconds(5);
      dataValue = (dataValue << 1) + (digitalRead(_DATA[j]) ? 1 : 0);
    }
    sprintf(printString, "Address:  0x%.4x   Data: 0x%.2x\n", i, dataValue);
    Serial.print(printString);
  }
	return;
}
/************************************************************************/
void EEPROM::writeData(byte inputData, unsigned int address){
	char printString[10];
	for(int i=0; i<8; i++){
		//From highest order bit to lowest order bit
		digitalWrite(_DATA[i], (inputData >> (7-i)) & 1);
		Serial.print((inputData >> (7 - i)) & 1);
		delayMicroseconds(_clockTime);
	}
	Serial.print("  Is the data sent to address ");
	sprintf(printString, "0x%.4x\n", address);
	Serial.println(printString);
	
	digitalWrite(_writePin, HIGH);
	setAddress(address);
	digitalWrite(_writePin, LOW);
	delayMicroseconds(_clockTime);
	digitalWrite(_writePin, HIGH);
	return;
}
/************************************************************************/
void EEPROM::writeDataSD(int programLength){
	if (!_SDCardReady) return;
	
	for(unsigned int addr = 0x0000; addr<programLength && _myFile.available();
		addr++){
		_myFile.seek(addr);
		writeData(_myFile.peek(), addr);
	}
	for(unsigned int addr = 0x7ff0; addr<0x7fff && _myFile.available();
		addr++) {
      _myFile.seek(addr);
      writeData(_myFile.peek(), addr);
  }
	return;
}
/************************************************************************/
void EEPROM::autoSetup(void){
	Serial.begin(SERIAL_SPEED);
	  
	  if (!SD.begin(_chipSelectPin)) {
		Serial.println("SD card initialization failed.");
		//while (1);
	  } else {
		  Serial.println("SD card initialization complete.");

		  if (!_myFile) {
			Serial.println("Error opening file");
			//while(1);
		  } else {
			_SDCardReady = true;
		  }
	  }
	  
	  pinMode(_writePin, OUTPUT);
	  pinMode(_OEPin, OUTPUT);

	  digitalWrite(_writePin, HIGH);
	  digitalWrite(_OEPin, HIGH);

	  for(int i=0; i<15; i++){
		pinMode(_ADDR[i], OUTPUT);
		digitalWrite(_ADDR[i], LOW);
	  }
	  return;
}
/************************************************************************/