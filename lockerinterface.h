
#include <TFT_eSPI.h>      // Hardware-specific library

#include <string>
#include <vector>

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
  void set_selection(std::vector<std::string> selection_);
  void show_selector(int page);
  bool check_selection(int & selection);
private:
  void touch_calibrate();
  
  TFT_eSPI tft;
  char strbuf[200];
  
  std::vector<std::string> selection;
  std::vector<TFT_eSPI_Button> buttons;
  std::vector<int> buttons_idx;

  int displayed_page = 0;

  int swipe_x1;
  int swipe_x2;
  int swipe_y1;
  int swipe_y2;
  
  int swipe_x1_v;
  int swipe_x2_v;
  int swipe_y1_v;
  int swipe_y2_v;
  
};
