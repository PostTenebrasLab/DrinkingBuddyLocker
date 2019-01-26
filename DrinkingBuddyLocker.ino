#include <SPI.h>
#include <Wire.h> //i2c oled
//#include <Adafruit_GFX.h> //oled graphics
//#include <Adafruit_SSD1306.h> //oled
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager


#include "Clock.h"
#include "HttpBuyTransaction.h"
#include "HttpClient.h"
#include "HttpSyncTransaction.h"
#include "RfidReader.h"
#include "Sound.h"

#include "lockerinterface.h"

static RfidReader rfid;
static Clock myclock;
static Sound sound;

static HttpClient http;

char* lastBadge = "";
/*
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please get a good version of Adafruit_SSD1306.h! from github mcauser branch 64x48");
#endif

*/
#define RESTART_RFID 30000UL
unsigned long lastRestartTime = millis();

/*
void oledPrint(const char* myText, bool clearOled = true)
{
  if(clearOled)
  {
    display.clearDisplay();
    display.setCursor(0,0);
  }
  display.println(myText);
  display.display();
  
}
*/

LockerInterface display;

void setup() {
    Serial.begin(115200);

    display.init();

  
    Serial.println("Starting display");
    /*
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    oledPrint("Start wifi", true);
    */
    Serial.println("Starting wifi...");
    display.status("Starting wifi...");
    sound.begin();
    sound.play("a1");    
        //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset settings - for testing
    //wifiManager.resetSettings();
    wifiManager.setAPCallback(configModeCallback);
    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    if(!wifiManager.autoConnect()) {
      /*
      display.clearDisplay();
      display.println("Failed to connect");
      display.display();
      */
      Serial.println("failed to connect and hit timeout");
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(1000);
    } 
    //oledPrint("Connected! :)", false);
    Serial.println("Wifi connected...)");
    display.status("Wifi connected...");
    Serial.println("Starting RFID...");
    display.status("Starting RFID...");
    SPI.begin();           // Init SPI bus
    //mfrc522.PCD_Init();    // Init MFRC522
    rfid.begin();
    sound.play("b1");
    sound.play("a1");
    display.status("Ready");
/*
    oledPrint("Ready :)");
    getFoodCount();
*/
    

}

void loop() {
  // put your main code here, to run repeatedly:

/*
  if((lastRestartTime + RESTART_RFID) < millis())   //we're having troubles with the reader,,maybe restarting would help.
  {
    lastRestartTime = millis();
//    rfid.restart();
//    Serial.print("Restarting the RFID reader at: "); Serial.println(lastRestartTime);
  }
*/
  /*
  char* badge = rfid.tryRead();
  if (badge)
  {
    sound.play("b1");
    lastBadge = badge;
    Serial.print("badge found ");
    Serial.println(badge);
    Serial.print("Last badge changed to: ");
    Serial.println(lastBadge);
    oledPrint("Badge: ");
    oledPrint(lastBadge,false);
    buyFood(badge);
    // ignore all waiting badge to avoid unintended double buy
    while (rfid.tryRead())
    {
      Serial.println("rfid.tryRead");
      delay(1000);
    }
    //oledPrint("Ready :)");
    
  } //end if badge
    */
  

}


/*
bool buyFood(char* badge)
{
  HttpBuyTransaction buyFoodTransaction(http);

  int BEER_ID = 42;
  int FOOD_ID = 5;

  Serial.println("Starting to get food...");
  if (!buyFoodTransaction.perform(badge, FOOD_ID, clock.getUnixTime()))
  {
    Serial.println("Error getting food...");
    oledPrint("Error: 1");
    return false;
  }

  Serial.println("End get food...");
  
  if (strcmp(buyFoodTransaction.getMessage(0), "ERROR") == 0)
  {
    lastBadge = "";
    Serial.print("Unknown badge or not enough credit");
    oledPrint("No credit ?");

    //sound.play(buyFoodTransaction.getMelody());
    return false;
  }
  else
  {
    Serial.println("OK food");
    oledPrint("Bon app!:)");
    sound.play(buyFoodTransaction.getMelody());
    delay(2000);
    getFoodCount();
    //PLAY SOUND
    
    return true;
  }
}
*/

/*
bool getFoodCount()
{
  HttpBuyTransaction buyFoodTransaction(http);

  Serial.println("Getting food count");
  if (!buyFoodTransaction.getFood())
  {
    Serial.println("Error getting food count...");
    oledPrint("Error: food count");
    return false;
  }

  Serial.println("End get food count...");
  
  Serial.print("People bought today: ");Serial.println(buyFoodTransaction.getMessage(1));
  oledPrint("Food paid today: ");
  oledPrint(buyFoodTransaction.getMessage(1),false);
  
  
  return true;
}
*/
void configModeCallback (WiFiManager *myWiFiManager) {
  //oledPrint("Plz config WiFi");
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
