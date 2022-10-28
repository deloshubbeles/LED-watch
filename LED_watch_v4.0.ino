/* LED WATCH
Two button LED watch with the aim of having the following functios:
  - Time and day of the week
  - ^Multi-button functionality
  - ^Low power / sleep* (incomplete)
  - ^Low battery warning
  - ^Light (4 white LEDs)
  - ^Stopwatch
  - ^Timer
  ^incomplete  

MCU: Microchip ATmega328p
RTC: M41T62
*/

/// Libraries
#include "Wire.h"                /// I2C connections
#include "M41T62.h"              /// M41T62 real time clock
#include "PinChangeInterrupt.h"  ///pin change interrupt
#include "LowPower.h"            /// low power sleep
#include "OneButton.h"           /// multi-button functionality
RTC_M41T62 RTC;                  /// real time clock

#define NUM_COL 6   /// columns are anodes
#define NUM_ROW 6   /// rows are cathodes
#define COL_ON LOW  /// the following four lines of code control swtiching the multiplexed LEDs on and off
#define COL_OFF HIGH
#define ROW_ON HIGH
#define ROW_OFF LOW

const int colLED[NUM_COL] = { 2, 3, 4, 5, 6, 7 };      /// pins for anode (+) connections on ATmega328p, columns in LED matrix
const int rowLED[NUM_ROW] = { 0, 1, A0, A1, A2, A3 };  /// pins for cathode (-) connections on ATmega328p, rows in LED matrix
//const int hourRows = 6;      /// hour rows
//const int hourCols = 2;      /// hour columns
const int button1 = 9;   /// pin for button 1
const int button2 = 10;  /// pin for button 2

volatile int hourRow;
volatile int hourColumn;
volatile int minuteRow;
volatile int minuteColumn;
volatile int minRemain;
volatile int wkDay;

int minuteRemainder;
int weekday;
int lastButtonState = LOW;  // the previous reading from the input pin
//int hourArray[hourRows][hourCols] = { { 1, 2 }, { 3, 4 }, { 5, 6 }, { 7, 8 }, { 9, 10 }, { 11, 12 } };
int t = 5;
int t2 = 10;
int t3 = 15;
int tOn = 200;
int tOff = 20;
uint8_t display[NUM_ROW][NUM_COL];  // this array holds the current image to display

unsigned long previousMillis = 0;
unsigned long previousMillisT = 0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long onTime = 20 * 1000;    /// duration time is displayed before going to deep sleep
unsigned long sleepTimer = 0;        /// keep time of how long watch has been awake

const long interval1 = 400;  /// duration of button 1 function
const long interval2 = 600;  /// duration of button 2 function

bool myFlag1 = false;  /// create flag1 for pin change interrupt
bool myFlag2 = false;  /// create flag2 for pin change interrupt
bool awake = false;
volatile bool wakeUp = false;

//OneButton button1(9, true);
//OneButton button2(10, true);

/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/// - - - - - - - - - - - - - - - setup + loop - - - - - - - - - - - - - - - -

/// No access to serial monitor

void setup() {
  pinMode(button1, INPUT_PULLUP);            /// button1 pulled LOW when pressed
  pinMode(button2, INPUT_PULLUP);            /// button2 pulled LOW when pressed
  Wire.begin();                              /// I2C communication with the RTC
  RTC.begin();                               /// start RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));  /// set RTC time to computer time

  /*
  /// link the button 1 functions
  button1.attachClick(singleClick1);
  button1.attachDoubleClick(doubleClick1);
  button1.attachMultiClick(multiClick1);
  button1.attachLongPressStart(pressStart1);
  button1.attachLongPressStop(pressStop1);
  button1.attachDuringLongPress(longPress1);

  /// link the button 2 functions
  button2.attachClick(singleClick2);
  button2.attachDoubleClick(doubleClick2);
  button2.attachMultiClick(multiClick2);
  button2.attachLongPressStart(pressStart2);
  button2.attachLongPressStop(pressStop2);
  button2.attachDuringLongPress(longPress2);
*/
  for (int i = 0; i < NUM_COL; i++)  /// set all column pins to OUTPUT and OFF
  {
    pinMode(colLED[i], OUTPUT);        /// set column LEDs to output
    digitalWrite(colLED[i], COL_OFF);  /// turn all columns off
  }
  for (int j = 0; j < NUM_ROW; j++)  /// set all row pins to OUTPUT and OFF
  {
    pinMode(rowLED[j], OUTPUT);        /// set row LEDs to output
    digitalWrite(rowLED[j], ROW_OFF);  /// turn all rows off
  }
  attachPCINT(digitalPinToPCINT(button1), setFlag1, CHANGE);  /// interrupt when button 1's state changes
  attachPCINT(digitalPinToPCINT(button2), setFlag2, CHANGE);  /// interrupt when button 2's state changes
}

