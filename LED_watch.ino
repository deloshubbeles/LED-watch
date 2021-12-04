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

const int colLED[NUM_COL] = {2, 3, 4, 5, 6, 7};          /// pins for anode (+) connections on ATmega328, columns in LED matrix
const int rowLED[NUM_ROW] = {0, 1, A0, A1, A2, A3};      /// pins for cathode (-) connections on ATmega328, rows in LED matrix

int colIdx = 0;                               // Index of column to be refreshed
uint8_t display[NUM_ROW][NUM_COL];            // Array holding what to display

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


/// - - - - - - - - - - - ROUTINES - - - - - - - - - - - - - - - -

/// Draws each column but so frequently you can't tell
static void RefreshDisplay() {
  bool rowState = ROW_ON;
  int c, r, rowPtr;

  // De-select old column
  for (int c = 0; c < NUM_COL; c++)
    digitalWrite(colLED[c], COL_OFF);

  // De-select old rows
  for (int r = 0; r < NUM_ROW; r++)
    digitalWrite(rowLED[r], ROW_OFF);

  colIdx = (colIdx + 1) % NUM_COL;

  // Select current column
  digitalWrite(colLED[colIdx], COL_ON);
  c = colIdx;
}

/// - - - - - - - - - - - SETUP + LOOP - - - - - - - - - - - - - - - -

/// REMINDER: no access to serial monitor

void setup() {
  Wire.begin();                                 /// I2C communication with the RTC
  pinMode(button1, INPUT_PULLUP);               /// button1 pulled LOW when pressed
  pinMode(button2, INPUT_PULLUP);               /// button2 pulled LOW when pressed
  RTC.begin();                                  /// start RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));     /// set RTC time to computer time

  for (int i = 0; i < NUM_COL; i++)              /// set all column pins to OUTPUT and OFF
  {
    pinMode(colLED[i], OUTPUT);                 /// output
    digitalWrite(colLED[i], COL_OFF);           /// turn off
  }
  for (int j = 0; j < NUM_ROW; j++)              /// set all row pins to OUTPUT and OFF
  {
    pinMode(rowLED[j], OUTPUT);                 /// output
    digitalWrite(rowLED[j], ROW_OFF);           /// turn off
  }
}

void loop() {
  //allOff();
  displayTime();

  //attachInterrupt(digitalPinToInterrupt(button1), wakeUp, LOW);             /// allow button1 to trigger interrupt (wake up) when LOW
  //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);                      /// enter power down state with Analog to Digital Converter (ADC) and Brown-out Detection (BOD) modules disabled
  //detachInterrupt(digitalPinToInterrupt(button1));                          /// disable external pin interrupt on wake up pin

  //int buttonState1 = digitalRead(button1);
  //if (buttonState1 == LOW) {
  //allOff();

  //} else {

  //ClearDisplay();
  //displayTime();
  //cylonTest();
  //runnerWatch();
  //matrix();
  //processing();
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


/// - - - - - - - - - - - display functions - - - - - - - - - - - - - - - -
    /// allOff
    /// ledOn
    /// ledOff

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

/// - - - - - - - - - - - - - - - time functions - - - - - - - - - - - - - - - -
/// displayTime
/// turnOnHourLeds
/// turnOnMinuteLeds
/// turnOnRemainderLeds
/// turnOnDayLeds

void displayTime()
{
  DateTime now = RTC.now();
  int hour = now.hour();
  if (hour > 12) {
    hour -= 12;
  };
  int minute = (now.minute());
  int weekday = now.dayOfWeek() - 1;
  int minuteRemainder = (minute % 10) - 5;

  turnOnHourLeds(hour);
  delayMicroseconds(100);
  turnOnMinuteLeds(minute);
  delayMicroseconds(100);
  turnOnDayLeds(weekday);
  delayMicroseconds(100);
  turnOnRemainderLeds(minuteRemainder);
  delayMicroseconds(100);
}

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
  /// Take 6 instead of 5 to get a zero index row to set
  if (minuteRemainder >= 0) {
    ledOn(minuteRemainder, 4);
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

/// - - - - - - - - - - - - other functions - - - - - - - - - - - - - - -
/// runnerWatch
/// cylonTest
/// processing
/// matrix

void runnerWatch()                                   /// tests all LEDS by running through each one
{
  for (int i = 0; i < rowPin2; i++)
  { for (int j = 0; j < colPin; j++)

    {
      //allOff();
      digitalWrite(rowLED[i], HIGH);                  /// turns on each LED
      digitalWrite(colLED[j], LOW);
      delay(10);
      digitalWrite(rowLED[i], LOW);                  /// turns off each LED
      digitalWrite(colLED[j], HIGH);
      //delay(1);
    }
  }
  delay(10);

  for (int j = 6; j < colPin; j--)
  { for (int i = 6; i < rowPin2; i--)

    {
      //allOff();
      digitalWrite(rowLED[i], HIGH);                  /// turns on each LED
      digitalWrite(colLED[j], LOW);
      delay(1);
      digitalWrite(rowLED[i], LOW);                  /// turns off each LED
      digitalWrite(colLED[j], HIGH);
      //delay(1);
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

void processing()
{
  for (int i = random(0, 6); i < 6; i++)
  {
    digitalWrite(rowLED[i], HIGH);
    digitalWrite(colLED[i], LOW);
    delay(random(4, 5));
  }

  for (int k = random(0, 6); k >= 0; k--)
  {
    digitalWrite(colLED[k], HIGH);
    digitalWrite(rowLED[k], LOW);
    delay(random(4, 5));
  }
  delay(random(4, 5));
}

void matrix()                /// turns on specfic LEDs
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
