#include "lockerinterface.h"

#include "FS.h"

#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false
// Numeric display box size and location
#define DISP_X 1
#define DISP_Y 10
#define DISP_W 238
#define DISP_H 50
#define DISP_TSIZE 3
#define DISP_TCOLOR TFT_CYAN

// We have a status line for messages
const int HEIGHT = 320;
const int WIDTH = 240;
const int STATUS_WIDTH = WIDTH;
const int STATUS_HEIGHT = 20;
const int STATUS_X = WIDTH/2; // Centred on this
const int STATUS_Y = HEIGHT - STATUS_HEIGHT;


using namespace fs;

LockerInterface::LockerInterface()
{

}

void LockerInterface::init()
{
  // Initialise the TFT screen
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(0);

  // Calibrate the touch screen and retrieve the scaling factors
  //touch_calibrate();

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  splash();
}

void LockerInterface::splash()
{
  // Draw keypad background
  tft.fillRect(0, 0, WIDTH, HEIGHT - STATUS_HEIGHT, TFT_DARKGREY);
  tft.fillRect(HEIGHT - STATUS_HEIGHT, HEIGHT - STATUS_Y, WIDTH, HEIGHT, TFT_BLACK);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.drawString("PTL", 80,120);
  tft.setFreeFont(&FreeSansBold12pt7b);
  tft.drawString("Locker", 80,166);
}

void LockerInterface::status(const std::string & msg)
{
  tft.setTextPadding(WIDTH);
  //tft.setCursor(STATUS_X, STATUS_Y);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(0);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.drawString(msg.c_str(), STATUS_X, STATUS_Y);
}

void LockerInterface::touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