void loop() {
  //getTime();  /// check that getting the time is not an energy-demanding task, plus implement deep sleep mode

  /// If the switch changed, due to noise or pressing:
  if (digitalRead(button1) != lastButtonState) {
    /// reset the debouncing timer
    lastDebounceTime = millis();


    if (myFlag1 == true)  /// if myFlag1 is true, display time
    {
      /*
    if ((millis() - lastDebounceTime) > debounceDelay) {
      /// whatever the reading is at, it's been there for longer than the debounce
      /// delay, so take it as the actual current state:
*/
      displayTime();
      unsigned long currentMillis = millis();           /// set the millis riiiiight now!
      if (currentMillis - previousMillis >= interval1)  /// if now minus the time when the next step is over is longer than <duration>
      {
        allOff();
        previousMillis = currentMillis;
        myFlag1 = false;  /// flag off
      }
    }
  }

  /// If the switch changed, due to noise or pressing:
  if (digitalRead(button2) != lastButtonState) {
    /// reset the debouncing timer
    lastDebounceTime = millis();

    if (myFlag2 == true)  /// if myFlag2 is true, display time
    {
      displayTimeBright();
      unsigned long currentMillisT = millis();            /// set the millis riiiiight now!
      if (currentMillisT - previousMillisT >= interval2)  /// if now minus the time when the next step is over is longer than <duration>
      {
        allOff();
        previousMillisT = currentMillisT;
        myFlag2 = false;  /// flag off
      }
    }
  }

  if (myFlag1 == false && myFlag2 == false)  /// if myFlag1 and myFlag2 are false, go to sleep
  {
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);  /// Zzzzzzz
  }
}

/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/// - - - - - - - - - - - - - - - button functions - - - - - - - - - - - - - - -

void setFlag1() {
  myFlag1 = true;
}

void setFlag2() {
  myFlag2 = true;
}
/*
/// button1
void singleClick1() {  /// function called when button pressed once
}
void doubleClick1() {  /// function called when button was pressed two times in a short timeframe
}
void multiClick1() {  /// function called when button pressed multiple (>2) times in a short timeframe
}
void pressStart1() {  /// function called when button held down for one second or more
}
void pressStop1() {  /// function called when button released after long hold
}

/// button2
void singleClick2() {  /// function called when button pressed once
}
void doubleClick2() {  /// function called when button was pressed two times in a short timeframe
}
void multiClick2() {  /// function called when button pressed multiple (>2) times in a short timeframe
}
void pressStart2() {  /// function called when button held down for one second or more
}
void pressStop2() {  /// function called when button released after long hold
}
*/

/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/// - - - - - - - - - - - - - - - time functions - - - - - - - - - - - - - - - -

void getTime() {
  static int lastMinute = -1;
  DateTime now = RTC.now();  /// set "now" from the computer's time

  if (now.minute() == lastMinute)
    return;  /// No need to re-calculate if the time has not changed

  lastMinute = now.minute();
  int hour = now.hour() % 12;
  int minute = now.minute() - 5;
  weekday = now.dayOfWeek() - 1;

  /// set hour
  if (hour == 0)
    hour = 12;
  hourRow = (hour - 1) / 2;
  hourColumn = (hour - 1) % 2;

  /// set minute --- something not quite right here
  minuteRow = minute / 10;
  minuteColumn = 2 + ((minute % 10) > 5);
  // minuteColumn = 2 + ((minute - 1) / 5)  % 2;
  minuteRemainder = minute % 5;
}

/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/// - - - - - - - - - - - - - - - display functions - - - - - - - - - - - - - -

/// turn on specific LEDs
inline void ledOn(int row, int col) {
  digitalWrite(rowLED[row], HIGH);    /// row pin to +3V
  digitalWrite(colLED[col], LOW);     /// column pin to 0V
  //allOff();
}

/// turn off specific LEDs
inline void ledOff(int row, int col) {
  digitalWrite(rowLED[row], LOW);     /// row pin to 0V
  digitalWrite(colLED[col], HIGH);    /// column pin to +3V
}

/// turn on all LEDs
void allOn() {
  for (int i = 0; i < NUM_COL; i++) {
    digitalWrite(colLED[i], COL_ON);    /// all columns to 3V
  }
  for (int j = 0; j < NUM_ROW; j++) {
    digitalWrite(rowLED[j], ROW_ON);    /// all rows to 3V
  }
}

/// turn off all LEDs
void allOff() {
  for (int i = 0; i < NUM_ROW; i++) {
    digitalWrite(rowLED[i], ROW_OFF);     /// all row pins to 0V
  }
  for (int j = 0; j < NUM_COL; j++) {
    digitalWrite(colLED[j], COL_OFF);     /// all column pins to 0V
  }
}

