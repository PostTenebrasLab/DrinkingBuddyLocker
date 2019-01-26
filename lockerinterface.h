
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
  void set_selection(std::vector<int> selection_);
  void show_selector(int page);
  bool check_selection(int & selection);
private:
  void touch_calibrate();
  
  TFT_eSPI tft;
  char strbuf[200];
  
  std::vector<int> selection;
  std::vector<TFT_eSPI_Button> buttons;
  std::vector<int> buttons_idx;

  int displayed_page = 0;
};
