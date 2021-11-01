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
const int dayRows = 1;
const int dayColumns = 7;

int t = 5;
int rowPin = 6;
int colPin = 6;
int rowPin2 = 6;
int colPin2 = 5;
int hourRow;
int hourColumn;
int minuteRow;
int minuteColumn;
int dayColumn;
int cathState = LOW;
int anodState = HIGH;

int hourArray[hourRows][hourCols] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}, {11, 12}};
int minuteArray[minRows][minCols] = {{5, 10}, {15, 20}, {25, 30}, {35, 40}, {45, 50}, {55, 60}};
int weekdayArray[dayColumns] = {0, 1, 2, 3, 4, 5, 6};

// - - - - - - - - - - - SETUP + LOOP - - - - - - - - - - - - - - - -

void setup()
{
  // REMINDER: no access to serial monitor

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
    allOff();
  } else {
    displayTime();
  }

  /*
    int buttonState2 = digitalRead(button2);
    if (buttonState2 == HIGH) {
    //allOff();
    } else {
    cylonTest();
    }*/
}

void wakeUp()
{
  // handler for the pin interrupt
}

/// - - - - - - - - - - - - test function - - - - - - - - - - - - - - -

void runnerWatch()                                   /// tests all LEDS by running through each one
{
  for (int i = 0; i < rowPin2; i++)
  { for (int j = 0; j < colPin2; j++)
    {
      allOff();
      digitalWrite(rowLED[i], HIGH);                  /// turns on each LED
      digitalWrite(colLED[j], LOW);
      delay(2);
      digitalWrite(rowLED[i], LOW);                  /// turns off each LED
      digitalWrite(colLED[j], HIGH);
      delay(2);
    }
  }
}

void cylonTest() /// row (x) then column (y) -- runs across days of the week
{
  for (int i = 0; i < 5; i++)
  {
    ledOn(i, 5);
    delay(t);
  }
  for (int j = 4; j >= 0; j--)
  {
    ledOn(j, 5);
    delay(t);
  }
  allOff();
}

/// - - - - - - - - - - - basic display functions - - - - - - - - - - - - - - - -

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
  digitalWrite(rowLED[row], HIGH);        /// cathode pin to +5V
  digitalWrite(colLED[col], LOW);         /// anode pin to 0V
}


void ledOff(int row, int col)                /// turns off specific LEDs
{
  digitalWrite(rowLED[row], LOW);         /// cathode pin to 0V
  digitalWrite(colLED[col], HIGH);         /// anode pin to +5V
}


/// - - - - - - - - - - - - - - - time functions - - - - - - - - - - - - - - - -

void displayTime()
{
  turnOnHourLeds();
  delay(1);
  turnOnMinuteLeds();
  delay(1);
  turnOnDayLeds();
  delay(2);
  allOff();
}

void turnOnHourLeds()
{
  DateTime now = RTC.now();
  int hour = now.hour();
  if (hour > 12) {
    hour -= 12;
  };
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

void turnOnMinuteLeds()
{
  DateTime now = (RTC.now());
  int minute = (now.minute());

  // Set minute
  if (minute % 10 == 5) {
    // The minute ends in 5
    minuteColumn = 2;
    minuteRow = (minute - 5) / 10;
  } else if (minute % 10 == 0) {
    // The minute ends in 0
    minuteColumn = 3;
    if (minute == 0) {
      minuteRow = 5;
    } else {
      minuteRow = (minute / 10) - 1;
    }
  }
  ledOn(minuteRow, minuteColumn);

  // Set minute remainder
  // Take 6 instead of 5 to get a zero index row to set
  int minuteRemainder = (minute % 10) - 6;
  if (minuteRemainder >= 0) {
    ledOn(minuteRemainder, 4);
  }
}

void turnOnDayLeds()
{
  DateTime now = RTC.now();
  // Take 1 away from dayOfWeek to get 0 indexed row to light up
  int weekday = now.dayOfWeek() - 1;
  if (weekday < 5)
  {
    // Only light up Monday - Friday
    ledOn(weekday, 5);
  }
}