void displayTime() {  /// still slight flicker to the LEDs, especially when viewed up close. Decreasing the delay, dims the LEDs 
  getTime();
  allOff();
  ledOn((int)hourRow, (int)hourColumn);
  delayMicroseconds(150);
  ledOff((int)hourRow, (int)hourColumn);
  ledOn((int)minuteRow, (int)minuteColumn);
  delayMicroseconds(150);
  ledOff((int)minuteRow, (int)minuteColumn);
  ledOn((int)minuteRemainder, 4);
  delayMicroseconds(100);
  ledOff((int)minuteRemainder, 4);
  ledOn((int)weekday, 5);
  delayMicroseconds(150);
  ledOff((int)weekday, 5);
}

void displayTimeBright() {  /// this makes the LEDs bright, but turns on additional LEDs for some reason
  getTime();
  ledOn((int)hourRow, (int)hourColumn);
  ledOn((int)minuteRow, (int)minuteColumn);
  ledOn((int)minuteRemainder, 4);
  ledOn((int)weekday, 5);
}

void ledTest()  /// tests all LEDS by running through each one
{
  for (int i = 0; i < NUM_ROW; i++) {
    for (int j = 0; j < NUM_COL; j++) {
      allOff();                      /// should I be turning all the LEDs off each time, or just one at a time
      digitalWrite(rowLED[i], LOW);  /// turns off each LED
      digitalWrite(colLED[j], HIGH);
      delayMicroseconds(1000);
      digitalWrite(rowLED[i], HIGH);  /// turns on each LED
      digitalWrite(colLED[j], LOW);
      delayMicroseconds(1000);
    }
  }
}

/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/// - - - - - - - - - - - - - - - - power functions - - - - - - - - - - - - - -

/* "It's a ATmega part so you should be able to do this internally in code.
Atmel had a application note for this. The issue is always calibrating the bandgap 1.1v reference.
If it's a one off a manual calibration is easy enough. You set the ADC to measure the known backgap
reference while having the ADC reference set to VCC. Run the calc and you should get a battery voltage." -- Wayne Peacock
*/

const long InternalReferenceVoltage = 1062;  /// Adjust this value to your board's specific internal BG voltage

/// results are Vcc * 100, so 3.3V would be 330?
int getBandgap() {
  /// REFS0 : Selects AVcc external reference
  /// MUX3 MUX2 MUX1 : Selects 1.1V (VBG)
  ADMUX = bit(REFS0) | bit(MUX3) | bit(MUX2) | bit(MUX1);
  ADCSRA |= bit(ADSC);           /// start conversion
  while (ADCSRA & bit(ADSC)) {}  /// wait for conversion to complete
  int results = (((InternalReferenceVoltage * 1024) / ADC) + 5) / 10;
  return results;
}  /// end of getBandgap


void lowBatteryX()  /// this needs to interrupt deep sleep mode
{
  ///if (results < /*put low voltage number here*/ )
  // {
  allOff();
  digitalWrite(rowLED[5], HIGH);  /// cathode pin to +5V
  digitalWrite(colLED[5], LOW);   /// anode pin to 0V
  delay(1);
  digitalWrite(rowLED[5], LOW);   /// cathode pin to 0V
  digitalWrite(colLED[5], HIGH);  /// anode pin to 5V
  delay(10);
  // }
}

void lowBattery()  /// row (x) then column (y) -- runs across days of the week
{
  for (int i = 0; i < 5; i++) {
    ledOn(5, 5);
    delay(5);
    ledOff(5, 5);
  }
  delay(50);
}

//------------------------------------------------------------------------

int time = 1;
int dtime1 = 10;
int dtime2 = 10000;

// matrix waterfall display
void DisplayMatrix() {

  for (int t = 0; t < time; t++) {
    //for (int r = random(5); r >= 0; r--) {
    for (int c = 0; c < random(10); c++) {
      /*
        ledOn(c, r);
        delayMicroseconds(dtime2);
        ledOff(c, r);
        delayMicroseconds(dtime1);
        */
      for (int i = 0; i < 5; i++) {  /// increment LED five times
        {
          int r = random(4);
          ledOn(c, r);
          delayMicroseconds(100);  /// brightest
          allOff();
          delay(2);

          ledOn(c, r);
          delayMicroseconds(50);  /// mid
          allOff();
          delay(2);

          ledOn(c, r);
          delayMicroseconds(10);  /// dimmest
          allOff();
          delay(2);
        }
        uint8_t rnd = byte(random(256));
        for (int r = 0; r < 8; r++) {
          ledOn(random(5), r);
          delayMicroseconds(50);
          ledOff(c, r);
        }
      }
    }
    //}
  }
}