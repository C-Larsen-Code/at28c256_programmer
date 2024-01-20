#include "EepromSerialCtrl.h"

/*****************************************************
* Program: Test individual addresses
*       
* Note: For an Arduino Mega, Pins 50-53 are used for
*       SPI, in this case with the SD card. They are
*       as follows:
*       MISO: 50, MOSI: 51, SCK: 52,  SS: 53
*
*****************************************************/

EepromSerialCtrl::EepromSerialCtrl(const byte sdCSPin): sdCSPin(sdCSPin) {
    Serial.begin(115200);
    Serial.flush();

    if (!SD.begin(this->sdCSPin)) {
        currState = CmdLnState::SDCardError;
        Serial.println("SD card initialization failed.");
    } else {
        currState = CmdLnState::ModeSelect;
        Serial.println("SD card initialization complete.");
    }
}

//------------------------------------------------------------------------

void EepromSerialCtrl::runCurrCase() {
    switch(currState) {
        case CmdLnState::SDCardError:
        // Do nothing; just loop until reset
        break;
        case CmdLnState::FileSelect:
        fileSelectCase();
        break;
        case CmdLnState::ModeSelect:
        modeSelectCase();
        break;
        case CmdLnState::Read:
        readCase();
        break;
        case CmdLnState::Write:
        writeCase();
        break;
    }
}

//------------------------------------------------------------------------

void EepromSerialCtrl::modeSelectCase() {
    Serial.print("(r)ead or (w)rite?: ");
    while (Serial.available() == 0) {}  // Wait for them to start typing
    String input = Serial.readString();
    input.trim();
    Serial.print('\n');

    if (input == "r" || input == "R") {
        currState = CmdLnState::Read;
    } else if (input == "w" || input == "W") {
        currState = CmdLnState::FileSelect;
    } else {
        Serial.println("Invalid option provided.");
    }
}

//------------------------------------------------------------------------

void EepromSerialCtrl::fileSelectCase() {
    Serial.println("Available Files:");

    File root = SD.open("/", FILE_READ);

    while (File entry = root.openNextFile()) {
        Serial.print('\t');
        Serial.println(entry.name());
        entry.close();
    }
    
    String input = getInput("Enter file name to use: ");

    file = SD.open(input);
    if (!file) {
        Serial.print(input);
        Serial.println(" is not a valid file.");
        return;
    }

    currState = CmdLnState::Write;
    root.close();
}

//------------------------------------------------------------------------

void EepromSerialCtrl::readCase() {
    unsigned int startAddr;
    unsigned int numOfLines;
    String input;

    input = getInput("Enter starting address: ");

    startAddr = toHexNum(input);

    // The function returns a negative to signify errors
    if (startAddr < 0) {
        Serial.print(input);
        Serial.println(" is not a valid address");
        return;
    }

    input = getInput("Enter number of lines to read (16 addr per line): ");
    numOfLines = input.toInt();

    eeprom.hexdump(startAddr, numOfLines);

    // After the reading is done, return to mode selection.
    currState = CmdLnState::ModeSelect;
}

//------------------------------------------------------------------------

void EepromSerialCtrl::writeCase() {
    String input1 = getInput("Enter the address of the first byte of the program to flash: ");
    int startAddr = toHexNum(input1);
    String input2 = getInput("Enter the address of the last byte of the program to flash: ");
    int finalAddr = toHexNum(input2);
    eeprom.writeData(file, startAddr, finalAddr);
    // After writing, close the file and return to mode selection.
    file.close();
    currState = CmdLnState::ModeSelect;
}

//------------------------------------------------------------------------

String EepromSerialCtrl::getInput(const char *request) {
    Serial.print(request);
    while (Serial.available() == 0) {}
    String input = Serial.readString();
    input.trim();
    Serial.print('\n');
    return input;
}

//------------------------------------------------------------------------

int EepromSerialCtrl::toHexNum(const String input) {
    unsigned int hexNum = 0;
    byte inputLastIdx = input.length() - 1;

    for (int i = 0; i <= inputLastIdx; i++) {
        char inpChar = input[i];
        // The hexadecimal digit must be multiplied by 16 for every spot to
        // the left it is. `shift` tracks how many times to let bit shift a
        // digit in order to multiply it by 16 the proper number of times.
        byte shift = (inputLastIdx - i) * 4;

        if ('a' <= input[i] && input[i] <= 'f') {
            // Add 10 for 'a' and one more for each letter up
            hexNum += (input[i] - 'a' + 10) << shift;
        } else if ('A' <= input[i] && input[i] <= 'F') {
            // Add 10 for 'A' and one more for each letter up
            hexNum += (input[i] - 'A' + 10) << shift;
        } else if ('0' <= input[i] && input[i] <= '9') {
            hexNum += (input[i] - '0') << shift;
        } else {
            // If a character doesn't match any of the above, it is not a
            // valid hexadecimal digit. Return the function with an error.
            return -1;
        }
    }
    //
    return hexNum;
}
