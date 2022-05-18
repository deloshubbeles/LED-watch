/// Displaying time (5 minutes fast) with consistent brightness
/// No flicker using TIMER_FREQ_HZ values of 1.5k to 10k, being dimmer and brighter, respectively.

#define TIMER_INTERRUPT_DEBUG         2
#define _TIMERINTERRUPT_LOGLEVEL_     0
#define USE_TIMER_1     true

#include "Wire.h"                    /// I2C library
#include "M41T62.h"                  /// RTC library for M41T62 - NB: Use "dayOfWeek"
#include "LowPower.h"                /// Low power library for deep sleep mode
#include <TimerInterrupt.h>          /// Library for timer interrupt
#include "OneButton.h"               /// Library for button functionality
RTC_M41T62 RTC;

#define TIMER_FREQ_HZ        10000.0        /// ~15k starts to act funny, 10k nice and bright, 1.5k dim but not flickering, 500 starts to flicker

#define NUM_COL       6                /// Columns are anodes
#define NUM_ROW       6                /// Rows are cathodes
#define COL_ON    LOW                 /// badge code has these four inverted
#define COL_OFF   HIGH
#define ROW_ON    HIGH
#define ROW_OFF   LOW

const int button1 = 9;
const int button2 = 10;

const int colLED[NUM_COL] = {2, 3, 4, 5, 6, 7};          /// pins for anode (+) connections on ATmega328, columns in LED matrix
const int rowLED[NUM_ROW] = {0, 1, A0, A1, A2, A3};      /// pins for cathode (-) connections on ATmega328, rows in LED matrix

const int hourRows = 6;
const int hourCols = 2;

int rowPin = 6;
int colPin = 6;
volatile int hourRow;
volatile int hourColumn;
volatile int minuteRow;
volatile int minuteColumn;
volatile int minRemain;
volatile int wkDay;
int minuteRemainder;
int weekday;

int hourArray[hourRows][hourCols] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}, {9, 10}, {11, 12}};

/// - - - - - - - - - - - SETUP + LOOP - - - - - - - - - - - - - - - -

/// No access to serial monitor

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
  ITimer1.init();                               /// initialise timer 1
  ITimer1.attachInterrupt(TIMER_FREQ_HZ, TimerHandler);
}

void loop() {
  getTime();
}

void TimerHandler() {
  displayTime();
  //cylon();
  //processing();
  //allOn();
  //lowBattery();
}

/// - - - - - - - - - - - display functions - - - - - - - - - - - - - - - -

void allOn() {                            /// turns on all LEDs
  /// all cathode pins to 5V
  for (int i = 0; i < NUM_COL; i++) {
    digitalWrite(colLED[i], COL_ON);
  }
  /// all cathode pins to 0V
  for (int j = 0; j < NUM_ROW; j++) {
    digitalWrite(rowLED[j], ROW_ON);
  }
}

void allOff() {                             /// turns off all LEDs
  /// all cathode pins to 0V
  for (int i = 0; i < rowPin; i++) {
    digitalWrite(rowLED[i], ROW_OFF);
  }
  /// all anode pins to +5V
  for (int j = 0; j < 1; j++) {
    digitalWrite(colLED[j], COL_OFF);
  }
}

inline void ledOn(int row, int col) {      /// turns on specific LEDs
  digitalWrite(rowLED[row], HIGH);        /// cathode pin to +5V
  digitalWrite(colLED[col], LOW);         /// anode pin to 0V
  //allOff();
}

inline void ledOff(int row, int col) {      /// turns on specific LEDs
  digitalWrite(rowLED[row], LOW);        /// cathode pin to +5V
  digitalWrite(colLED[col], HIGH);         /// anode pin to 0V
}

void ledOnCylon(int row, int col)                /// turns on specfic LEDs
{
  allOff();
  digitalWrite(rowLED[row], HIGH);        /// cathode pin to +5V
  digitalWrite(colLED[col], LOW);         /// anode pin to 0V
  //digitalWrite(rowLED[row], LOW);
  //digitalWrite(colLED[col], HIGH);
}

