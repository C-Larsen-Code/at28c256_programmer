#include "EEPROM_Comm.h"
#define HEXDUMP_LINE 55

/*************************************************************************/
// Some serial print variants to use, depending on if the serial port has
// been initialized or not
/*************************************************************************/

void noSrlPrint(char str[]) {
    return;
}

void srlPrint(char str[]) {
    Serial.print(str);
}

void srlPrintLn(char str[]) {
    Serial.println(str);
}

/*************************************************************************
*    Function Definitions
**************************************************************************
*    -EEPROM:        Constructor for instance of this class
*    -setAddress:    Output an address between 0x0000 and 0x7fff to the
*                    EEPROM
*    -startWrite:    Puts all eight data lines into output mode (starting at
*                    LOW)
*    -startRead:     Puts all eight data lines into input mode
*    -readData:      Prints "howManyAddresses" number of addresses/data
*                    stored there, starting from the reference address
*                    (which is parameter one)
*    -writeByte:     Writes specified data to an address on the EEPROM
*    -writeData:    
**************************************************************************/

/*************************************************************************/

EEPROM::EEPROM(const byte clockTime, const byte *dataPins,
               const byte *addrPins, const byte writePin, const byte OEPin):
    _clockTime(clockTime), _dataPins(dataPins),
    _addrPins(addrPins), _writePin(writePin), _OEPin(OEPin)
{
    _currCommMode = CommMode::none;

    this->setup();
}

EEPROM::EEPROM() {
    _clockTime = 255;
    
    // Pin A7 is the highest order Data bit (D7)
    static const byte defaultDataPins[8] = {A7, A6, A5, A4, A3, A2, A1, A0};
    // Pin 47 is the (second) highest order Address bit (A14)
    static const byte defaultAddrPins[15] = {33, 34, 35, 36, 37, 38, 39, 40,
                                             41, 42, 43, 44, 45, 46, 47};

    _dataPins = defaultDataPins;
    _addrPins = defaultAddrPins;
    _writePin = A9;
    _OEPin = A8;

    _currCommMode = CommMode::none;

    this->setup();
}

/*************************************************************************/

void EEPROM::setAddress(unsigned int twoByte) const {
    //Lowest to highest order bits (pins 53 to 39)
    for(int i=0; i<15; i++){
        digitalWrite(_addrPins[i], (twoByte >> i) & 1);
    }
}

/************************************************************************/

void EEPROM::setup(void) {
    pinMode(_writePin, OUTPUT);
    pinMode(_OEPin, OUTPUT);

    digitalWrite(_writePin, HIGH);
    digitalWrite(_OEPin, HIGH);

    for(int i=0; i<15; i++){
    pinMode(_addrPins[i], OUTPUT);
    digitalWrite(_addrPins[i], LOW);
    }

    if (Serial) {
        serialPrint = &srlPrint;
        serialPrintLn = &srlPrintLn;
    } else {
        serialPrint = &noSrlPrint;
        serialPrintLn = &noSrlPrint;
    }
}

/************************************************************************/

void EEPROM::printInternalVals(void) const {
    char buffer[101];
    char smallBuffer[6];

    // Clock Time
    sprintf(buffer, "_clockTime\t:\t%d", _clockTime);
    this->serialPrintLn(buffer);

    // Write Enable Pin
    sprintf(buffer, "_writePin\t:\t%d", _writePin);
    this->serialPrintLn(buffer);

    // Output Enable Pin
    sprintf(buffer, "_OEPin\t\t:\t%d", _OEPin);
    this->serialPrintLn(buffer);

    // Data Lines
    sprintf(buffer, "Data\t\t:\t");
    for (int i=0; i<7; i++) {
        sprintf(smallBuffer, "%d, ", _dataPins[i]);
        strcat(buffer, smallBuffer);
    }
    sprintf(smallBuffer, "%d", _dataPins[7]);
    strcat(buffer, smallBuffer);
    this->serialPrintLn(buffer);

    // Address Lines
    sprintf(buffer, "Address\t\t:\t");
    for (int i=0; i<14; i++) {
        sprintf(smallBuffer, "%d, ", _addrPins[i]);
        strcat(buffer, smallBuffer);
    }
    sprintf(smallBuffer, "%d", _addrPins[14]);
    strcat(buffer, smallBuffer);
    this->serialPrintLn(buffer);
}

/************************************************************************/

void EEPROM::startWrite() {
    for(int i=0; i<8; i++){
        pinMode(_dataPins[i], OUTPUT);
        digitalWrite(_dataPins[i], LOW);
    }
    digitalWrite(_OEPin, HIGH);
    this->serialPrintLn("Beginning write:");
    _currCommMode = CommMode::write;
}

/************************************************************************/

void EEPROM::writeByte(byte inputData, unsigned int address) const {
    char printString[10];
    for(int i=0; i<8; i++){
        //From highest order bit to lowest order bit
        digitalWrite(_dataPins[i], (inputData >> (7-i)) & 1);
        delayMicroseconds(_clockTime);
    }
    this->serialPrint("  Is the data sent to address ");
    if (Serial) sprintf(printString, "0x%.4x\n", address);
    this->serialPrint(printString);
    
    digitalWrite(_writePin, HIGH);
    setAddress(address);
    digitalWrite(_writePin, LOW);
    delayMicroseconds(_clockTime);
    digitalWrite(_writePin, HIGH);
}

/************************************************************************/

void EEPROM::writeData(Stream &inpText, int programLength) {
    if (_currCommMode != CommMode::write) {
        this->startWrite();
    }

    // Write the main part of the executable to the EEPROM
    for(unsigned int addr = 0x0000; addr<programLength && inpText.available();
        addr++){
        writeByte(inpText.read(), addr);
    }
}

/************************************************************************/

void EEPROM::startRead() {
    // Only start a read if Serial output has been enabled
    if (!Serial) return;

    for(int i=0; i<8; i++){
        pinMode(_dataPins[i], INPUT);
    }
    digitalWrite(_OEPin, LOW);
    // The delay is to ensure correct reading of the first three-ish addresses
    delay(100);
    _currCommMode = CommMode::read;
}

/************************************************************************/

void EEPROM::readData(long int startAddress, long int howManyAddresses) {
    if (!Serial) return;
    if (_currCommMode != CommMode::read) {
        this->startRead(); 
    }

    for(long int i = startAddress; i < startAddress + howManyAddresses; i++){
        unsigned long int dataValue = 0;
        char printString[40];
        
        for(int j = 0; j < 8; j++){
            setAddress(i);
            delayMicroseconds(50);
            dataValue = (dataValue << 1) + (digitalRead(_dataPins[j]) ? 1 : 0);
        }
        sprintf(printString, "Address:  0x%.4lx   Data: 0x%.2x\n", i, dataValue);
        this->serialPrint(printString);
    }
}

/************************************************************************/

void EEPROM::hexdump(int numOfLines) const {
    if (!Serial) return;

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
                dataValue = (dataValue << 1) + (digitalRead(_dataPins[k]) ? 1 : 0);
            }
            sprintf(buffer, " %0.2x", dataValue);
            if (j == 0x7) strcat(buffer, " ");
            else if (j == 0xF)
                if (i == numOfLines - 1) strcat (buffer, "\0");
                else strcat(buffer, "\n"); 
            
            strcat(outString, buffer);
        }
    }
    this->serialPrint(outString);
}

/************************************************************************/
