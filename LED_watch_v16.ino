/// LED watch - version 15
///     -- Display time: button1 polls the RTC and turns on relevant LEDs on multiplexed display

/// 01/11/21 -- added code for low power mode and interrupt - not tested


#include "Wire.h"                    /// I2C library
#include "LowPower.h"                /// Low power library for deep sleep mode
#include "M41T62.h"                  /// RTC library for M41T62 (on-board watch) - NB: Use "dayOfWeek"
RTC_M41T62 RTC;                     /// on-board M41T62 RTC

const int button1 = 9;
const int button2 = 10;

const int rowLED[6] = {0, 1, A0, A1, A2, A3};      /// pins for cathode (-) connections on ATmega328, rows in LED matrix
const int colLED[6] = {2, 3, 4, 5, 6, 7};          /// pins for anode (+) connections on ATmega328, columns in LED matrix

const int hourRows = 6;
const int hourCols = 2;
const int minRows = 6;
const int minCols = 2;
const int dayNum = 7;
const int dayCols = 1;

int t = 5;
int rowPin = 6;
int colPin = 6;
int rowPin2 = 6;
int colPin2 = 5;
int rowPosHR;
int colPosHR;
int rowPosMin;
int colPosMin;
int numPosDAY;
int cathState = LOW;
int anodState = HIGH;

int hourArray[hourRows][hourCols] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}, {11, 12}};
int minuteArray[minRows][minCols] = {{5, 10}, {15, 20}, {25, 30}, {35, 40}, {45, 50}, {55, 60}};
int weekdayArray[dayNum] = {0, 1, 2, 3, 4, 5, 6};

// - - - - - - - - - - - SETUP + LOOP - - - - - - - - - - - - - - - -

void setup() {

  ///REMINDER: no access to serial monitor

  Wire.begin();                                 /// I2C communication with the RTC
  pinMode(button1, INPUT_PULLUP);               /// button1 pulled LOW when pressed
  pinMode(button2, INPUT_PULLUP);               /// button2 pulled LOW when pressed
  RTC.begin();                                  /// start RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));     /// set RTC time to computer time

  for (int i = 0; i < rowPin; i++)              /// set all LED pins to OUTPUT
  {
    pinMode(rowLED[i], OUTPUT);
  }
  for (int j = 0; j < colPin; j ++)
  {
    pinMode(colLED[j], OUTPUT);
  }
}

void loop()
{
  attachInterrupt(digitalPinToInterrupt(button1), wakeUp, LOW);   /// allow button1 to trigger interrupt (wake up) when LOW
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);        /// enter power down state with Analog to Digital Converter (ADC) and Brown-out Detection (BOD) modules disabled
  detachInterrupt(0);                                      /// disable external pin interrupt on wake up pin

  int buttonState1 = digitalRead(button1);
  if (buttonState1 == HIGH) {
    //alloff();
  } else {
    displayTime();
  }

  /*
    int buttonState2 = digitalRead(button2);
    if (buttonState2 == HIGH) {
    //alloff();
    } else {
    cylonTest();
    }*/
}


void wakeUp()
{
  // handler for the pin interrupt
}

/// - - - - - - - - - - - - test function - - - - - - - - - - - - - - -

void runner_watch()                                   /// tests all LEDS by running through each one
{
  for (int i = 0; i < rowPin2; i++)
  { for (int j = 0; j < colPin2; j++)
    {
      alloff();
      digitalWrite(rowLED[i], HIGH);                  /// turns off each LED
      digitalWrite(colLED[j], LOW);
      delay(2);
      digitalWrite(rowLED[i], LOW);                  /// turns on each LED
      digitalWrite(colLED[j], HIGH);
      delay(2);
    }
  }
}

void cylonTest() /// row (x) then column (y) -- runs across days of the week
{
  for (int i = 0; i < 5; i++)
  {
    //alloff();
    LEDon(i, 5);
    delay(t);
  }
  for (int j = 4; j >= 0; j--)
  {
    LEDon(j, 5);
    delay(t);
  }
  alloff();
}