/// - - - - - - - - - - - - - - - time functions - - - - - - - - - - - - - - - -

void getTime()
{
  static int lastMinute = -1;
  DateTime now = RTC.now();

  if (now.minute() == lastMinute)
    return;  /// No need to re-calculate if the time has not changed

  lastMinute = now.minute();

  int hour = now.hour() % 12;
  int minute = now.minute();
  weekday = now.dayOfWeek() - 1;

  /// set hour
  if (hour == 0)
    hour = 12;
  hourRow = (hour - 1) / 2;
  hourColumn = (hour - 1) % 2;

  /// set minute
  minuteRow = minute / 10;
  minuteColumn = 2 + ((minute - 1) / 5)  % 2;
  minuteRemainder = minute % 5;
}

void displayTime() {
  ledOn((int)hourRow, (int)hourColumn);
  ledOff((int)hourRow, (int)hourColumn);
  ledOn((int)minuteRow, (int)minuteColumn);
  ledOff((int)minuteRow, (int)minuteColumn);
  ledOn((int)minuteRemainder, 4);
  ledOff((int)minuteRemainder, 4);
  ledOn(weekday, 5);
  ledOff(weekday, 5);
}

/// - - - - - - - - - - - pattern functions - - - - - - - - - - - - - - - -

void cylon() /// row (x) then column (y) -- runs across days of the week
{
  for (int i = 0; i < 5; i++)
  {
    ledOnCylon(i, 5);
    delay(1000);
  }
  for (int j = 4; j >= 0; j--)
  {
    ledOnCylon(j, 5);
    delay(1000);
  }
}

void processing()
{
  for (int i = random(0, 6); i < NUM_ROW; i++)
  {
    for (int j = random(0, 6); j < NUM_COL; j++) {
      digitalWrite(rowLED[i], HIGH);
      digitalWrite(colLED[j], LOW);
      delay(1000);
      digitalWrite(rowLED[j], LOW);
      digitalWrite(colLED[i], HIGH);
    }
  }
  for (int j = random(1, 6); j < NUM_ROW; j++)
  {
    digitalWrite(rowLED[j], HIGH);
    digitalWrite(colLED[j], HIGH);
    delay(1000);
  }
  delay(2500);
}

/// - - - - - - - - - - - - power functions - - - - - - - - - - - - - - -

const long InternalReferenceVoltage = 1062;  /// Adjust this value to your board's specific internal BG voltage

/// results are Vcc * 100, so 5V would be 500
int getBandgap ()
{
  /// REFS0 : Selects AVcc external reference
  /// MUX3 MUX2 MUX1 : Selects 1.1V (VBG)
  ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
  ADCSRA |= bit( ADSC );  /// start conversion
  while (ADCSRA & bit (ADSC))
  { }  /// wait for conversion to complete
  int results = (((InternalReferenceVoltage * 1024) / ADC) + 5) / 10;
  return results;
} /// end of getBandgap

void lowBattery() /// this needs to interrupt deep sleep mode
{
  ///if (results < /*put low voltage number here*/ )
  // {
  allOff();
  digitalWrite(rowLED[5], HIGH);        /// cathode pin to +5V
  digitalWrite(colLED[5], LOW);         /// anode pin to 0V
  delay(1);
  digitalWrite(rowLED[5], LOW);        /// cathode pin to 0V
  digitalWrite(colLED[5], HIGH);         /// anode pin to 5V
  delay(100000);
  // }
}

/// - - - - - - - - - - - - button functions - - - - - - - - - - - - - - -

/// when button pressed once
void singleClick()
{
}

/// when button was pressed two times in a short timeframe
void doubleClick()
{
}

/// when button pressed multiple (>2) times in a short timeframe
void multiClick()
{
}

/// when button held down for one second or more
void pressStart()
{
}

/// function called when button released after long hold
void pressStop()
{
}
