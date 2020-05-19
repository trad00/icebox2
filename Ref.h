int tRefArr[9][2] = {
  +2, +4, //lvl 1
  +0, +2, //lvl 2
  -2, +1, //lvl 3
  -4, +1, //lvl 4
  -6, +1, //lvl 5
  -8, +1, //lvl 6
 -10, -2, //lvl 7
 -12, -2, //lvl 8
 -14, -2  //lvl 9
};

int tFrzArr[9][2] = {
  -5,  -4, //lvl 1
  -7,  -6, //lvl 2
  -9,  -7, //lvl 3
 -11,  -9, //lvl 4
 -13, -10, //lvl 5
 -15, -12, //lvl 6
 -17, -14, //lvl 7
 -20, -16, //lvl 8
 -23, -19  //lvl 9
};

int tTestArr[9][2] = {
  33, 34, //lvl 1
  32, 33, //lvl 2
  31, 32, //lvl 3
  30, 31, //lvl 4
  29, 30, //lvl 5
  28, 29, //lvl 6
  27, 28, //lvl 7
  26, 27, //lvl 8
  25, 26  //lvl 9
};

//запуск компрессора
#define STARTTIME 500 //время работы пусковой обмотки компрессора
#define STARTCHECKTIME 5*60000 //5 мин. - время контроля запуска компрессора, за это время должна начать понижаться температура
#define STARTCHECKTEMP 0.1 //необходимое изменение температуры за время контроля запуска компрессора
#define STARTRESTTIME  10*60000 //10 мин. - время отдыха/остывания компрессора, если компрессор не запустился его нужно принудительно выключить на время


//управление реле
#define ON LOW
#define OFF HIGH

class Compressor {
  public:
    Compressor(int p1, int p2) : pinMain(p1), pinStart(p2) {};
    void begin() {
      pinMode(pinMain, OUTPUT);
      pinMode(pinStart, OUTPUT);
      stop();
    };
    bool started() {
      return digitalRead(pinMain) == ON;
    };
    void start() {
      if (!started()) {
        digitalWrite(pinMain, ON);
        digitalWrite(pinStart, ON);
        prevMs = millis();
      }
    };
    void stop() {
      digitalWrite(pinMain, OFF);
      digitalWrite(pinStart, OFF);
    };
    void loop() {
      if (millis() - prevMs >= STARTTIME) {
        if (digitalRead(pinStart) == ON) {
          digitalWrite(pinStart, OFF);
        }
      }
    };
  private:
    int pinMain, pinStart;
    unsigned long prevMs;
};

class Ref {
  public:
    Ref(int p1, int p2) : compressor(p1, p2) {
      lampConnected = false;
      tFreezer = 0;
      tAir = 0;
      tMin = 0;
      tMax = 0;
      lvl = 0;
      off = false;
      frz = false;
      prevMs = 0;
      startStage = 0;
      startT = 0;
      startMs = 0;
      tArr = tRefArr;
    };
    
    void lvlUp() {
      off = false;
      if (lvl < 9) {
        lvl++;
        tMinMaxUpd();
        frz = false;
      }
      else {
        lvl = 10;
        frz = true;
      }
      onLvlChange(this);
      controlLoop(tFreezer, tAir);
      lampControl();
    };
    
    void lvlDn() {
      frz = false;
      if (lvl > 1) {
        lvl--;
        tMinMaxUpd();
        off = false;
      }
      else {
        lvl = 0;
        off = true;
      }
      onLvlChange(this);
      controlLoop(tFreezer, tAir);
      lampControl();
    };
    
    void lvlSet(int l) {
      lvl = l;
      frz = false;
      off = false;
      if (lvl > 10) {
        lvl = 10;
      } else if (lvl < 0) {
        lvl = 0;
      }
      if (lvl == 0) {
        off = true;
      } else if (lvl == 10) {
        frz = true;
      }
      tMinMaxUpd();
      onLvlChange(this);
      controlLoop(tFreezer, tAir);
      lampControl();
    };
    
    char lvlChar() {
      if (off)
        return '-';
      else if (frz)
        return '*';
      else
        return char(48+lvl);
    };

    int getLvl() {
      return lvl;
    };

    bool isOff() {
      return off;
    }

    bool isFreezing() {
      return frz;
    }

    bool compressorStarted() {
      return compressor.started();
    };
    
    float getTemperatureFreezer() {
      return tFreezer;
    }
    
    float getTemperatureAir() {
      return tAir;
    }
    
    float getMinTemperature() {
      return tMin;
    }
    
    float getMaxTemperature() {
      return tMax;
    }

    void begin(int l, void (*lvlChangeCallback)(Ref*)) {
      lvl = l;
      onLvlChange = lvlChangeCallback;
      if (lvl >= 1 && lvl <= 9) {
        tMinMaxUpd();
        off = false;
        frz = false;
      } else if (lvl >= 10) {
        lvl = 10;
        tMinMaxUpd();
        off = false;
        frz = true;
      } else {
        lvl = 0;
        off = true;
        frz = false;
      }
      lampControl();
      prevMs = millis();
      compressor.begin();
    };
    
    void connectLamp(int pin) {
      pinMode(pin, OUTPUT);
      lampConnected = true;
      lampPin = pin;
    };
    void lampControl() {
      if (lampConnected){
        digitalWrite(lampPin, off ? ON: OFF);
      }
    };
    
    void loop() {
      compressor.loop();
    }
    
    void controlLoop(float cur_tFreezer, float cur_tAir) {
      tFreezer = cur_tFreezer;
      tAir = cur_tAir;
      if (off) {
        if (compressor.started())
          stopCompressor();
      } else if (frz) {
        if (!compressor.started())
          startCompressor();
      } else {
        if (tFreezer >= tMax) {
          startCompressor();
        } else if (tFreezer <= tMin) {
          stopCompressor();
        }
      }
      startCheck();
    };

  private:
    virtual void tMinMaxUpd() {
      if (lvl > 0 && lvl < 10) {
        tMin = tArr[lvl-1][0];
        tMax = tArr[lvl-1][1];
      }
    }

    void startCompressor() {
      if (!compressor.started() && startStage != 2) {
        compressor.start();
        startStage = 1;
        startT = tFreezer;
        startMs = millis();
      }
    }
    
    void stopCompressor() {
      if (compressor.started()) {
        compressor.stop();
        startStage = 0;
      }
    }
    
    void startCheck() {
      if (startStage == 1) {
        if (millis() - startMs >= STARTCHECKTIME || tFreezer + STARTCHECKTEMP <= startT) {
          if (tFreezer + STARTCHECKTEMP <= startT) {
            startStage = 0;
          } else {
            stopCompressor();
            startStage = 2;
            startMs = millis();
          }
        }
      } else if (startStage == 2) {
        if (millis() - startMs >= STARTRESTTIME) {
          startStage = 0;
          startCompressor();
        }
      }
    }
  
  private:
    Compressor compressor;
    bool lampConnected;
    int lampPin;
    float tFreezer, tMin, tMax; //температура на испарителе: текущая, мин, макс
    float tAir; //температура воздуха в камере
    int lvl; //уровень регулировки
    bool off; //выключен
    bool frz; //заморозка без контроля температуры
    unsigned long prevMs;
    int startStage;
    float startT;
    unsigned long startMs;
    void (*onLvlChange)(Ref*);
    friend class Frz;
    int (*tArr)[2];
};

class Frz : public Ref {
  public:
    Frz(int p1, int p2) : Ref(p1, p2) {
      tArr = tFrzArr;
    }
};
