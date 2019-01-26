/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _RFIDREADER_H
#define _RFIDREADER_H

#include <MFRC522.h>

#include "Pins.h"

class RfidReader
{
public:

    //RfidReader() : mfrc522(PIN_RFID_SS, PIN_RFID_RESET) //testing a new version of the MFRC522 library,,no need for reset !
    RfidReader() : mfrc522()
    {
    }

    void begin()
    {
        //mfrc522.PCD_Init();
        mfrc522.PCD_Init(PIN_RFID_SS,UINT8_MAX);
    }
    void restart()
    {
      mfrc522.PCD_Reset();
      digitalWrite(PIN_RFID_RESET, LOW); //Hard reset
      begin();
    }

    char* tryRead();

private:

    byte parseHexNibble(char);
    byte parseHexByte(char*);
    byte computeCheckSum(char*);

    char buffer[21];
    MFRC522 mfrc522;
};

#endif
