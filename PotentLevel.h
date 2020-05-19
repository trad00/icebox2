//#include <Wire.h>
#include <Adafruit_ADS1015.h>

int MulDiv(int nNumber, int nNumerator, int nDenominator) {
  return nNumber * nNumerator *10000 / nDenominator / 10000;
}

class PotentLevel {
  public:
    PotentLevel(int8_t lvls = -1, int16_t minPot = -1, int16_t maxPot = -1) {
      if (lvls > -1)
        levels = lvls;
      if (minPot > -1)
        minPotent = minPot;
      if (maxPot > -1)
        maxPotent = maxPot;
    }
    void begin(uint8_t chnl, void (*lvlChangeCallback)(PotentLevel*)) {
      channel = chnl;
      onLvlChange = lvlChangeCallback;
      ads.begin();
      lastLevel = getCurrentLevel();
    }
    int16_t getCurrentLevel() {
      int16_t adc = ads.readADC_SingleEnded(channel) + 10;
      int16_t level = MulDiv(adc, levels, (maxPotent - minPotent));
      return level;
    }
    void loop() {
      if (millis() - lastUpdate >= updateInterval || lastUpdate == 0) {
        lastUpdate = millis();
        if (levels > 0) {
          int16_t level = getCurrentLevel();
          if (lastLevel != level) {
            lastLevel = level;
            onLvlChange(this);
          }
        }
      }
    }
    uint8_t getChanel() {
      return channel;
    }
    int16_t getLevel() {
      return lastLevel;
    }
    
  private:
    Adafruit_ADS1115 ads;
    unsigned long lastUpdate = 0;
    unsigned long updateInterval = 100;
    uint8_t channel = 0;
    int16_t lastLevel = -1;
    int16_t levels = 10;
    int16_t minPotent = 100;
    int16_t maxPotent = 16000;
    void (*onLvlChange)(PotentLevel*);
};
