/// Displaying time (5 minutes fast) with consistent brightness
/// No flicker using TIMER_FREQ_HZ values of 1k to 10k, being dimmer and brighter, respectively.

#define TIMER_INTERRUPT_DEBUG         2
#define _TIMERINTERRUPT_LOGLEVEL_     0
#define USE_TIMER_1     true

#include "Wire.h"                    /// I2C library
#include "M41T62.h"                  /// RTC library for M41T62 - NB: Use "dayOfWeek"
#include <TimerInterrupt.h>
RTC_M41T62 RTC;

#define TIMER_FREQ_HZ        10000.0        /// ~15k starts to act funny, 10k nice and bright, 1.5k dim but not flickering, 500 starts to flicker

#define NUM_COL       6                /// Columns are anodes
#define NUM_ROW       6                /// Rows are cathodes
#define COL_ON    LOW                 /// badge code has these four inverted
#define COL_OFF   HIGH
#define ROW_ON    HIGH
#define ROW_OFF   LOW

const int colLED[NUM_COL] = {2, 3, 4, 5, 6, 7};          /// pins for anode (+) connections on ATmega328, columns in LED matrix
const int rowLED[NUM_ROW] = {0, 1, A0, A1, A2, A3};      /// pins for cathode (-) connections on ATmega328, rows in LED matrix

const int hourRows = 6;
const int hourCols = 2;

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
  for (int i = 0; i < NUM_ROW; i++) {
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

  /// Set minute
  minuteRow = minute / 10;
  minuteColumn = 2 + ((minute - 1) / 5)  % 2;
  //minuteColumn = 2 + ((minute % 10) > 5);
  
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
