#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_SSD1306.h>

//разрешение дисплея
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

//время
#define TIME_OFFSET 3*60*60 // смещение часового пояса в сек.
#define TIME_UPD_INTERVAL 60*60*1000 // период запроса к ntp в ms

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", TIME_OFFSET, TIME_UPD_INTERVAL);

class Ref;

class Display {
  public:
    Display(Ref* pref1 = 0, Ref* pref2 = 0) : pRef1(pref1), pRef2(pref2), display(SCREEN_WIDTH, SCREEN_HEIGHT) {}
    
    void begin(int dd, void (*detailChangeCallback)(Display*)) {
      detail = dd;
      onDetailChange = detailChangeCallback;
      display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
      display.clearDisplay();
      display.setTextColor(WHITE);
      display.setTextSize(1);
    }
    
    void page0() {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println(VER);
      display.println("wi-fi connecting...");
      display.display();
    }
    
    void page1(String info) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println(VER);
      display.println("wi-fi connected");
      display.println(info);
      display.println("update check...");
      display.display();
    }
    
    void page3(String info) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println(VER);
      display.println("wi-fi NOT connected");
      display.println(info);
      display.display();
    }
    
    void update(bool force = false) {
      if (lvlPageOn)
        return;
        
      if (millis() - timeLastUpdate >= timeUpdateInterval || timeLastUpdate == 0 || force) {
        timeLastUpdate = millis();

        display.clearDisplay();
        display.setTextColor(WHITE);
        
        if (detail >= 1) {
          if (WiFi.status() == WL_CONNECTED)
            timeClient.update();
          
          //время ЧЧ:ММ
          String timeHHMM;
          if (WiFi.status() == WL_CONNECTED) {
            timeHHMM = timeClient.getFormattedTime().substring(0, 5);
          } else {
            timeHHMM = "--:--";
          }
  
          int clockPos;
          if (detail == 1)
            clockPos = 23;
          else if (detail == 2)
            clockPos = 28;
          else if (detail == 3)
            clockPos = 17;
            
          display.setTextSize(3);
          display.setCursor(clockPos, 4);
          display.print(timeHHMM);
          
          //секунда
          int sec = timeClient.getSeconds();
          display.fillRect(clockPos, 29, 59*1.3, 3, BLACK);
          display.fillRect(clockPos + sec*1.3, 29, 3, 3, WHITE);
        }
        
        if (detail >= 2) {
          //статус холодильной камеры
          bool refOn = pRef1->compressorStarted();
          if (refOn) {
            display.fillRect(0, 0, 13, 17, WHITE);
            display.setTextColor(BLACK, WHITE);
          }
          display.setTextSize(2);
          display.setCursor(1, 1);
          display.print(pRef1->lvlChar());
          if (refOn)
            display.setTextColor(WHITE);
          
          //статус морозильной камеры
          refOn = pRef2->compressorStarted();
          if (refOn) {
            display.fillRect(0, 16, 13, 17, WHITE);
            display.setTextColor(BLACK, WHITE);
          }
          display.setTextSize(2);
          display.setCursor(1, 18);
          display.print(pRef2->lvlChar());
          if (refOn)
            display.setTextColor(WHITE);
        }
        
        if (detail >= 3) {
          //температура холодильной камеры
          display.setTextSize(1);
          if (pRef1->getTemperatureFreezer() != -127) {
            display.setCursor(104, 0);
            display.print(formatTemp(pRef1->getTemperatureFreezer()));
          }
          if (pRef1->getTemperatureAir() != -127) {
            display.setCursor(104, 8);
            display.print(formatTemp(pRef1->getTemperatureAir()));
          }
          
          //температура морозильной камеры
          display.setTextSize(1);
          if (pRef2->getTemperatureFreezer() != -127) {
            display.setCursor(104, 16);
            display.print(formatTemp(pRef2->getTemperatureFreezer()));
          }
          if (pRef2->getTemperatureAir() != -127) {
            display.setCursor(104, 24);
            display.print(formatTemp(pRef2->getTemperatureAir()));
          }
        }
        
        display.display();
      }
    }
    
    void lvlPageBegin(int refNum, int lvl, char lvlChar) {
      lvlPageOn = true;
      lvlPageLastMillis = millis();
      
      int pos = 35;
      if (detail >= 3)
        pos = pos - 10;

      display.clearDisplay();
      if (refNum == 1)
        display.fillRoundRect(pos, 0, 10, 17, 2, WHITE);
      else
        display.fillRoundRect(pos, 15, 10, 17, 2, WHITE);
        
      display.setTextSize(4);
      display.setCursor(pos+24, 2);
      display.print(lvlChar);

      if (detail >= 3 && lvl > 0 && lvl < 10) {
        Ref* pRef;
        if (refNum == 1)
          //температура холодильной камеры
          pRef = pRef1;
        else
          //температура морозильной камеры
          pRef = pRef2;

        display.setTextSize(2);
        display.setCursor(90, 0);
        display.print(formatTemp2(pRef->getMaxTemperature()));
        display.setCursor(90, 16);
        display.print(formatTemp2(pRef->getMinTemperature()));
      }
      display.display();
    }

    void loop() {
      if (lvlPageOn)
        if (millis() - lvlPageLastMillis >= lvlPageLastInterval || lvlPageLastMillis == 0)
          lvlPageOn = false;
      update();
    }

    void detailUp() {
      detail++;
      if (detail == 4)
        detail = 0;
      onDetailChange(this);
      update(true);
    }
    
    int getDetail() {
      return detail;
    }
    
  private:
    String formatTemp(float t) {
      String str;
      if (t >= 10 || t <= -10)
        str = String(t, 0);
      else
        str = String(t, 1);
      while (str.length() < 4)
        str = " " + str;
      return str;
    }
    String formatTemp2(float t) {
      String str;
      str = String(t, 0);
      while (str.length() < 3)
        str = " " + str;
      return str;
    }
  
  private:
    Adafruit_SSD1306 display;
    Ref* pRef1;
    Ref* pRef2;
    int detail = 2;
    unsigned long timeLastUpdate = 0;
    unsigned long timeUpdateInterval = 1000;
    bool lvlPageOn = false;
    unsigned long lvlPageLastMillis = 0;
    unsigned long lvlPageLastInterval = 3000;
    void (*onDetailChange)(Display*);
};
