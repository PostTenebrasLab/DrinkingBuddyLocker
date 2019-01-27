#include <SPI.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager

#include <PubSubClient.h>

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

//MQTT stuff
//const char* mqtt_server = "mqtt.lan.posttenebraslab.ch";
const char* mqtt_server = "185.250.59.200";
WiFiClient espClient;
PubSubClient client(espClient);


char strbuf[200];

char* lastBadge = "";
/*
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please get a good version of Adafruit_SSD1306.h! from github mcauser branch 64x48");
#endif

*/
bool getLocker(char* badge);
void configModeCallback (WiFiManager *myWiFiManager);

#define RESTART_RFID 30000UL
unsigned long lastRestartTime = millis();

unsigned long last_open_request = millis();
unsigned long menu_timeout = 20000;
const long GRACE_PERIOD = 20000;  //20 secs



LockerInterface display;

enum iface_state_machine {
  IFACE_STATE_STARTUP,
  IFACE_STATE_SWIPE,
  IFACE_STATE_MENU
};

iface_state_machine iface_state = IFACE_STATE_STARTUP;


void setup() {
    iface_state = IFACE_STATE_STARTUP;
    Serial.begin(115200);

    Serial.println("Init Display");
    display.init();

  
    Serial.println("Starting display");

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

    client.setServer(mqtt_server, 1883);
    
    sound.play("b1");
    sound.play("a1");
    display.status("Ready");
/*
    oledPrint("Ready :)");
    getFoodCount();
*/
    

   // delay(2000);
    
   // display.set_selection(std::vector<std::string>{"Ta","Mere","En","String","A","La","Migros"});
    
   // display.show_selector(0);
   // display.status("Select Locker");

    iface_state = IFACE_STATE_SWIPE;
}

void loop() {
  // put your main code here, to run repeatedly:

  switch (iface_state)
  {
    case IFACE_STATE_SWIPE:
    {
      // swipe state
      char* badge = rfid.tryRead();
      if (badge)
      { 
        sound.play("b1");
        lastBadge = badge;
        Serial.print("badge found ");
        Serial.println(badge);
        Serial.print("Last badge changed to: ");
        Serial.println(lastBadge);
      
        // ignore all waiting badge to avoid unintended double buy
        while (rfid.tryRead())
        {
          Serial.println("rfid.tryRead");
          delay(1000);
        }
        if (getLocker(badge))
        {
          
          iface_state = IFACE_STATE_MENU;
          display.show_selector(0);
          last_open_request = millis();
          return;
        }
        // move to menu selector state
      } //end if badge

      // Display Swipe
      display.swipe_prompt();
    }
    case IFACE_STATE_MENU:
    {
      // checkfor timeout
      if (millis() - last_open_request > menu_timeout)
        iface_state = IFACE_STATE_SWIPE;
        
      int sel = 0;
      if (display.check_selection(sel))
      {
        if (sel == -3)
        {
          // cancel button
          iface_state = IFACE_STATE_SWIPE;
          return;
        }
        sprintf(strbuf, "selected: %d", sel);

        // PLACEHOLDER, send out locker open command.

        if (!client.connected()) {
            reconnect();
        }
        //client.loop();
        delay(500);

        client.publish("Locker", "0A");
  
        display.status(strbuf);

        iface_state = IFACE_STATE_SWIPE;
      }
    }
  }
}



bool getLocker(char* badge)
{
  HttpBuyTransaction lockerTransaction(http);

  Serial.println("Starting to get lockers...");
  if (!lockerTransaction.performLocker(badge, myclock.getUnixTime()))
  {
    Serial.println("Error getting lockers...");
    return false;
  }

  Serial.println("End get lockers...");
 // Serial.println(lockerTransaction.getMessage(0));
 // Serial.println(lockerTransaction.getMessage(1));
 std::vector<std::string> message = lockerTransaction.getMessage();
 
  if (message.size() > 0 && message[0] == "ERROR")
  {
    lastBadge = "";
    Serial.print("Unknown badge or no lockers");

    //sound.play(lockerTransaction.getMelody());
    return false;
  }
  else
  {
    Serial.println("OK locker");
    digitalWrite(PIN_RELAY, HIGH); // turn on relay with voltage HIGH
    
    sound.play(lockerTransaction.getMelody());
    //PLAY SOUND
    display.set_selection(message);
    
    return true;
  }
}

void configModeCallback (WiFiManager *myWiFiManager) {
  //oledPrint("Plz config WiFi");
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client","ptllocker1","P0stL0ck")) {
      Serial.println("connected");
      client.subscribe("Locker");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