/// - - - - - - - - - - - basic display functions - - - - - - - - - - - - - - - -

void alloff()                             /// turns off all LEDs
{
  /// all cathode pins to 0V
  for (int i = 0; i < rowPin; i++)
  {
    digitalWrite(rowLED[i], LOW);
  }
  /// all anode pins to +5V
  for (int j = 0; j < colPin; j++)
  {
    digitalWrite(colLED[j], HIGH);
  }
}

void LEDon(int row, int col)                /// turns on specfic LEDs
{
  alloff();                               /// turn off other LEDs
  digitalWrite(rowLED[row], HIGH);        /// cathode pin to +5V
  digitalWrite(colLED[col], LOW);         /// anode pin to 0V
}


void LEDoff(int row, int col)                /// turns off specific LEDs
{
  alloff();                               /// turn off other LEDs
  digitalWrite(rowLED[row], LOW);         /// cathode pin to 0V
  digitalWrite(colLED[col], HIGH);         /// anode pin to +5V
}


/// - - - - - - - - - - - - - - - time functions - - - - - - - - - - - - - - - -

void displayTime()
{
  findPosHR();                  /// finds position of hour led
  findPosMin();                 /// finds position of minute led
  LEDon(rowPosHR, colPosHR);     /// turns on hour led
  delayMicroseconds(500);
  //delay(1);
  LEDon(rowPosMin, colPosMin);  /// turns on minute led

  LEDoff(0, 0);
  //LEDon(numPosDAY, 5);        /// turns on day led
}

void findPosHR()
{
  DateTime now = (RTC.now());
  int hourVal = (now.hour());
  if (hourVal > 12) {
    hourVal = (hourVal - 12);
  }
  else
  {
  };
  for (int i = 0; i < hourRows; i++) {
    for (int j = 0; j < hourCols; j++) {
      if (hourVal == hourArray[i][j]) {
        (rowPosHR = i);
        (colPosHR = j);
      }
    }
  }
  alloff();
}

void findPosMin()
{
  DateTime now = (RTC.now());
  int minuteVal = (now.minute());
  for (int i = 0; i < minRows; i++) {
    for (int j = 0; j < minCols; j++) {
      if ((1 <= minuteVal) && (minuteVal <=  5)) {
        LEDon(0, 2);
      }
      else if ((5 <= minuteVal) && (minuteVal <= 10)) {
        LEDon(0, 3);
      }
      else if ((11 <= minuteVal) && (minuteVal <= 15)) {
        LEDon(1, 2);
      }
      else if ((16 <= minuteVal) && (minuteVal <=  20)) {
        LEDon(1, 3);
      }
      else if ((21 <= minuteVal) && (minuteVal <= 25)) {
        LEDon(2, 2);
      }
      else if ((26 <= minuteVal) && (minuteVal <= 30)) {
        LEDon(2, 3);
      }
      else if ((31 <= minuteVal) && (minuteVal <= 35)) {
        LEDon(3, 2);
      }
      else if ((36 <= minuteVal) && (minuteVal <= 40)) {
        LEDon(3, 3);
      }
      else if ((41 <= minuteVal) && (minuteVal <=  45)) {
        LEDon(4, 2);
      }
      else if ((46 <= minuteVal) && (minuteVal <=  50)) {
        LEDon(4, 3);
      }
      else if ((51 <= minuteVal) && (minuteVal <= 55)) {
        LEDon(5, 2);
      }
      else if ((56 <= minuteVal) && (minuteVal <= 60)) {
        LEDon(5, 3);
      }
      //(rowPosMin = i);
      //(colPosMin = j);
    }
  }
  alloff();
}

void findPosDAY()
{
  DateTime now = (RTC.now());
  int weekdayVal = (now.dayOfWeek()); /// dayOfWeek for watch RTC
  for (int i = 0; i < dayNum; i++) {
    if ((1 < weekdayVal) && (weekdayVal <  6)) {
      if (weekdayVal == weekdayArray[i]) {
        {
          (numPosDAY = i);
        }
      }
    }
  }
}
