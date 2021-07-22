#include "EEPROM_Comm.h"
#define HEXDUMP_LINE 55

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
EEPROM::EEPROM(char fileName[], const int clockTime,int chipSelectPin,
			   const byte *DATA, const byte *ADDR, const byte writePin,
			   const byte OEPin) {
	_fileName = fileName;
	_clockTime = clockTime;
	
	_chipSelectPin = chipSelectPin;
	_DATA = DATA;
	_ADDR = ADDR;
	_writePin = writePin;
	_OEPin = OEPin;
}

EEPROM::EEPROM(char fileName[]) {
	_fileName = fileName;
	_clockTime = 10000;
	
	// Pin A7 is the highest order Data bit (D7)
	static const byte default_DATA[8] = {A7, A6, A5, A4, A3, A2, A1, A0};
	// Pin 47 is the (second) highest order Address bit (A14)
	static const byte default_ADDR[15] = {33, 34, 35, 36, 37, 38, 39, 40,
										 41, 42, 43, 44, 45, 46, 47};

	_chipSelectPin = 53; 
	_DATA = default_DATA;
	_ADDR = default_ADDR;
	_writePin = A9;
	_OEPin = A8;
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
void EEPROM::autoSetup(void){
	Serial.begin(SERIAL_SPEED);
	  
	if (!SD.begin(_chipSelectPin)) {
		Serial.println("SD card initialization failed.");
		_SDCardReady = false;
		_myFile = SD.open(_fileName);
		//while (1);
	} else {
		  Serial.println("SD card initialization complete.");

		  if (!(_myFile = SD.open(_fileName))) {
			Serial.println("Error opening file");
			_SDCardReady = false;
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
void EEPROM::printInternalVals(void) {
	char buffer[101];
	char smallBuffer[6];

	Serial.println();

	// File Name
	sprintf(buffer, "_fileName\t:\t%s", _fileName);
	Serial.println(buffer);

	// Clock Time
	sprintf(buffer, "_clockTime\t:\t%d", _clockTime);
	Serial.println(buffer);

	// Chip Select Pin
	sprintf(buffer, "_chipSelectPin\t:\t%d", _chipSelectPin);
	Serial.println(buffer);

	// Write Enable Pin
	sprintf(buffer, "_writePin\t:\t%d", _writePin);
	Serial.println(buffer);

	// Output Enable Pin
	sprintf(buffer, "_OEPin\t\t:\t%d", _OEPin);
	Serial.println(buffer);

	// Data Lines
	sprintf(buffer, "Data\t\t:\t");
	for (int i=0; i<7; i++) {
		sprintf(smallBuffer, "%d, ", _DATA[i]);
		strcat(buffer, smallBuffer);
	}
	sprintf(smallBuffer, "%d", _DATA[7]);
	strcat(buffer, smallBuffer);
	Serial.println(buffer);

	// Address Lines
	sprintf(buffer, "Address\t\t:\t");
	for (int i=0; i<14; i++) {
		sprintf(smallBuffer, "%d, ", _ADDR[i]);
		strcat(buffer, smallBuffer);
	}
	sprintf(smallBuffer, "%d", _ADDR[14]);
	strcat(buffer, smallBuffer);
	Serial.println(buffer);

	Serial.println();

	return;
}
/************************************************************************/
void EEPROM::startWrite(){
	for(int i=0; i<8; i++){
		pinMode(_DATA[i], OUTPUT);
		digitalWrite(_DATA[i], LOW);
	}
	digitalWrite(_OEPin, HIGH);
	Serial.println("Beginning write:");
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
	Serial.print(printString);
	
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
	for(unsigned int addr = 0x7ff0; addr<=0x7fff && _myFile.available();
		addr++) {
      _myFile.seek(addr);
      writeData(_myFile.peek(), addr);
  }
	return;
}
/************************************************************************/
void EEPROM::startRead(){
	for(int i=0; i<8; i++){
		pinMode(_DATA[i], INPUT);
	}
	digitalWrite(_OEPin, LOW);
	Serial.println("Ready to read values");
	// The delay is to ensure correct reading of the first three-ish addresses
	delay(100);
	return;
}
/************************************************************************/
void EEPROM::readData(long int startAddress, long int howManyAddresses){
	for(long int i = startAddress; i < startAddress + howManyAddresses; i++){
		unsigned long int dataValue = 0;
		char printString[40];
		
		for(int j = 0; j < 8; j++){
			setAddress(i);
			delayMicroseconds(50);
			dataValue = (dataValue << 1) + (digitalRead(_DATA[j]) ? 1 : 0);
		}
		sprintf(printString, "Address:  0x%.4lx   Data: 0x%.2x\n", i, dataValue);
		Serial.print(printString);
	}
	return;
}
/************************************************************************/
void EEPROM::hexdump(int numOfLines){
	// 55 characters per line (plus ending null character)
	char *outString = (char*) calloc(numOfLines * HEXDUMP_LINE, sizeof(char));
	char buffer[6] = {0};
	for (int i = 0; i < numOfLines * 0x10; i+=0x10) {
		
		sprintf(buffer, "%0.4x ", i);
		strcat(outString, buffer);
		
		for (int j = 0; j < 0x10; j++) {
			unsigned int dataValue = 0;
			// Read in all 8 data bits
			for(int k = 0; k < 8; k++){
				setAddress(i+j);
				delayMicroseconds(50);
				dataValue = (dataValue << 1) + (digitalRead(_DATA[k]) ? 1 : 0);
			}
			sprintf(buffer, " %0.2x", dataValue);
			if (j == 0x7) strcat(buffer, " ");
			else if (j == 0xF)
				if (i == numOfLines - 1) strcat (buffer, "\0");
				else strcat(buffer, "\n"); 
			
			strcat(outString, buffer);
		}
	}
	Serial.print(outString);
	return;
}
/************************************************************************/