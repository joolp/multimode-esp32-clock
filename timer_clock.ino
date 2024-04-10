#include <Wire.h>
#include <TM1650.h>
#include <ESP32Time.h>

#define ANALOGIN 15
#define DIGITALOUT 5

TM1650 d;
ESP32Time T(25200);

byte nled = 5;
byte pled[] = {2, 4, 16, 17, 18};

enum MODES {OFF_MODE, CLOCK_MODE, PAUSE_MODE, STOPWATCH_MODE};
byte currentMode = OFF_MODE;

unsigned long t = 0;
byte tt = 0;

int timenow;
int stopwatch_time_csec = 0;
byte stopwatch_time_min = 0;
int analogin = 0;


hw_timer_t * timer = NULL;

void IRAM_ATTR timer_isr(){
  if (currentMode == OFF_MODE)
  {

  }
  else
  {
    if (currentMode == STOPWATCH_MODE)
    {
      stopwatch_time_csec++;
      if (stopwatch_time_csec >= 6000)
      {
        stopwatch_time_min++;
        stopwatch_time_csec = 0;
      }
    }

  }

}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  d.init();
  
  for (int i = 0; i < nled; i++)
  {
    pinMode(pled[i], OUTPUT);
  }

  pinMode(DIGITALOUT, OUTPUT);
  pinMode(ANALOGIN, INPUT);
  digitalWrite(DIGITALOUT, HIGH);

  uint8_t timer_id = 0;
  uint16_t prescaler = 80;
  int threashold = 10000;
  timer = timerBegin(timer_id, prescaler, true);
  timerAttachInterrupt(timer, &timer_isr, true);
  timerAlarmWrite(timer, threashold, true);
  timerAlarmEnable(timer);
}

void loop() {
  analogin = analogRead(ANALOGIN);

  if (analogin < 100)
  {
    // Serial.println("Current mode: Off");
    currentMode = OFF_MODE;
    stopwatch_time_csec = 0;
    stopwatch_time_min = 0;

    tt = 0;

    d.displayOff();
  }
  else
  {
    d.displayOn();
    d.setBrightness(TM1650_MAX_BRIGHT);
    if (analogin < 2000)
    {
      
      if (Serial.available() > 0)
      {
        timenow = Serial.parseInt();
        
        if (timenow)
        {
          T.setTime(timenow);
        }

        Serial.println(timenow);
      }

      t = T.getHour(true) * 100 + T.getMinute();
      tt = T.getDay();
      // Serial.println("Current mode: Clock");
      currentMode = CLOCK_MODE;
    }
    else 
    {
      t = stopwatch_time_csec;
      tt = stopwatch_time_min;
      if (analogin < 3000)
      {
        // Serial.println("Current mode: Pause");
        currentMode = PAUSE_MODE;
      }
      else
      {
        // Serial.println("Current mode: Stopwatch");
        currentMode = STOPWATCH_MODE;
      }
    }

  }
  
  char t_1 = ((t % 10000) / 1000) + '0';
  char t_2 = ((t % 1000) / 100) + '0';
  char t_3 = ((t % 100) / 10) + '0';
  char t_4 = ((t % 10) / 1) + '0';
  char line[] = {t_1,t_2,t_3,t_4};

  for (int i = 0; i < nled; i++)
  {
    digitalWrite(pled[i], bitRead(tt, i));
  }
  
  d.displayString(line);
}