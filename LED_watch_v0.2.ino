/// LED watch - version 2.0

#include "Wire.h"                    /// I2C library
#include "LowPower.h"                /// Low power library for deep sleep mode
#include "M41T62.h"                  /// RTC library for M41T62 (on-board watch) - NB: Use "dayOfWeek"
RTC_M41T62 RTC;                      /// on-board M41T62 RTC

#define NUM_COL       6                /// Columns are anodes
#define NUM_ROW       6                /// Rows are cathodes
#define COL_ON    HIGH
#define COL_OFF   LOW
#define ROW_ON    LOW
#define ROW_OFF   HIGH

const int button1 = 9;
const int button2 = 10;

const int colLED[6] = {2, 3, 4, 5, 6, 7};          /// pins for anode (+) connections on ATmega328, columns in LED matrix
const int rowLED[6] = {0, 1, A0, A1, A2, A3};      /// pins for cathode (-) connections on ATmega328, rows in LED matrix

const int hourRows = 6;
const int hourCols = 2;
const int minRows = 6;
const int minCols = 2;
const int dayRows = 1;
const int dayColumns = 7;

int t = 5;
int rowPin = 6;
int colPin = 6;
int hourRow;
int hourColumn;
int minuteRow;
int minuteColumn;

int hourArray[hourRows][hourCols] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}, {11, 12}};

/// - - - - - - - - - - - SETUP + LOOP - - - - - - - - - - - - - - - -

/// no access to serial monitor

void setup() {
  Wire.begin();                                 /// I2C communication with the RTC
  //pinMode(button1, INPUT_PULLUP);               /// button1 pulled LOW when pressed
  //pinMode(button2, INPUT_PULLUP);               /// button2 pulled LOW when pressed
  RTC.begin();                                  /// start RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));     /// set RTC time to computer time

  for (int i = 0; i < rowPin; i++)              /// set all column pins to OUTPUT and OFF
  {
    pinMode(colLED[i], OUTPUT);                 /// output
    digitalWrite(colLED[i], COL_OFF);           /// turn off
  }
  for (int j = 0; j < colPin; j++)              /// set all row pins to OUTPUT and OFF
  {
    pinMode(rowLED[j], OUTPUT);                 /// output
    digitalWrite(rowLED[j], ROW_OFF);           /// turn off
  }
}

void loop() {

  //attachInterrupt(digitalPinToInterrupt(button1), wakeUp, LOW);             /// allow button1 to trigger interrupt (wake up) when LOW
  //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);                      /// enter power down state with Analog to Digital Converter (ADC) and Brown-out Detection (BOD) modules disabled
  //detachInterrupt(digitalPinToInterrupt(button1));                          /// disable external pin interrupt on wake up pin

  displayTime();
  //computing();

  //int buttonState1 = digitalRead(button1);
  //if (buttonState1 == LOW) {
  //allOff();

  //} else {

  //  }
  /*
      int buttonState2 = digitalRead(button2);
      if (buttonState2 == LOW) {
      //allOff();
      } else {
      cylonTest();
      } */
}

void wakeUp()
{
  /// handler for the pin interrupt
}


/// - - - - - - - - - - - led routines - - - - - - - - - - - - - - - -

void allOn()                             /// turns off all LEDs
{
  /// all cathode pins to 0V
  for (int i = 0; i < rowPin; i++)
  {
    digitalWrite(rowLED[i], HIGH);
  }
  /// all anode pins to +5V
  for (int j = 0; j < colPin; j++)
  {
    digitalWrite(colLED[j], LOW);
  }
}

void allOff()                             /// turns off all LEDs
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

void ledOn(int row, int col)                /// turns on specfic LEDs
{
  allOff();
  digitalWrite(rowLED[row], HIGH);        /// cathode pin to +5V
  digitalWrite(colLED[col], LOW);         /// anode pin to 0V
}

void ledOff(int row, int col)                /// turns off specific LEDs
{
  digitalWrite(rowLED[row], LOW);         /// cathode pin to 0V
  digitalWrite(colLED[col], HIGH);         /// anode pin to +5V
}

/// - - - - - - - - - - - - - - - time routines - - - - - - - - - - - - - - - -

void displayTime()
{
  DateTime now = RTC.now();
  int hour = now.hour();
  int minute = (now.minute());
  int minuteRemainder = minute % 5;
  int weekday = now.dayOfWeek() - 1;

  /// set hour
  if (hour > 12) {
    hour -= 12;
  };

  /// display the time
  turnOnHourLeds(hour);
  delayMicroseconds(100);
  turnOnMinuteLeds(minute);
  delayMicroseconds(100);
  turnOnRemainderLeds(minuteRemainder);
  delayMicroseconds(100);
  turnOnDayLeds(weekday);
  delayMicroseconds(100);
}
//}

void turnOnHourLeds(int hour)
{
  for (int i = 0; i < hourRows; i++) {
    for (int j = 0; j < hourCols; j++) {
      if (hour == hourArray[i][j]) {
        (hourRow = i);
        (hourColumn = j);
      }
    }
  }
  ledOn(hourRow, hourColumn);
}

void turnOnMinuteLeds(int minute)
{
  /// Set minute
  if (minute % 10 == 5) {
    /// The minute ends in 5
    minuteColumn = 2;
    minuteRow = (minute - 5) / 10;
  } else if (minute % 10 == 0) {
    /// The minute ends in 0
    minuteColumn = 3;
    if (minute == 0) {
      minuteRow = 5;
    } else {
      minuteRow = (minute / 10) - 1;
    }
  }
  ledOn(minuteRow, minuteColumn);
}

void turnOnRemainderLeds(int minuteRemainder)
{
  /// Set minute remainder
  if (minuteRemainder > 0) {
    /// Take 1 from the minuteRemainder to set the zero indexed LED
    /// e.g. 1 minute, set the (0, 4) LED
    ledOn((minuteRemainder - 1), 4);
  }
}

void turnOnDayLeds(int weekday)
{
  /// Take 1 away from dayOfWeek to get 0 indexed row to light up
  if (weekday < 5)
  {
    /// Only light up Monday - Friday
    ledOn(weekday, 5);
  }
}

/// - - - - - - - - - - - - pattern routines - - - - - - - - - - - - - - -

void computing()                /// turns on specfic LEDs
{
  for (int i = random(0, 6); i < rowPin; i++)
  {
    for (int j = random(0, 6); j < colPin; j++) {
      digitalWrite(rowLED[i], HIGH);
      digitalWrite(colLED[j], LOW);
      delay(5);
      digitalWrite(rowLED[j], LOW);
      digitalWrite(colLED[i], HIGH);
    }
  }
  for (int j = random(0, 6); j < rowPin; j++)
  {
    digitalWrite(rowLED[j], HIGH);
    digitalWrite(colLED[j], HIGH);
    delay(5);
  }
  delay(1);
}
