
#include <TFT_eSPI.h>      // Hardware-specific library

#include <string>

class LockerInterface
{
public:
  LockerInterface();

  void init();

  // Show splashscreen
  void splash();
  // Print stuff in status bar
  void status(const std::string & msg);
  
private:
  void touch_calibrate();
  
  TFT_eSPI tft;
};
