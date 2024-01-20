#include "AT28C256_Programmer.h"

/*************************************************************************/
// Some serial print variants to use, depending on if the serial port has
// been initialized or not
/*************************************************************************/

void noSrlPrint(const char str[]) {
    return;
}

void srlPrint(const char str[]) {
    Serial.print(str);
}

void srlPrintLn(const char str[]) {
    Serial.println(str);
}

/*************************************************************************
*    Function Definitions
**************************************************************************
*    -EepromProgrammer: Constructor for instance of this class
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

EepromProgrammer::EepromProgrammer(const byte clockTime, const byte *dataPins,
               const byte *addrPins, const byte writePin, const byte OEPin):
    _clockTime(clockTime), _dataPins(dataPins),
    _addrPins(addrPins), _writePin(writePin), _OEPin(OEPin)
{
    _currCommMode = CommMode::none;

    this->setup();
}

EepromProgrammer::EepromProgrammer() {
    _clockTime = 1000;
    
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

void EepromProgrammer::setAddress(unsigned int twoByte) const {
    //Lowest to highest order bits (pins 53 to 39)
    for(int i=0; i<15; i++){
        digitalWrite(_addrPins[i], (twoByte >> i) & 1);
    }
}

/************************************************************************/

void EepromProgrammer::setup(void) {
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

void EepromProgrammer::startWrite() {
    for(int i=0; i<8; i++){
        pinMode(_dataPins[i], OUTPUT);
        digitalWrite(_dataPins[i], LOW);
    }
    digitalWrite(_OEPin, HIGH);
    this->serialPrintLn("Beginning write:");
    _currCommMode = CommMode::write;
}

/************************************************************************/

void EepromProgrammer::writeByte(byte inputData, unsigned int address) const {
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

void EepromProgrammer::writeData(
    Stream &inpText, unsigned int startAddr, unsigned int finalAddr)
{
    if (_currCommMode != CommMode::write) {
        this->startWrite();
    }

    // Write the main part of the executable to the EEPROM
    for(unsigned int addr = startAddr;
        addr <= finalAddr && addr < 0x7FFF && inpText.available();
        addr++)
    {
        writeByte(inpText.read(), addr);
    }
}

/************************************************************************/

void EepromProgrammer::startRead() {
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

// Imitate the behavior of `hexdump -C`
void EepromProgrammer::hexdump(unsigned int startAddr, unsigned int numOfLines) {
    if (!Serial) return;
    if (_currCommMode != CommMode::read) {
        this->startRead(); 
    }

    // Start lines at the beginning of a 16 byte block (aka the last hex
    // digit of the address is 0)
    startAddr = startAddr >> 4 << 4;

    // 55 characters per line (plus ending null character)
    char buffer[6] = {0};

    // Print lines requested unless they go past the last address (7FFF).
    for (unsigned int i = startAddr;
         i < startAddr + numOfLines * 0x10 && i < 0x7FFF; i+=0x10)
    {
        String outString;

        sprintf(buffer, "%0.4x ", i);
        outString.concat(buffer);
        
        for (unsigned int j = 0; j < 0x10; j++) {
            unsigned int dataValue = 0;
            // Read in all 8 data bits
            for(unsigned int k = 0; k < 8; k++){
                setAddress(i+j);
                delayMicroseconds(50);
                dataValue = (dataValue << 1) + (digitalRead(_dataPins[k]) ? 1 : 0);
            }
            sprintf(buffer, " %0.2x", dataValue);
            if (j == 0x7) strcat(buffer, " ");
            else if (j == 0xF)
                if (i == numOfLines - 1) strcat (buffer, "\0");
                else strcat(buffer, "\n"); 
            
            outString.concat(buffer);
        }
        this->serialPrint(outString.c_str());
    }
}

/************************************************************************/
