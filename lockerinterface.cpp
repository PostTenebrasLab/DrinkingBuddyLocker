#include "lockerinterface.h"

#include "FS.h"

#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library



#define CALIBRATION_FILE "/TouchCalData1"
// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
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

const int DOWN = -1;
const int UP = -2;

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
  touch_calibrate();

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  splash();


}

void LockerInterface::splash()
{
  // Draw keypad background
  tft.fillRect(0, 0, WIDTH, HEIGHT - STATUS_HEIGHT, TFT_DARKGREY);
  clear_status_bar();

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.drawString("PTL", 80,120);
  tft.setFreeFont(&FreeSansBold12pt7b);
  tft.drawString("Locker", 80,166);
}

void LockerInterface::clear_status_bar()
{
  tft.fillRect(HEIGHT - STATUS_HEIGHT, HEIGHT - STATUS_Y, WIDTH, HEIGHT, TFT_BLACK);
}

void LockerInterface::swipe_prompt()
{
  // Draw keypad background
  tft.fillRect(0, 0, WIDTH, HEIGHT - STATUS_HEIGHT, TFT_DARKGREY);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.drawString("SWIPE", 120,120);
  tft.drawString("CARD", 120,166);
  
}

void LockerInterface::set_selection(std::vector<int> selection_)
{
  selection.clear();
  selection = selection_;
}

void LockerInterface::show_selector(int page)
{
  
  Serial.println("Show selector");
  tft.fillRect(0, 0, WIDTH, HEIGHT - STATUS_HEIGHT, TFT_BLACK);
  displayed_page = page;
  buttons.clear();
  buttons_idx.clear();
  const int num_per_page = 3;
  tft.setFreeFont(&FreeSansBold12pt7b);
  int num_dat = selection.size();
  int start_idx = num_per_page*page;
  start_idx = (start_idx > num_dat)?(num_dat/num_per_page-1)*num_per_page:start_idx;
  int stop_idx = start_idx + num_per_page;
  stop_idx = (stop_idx > num_dat)?num_dat:stop_idx;

  int bt_idx = 0;
  int x1 = WIDTH/2;
  int width = WIDTH/2;
  int y0 = 30;
  int height = 48;
  int offset = 50;
  sprintf(strbuf, "start: %d stop: %d num: %d", start_idx, stop_idx, num_dat);
  
  Serial.println(strbuf);
  for (int i = start_idx; i < stop_idx; i++)
    {
      int y1 = (bt_idx +1)* offset + y0;
      
      sprintf(strbuf, "%d", selection[i]);
      
      buttons.emplace_back(TFT_eSPI_Button());
      buttons_idx.emplace_back(i);
      buttons.back().initButton(&tft, x1,y1, width, height, TFT_WHITE, TFT_DARKGREY, TFT_WHITE, strbuf, 0);
      bt_idx++;
    }

  if (start_idx > 0)
    {
      buttons.emplace_back(TFT_eSPI_Button());
      buttons_idx.emplace_back(DOWN);
      buttons.back().initButton(&tft, x1,y0, width, height, TFT_WHITE, TFT_DARKGREY, TFT_WHITE, "Up", 0);
      bt_idx++;
    }

  if (stop_idx < num_dat)
    {
      buttons.emplace_back(TFT_eSPI_Button());
      buttons_idx.emplace_back(UP);
      buttons.back().initButton(&tft, x1, y0+(num_per_page+1)*offset, width, height, TFT_WHITE, TFT_DARKGREY, TFT_WHITE, "Down", 0);
      bt_idx++;
    }
  for ( auto & b : buttons)
    b.drawButton();
  
}

bool LockerInterface::check_selection(int & sel)
{
  sel = 0;
  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

  // check if we have a click
  bool pressed = tft.getTouch(&t_x, &t_y);
  bool found = false;
  if (pressed)
    {
      sprintf(strbuf, "pressed %d %d", t_x, t_y);
      status(strbuf);
      // check if we are in a region
      for (int i = 0; i < buttons.size(); i++)
	{
      
	  if (buttons[i].contains(t_x,t_y))
	    {
	      
	      buttons[i].press(true);
	      buttons[i].drawButton();
	      sel = buttons_idx[i];
	      found = true;
	    }
	}
 
    }

  if (found )
    {
      if (sel == DOWN)
	{
	  show_selector(displayed_page - 1);
	}
      else if (sel == UP)
	{
	  show_selector(displayed_page + 1);
	}
      else
	return true;
    }
  return false;
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
