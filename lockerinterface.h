
#include <TFT_eSPI.h>      // Hardware-specific library

#include <string>
#include <vector>

class Button
{
public:
  Button(  TFT_eSPI & tft, int x, int y, int width, int height, std::string label, int value);
  
  bool in_bounds(int x, int y);

  void draw();

  int value;  
private:
  int x1;
  int x2;
  int y1;
  int y2;
  int x;
  int y;
  std::string label;


  TFT_eSPI & tft;  
};

class LockerInterface
{
public:
  LockerInterface();

  void init();

  // Show splashscreen
  void splash();
  // Print stuff in status bar
  void status(const std::string & msg);

  
  void clear_status_bar();
  void swipe_prompt();
  void set_selection(std::vector<int> selection_);
  void show_selector(int page);
  bool check_selection(int & selection);
private:
  void touch_calibrate();
  
  TFT_eSPI tft;
  char strbuf[200];
  
  std::vector<int> selection;
  std::vector<Button> buttons;
};
