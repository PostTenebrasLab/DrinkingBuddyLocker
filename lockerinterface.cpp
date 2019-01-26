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


Button::Button(  TFT_eSPI & tft_, int x1_, int x2_, int y1_, int y2_, std::string label_, int value_):
  tft(tft_)
{
  x = (x1_ + x2_)/2;
  y = (y1_ + y2_)/2;
  
  // compute bounds
  x1 = x1_;
  x2 = x2_;

  y1 = y1_;
  y2 = y2_;
  label = label_;
  value = value_;
}

bool Button::in_bounds(int x_, int y_)
{
  return (x1 < x_) && (x2 > x_) && (y1 < y_) && (y2 > y_);
}

void Button::draw()
{
  tft.fillRect(x1,y1,x2-x1,y2-y1, TFT_DARKGREY);
  tft.drawRect(x1,y1,x2-x1,y2-y1, TFT_WHITE);

  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextDatum(CC_DATUM);
  tft.setFreeFont(&FreeSansBold12pt7b);
  tft.drawString(label.c_str(), x, y);
}

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
   
  buttons.clear();
  const int num_per_page = 3;
  
  int num_dat = selection.size();
  int start_idx = num_dat/num_per_page*page;
  start_idx = (start_idx > num_dat)?(num_dat/num_per_page-1)*num_per_page:start_idx;
  int stop_idx = start_idx + num_per_page;
  stop_idx = (stop_idx > num_dat)?num_dat:stop_idx;

  int bt_idx = 0;
  int x1 = WIDTH/3;
  int x2 = 2*WIDTH/3;
  int y0 = 50;
  int height = 30;
  int offset = 50;
  sprintf(strbuf, "start: %d stop: %d num: %d", start_idx, stop_idx, num_dat);
  status(strbuf);
  Serial.println(strbuf);
  for (int i = start_idx; i < stop_idx; i++)
    {
      int y1 = bt_idx * offset + y0;
      int y2 = y1 + height;
      sprintf(strbuf, "%d", selection[i]); 
      buttons.emplace_back(Button(tft, x1, x2, y1, y2, std::string(strbuf), selection[i]));
      bt_idx++;
    }

  for ( auto & b : buttons)
    b.draw();
  
}

bool LockerInterface::check_selection(int & sel)
{
  sel = 0;
  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

  // check if we have a click
  bool pressed = tft.getTouch(&t_x, &t_y);

  if (pressed)
    {
      // check if we are in a region
      for (auto & b : buttons)
	{
	  if (b.in_bounds(t_x,t_y))
	    {
	      sel = b.value;
	    return true;
	    }
	}

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
