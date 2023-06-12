# LED Watch

### Objective
To display the time and day of week on multiplexed LEDs with button presses, plus more.

**Other intended functionality (yet to be implemented)**
- Low power sleep mode to conserve power when inactive
- Flash low battery warning LED when battery voltage drops too low
- Display LEDs at different brightnesses depending on time of day or button pressed (e.g., brighter in daylight, dimmer at night)
- Stopwatch
- Countdown timer

### Displaying the time
![time](https://user-images.githubusercontent.com/46733227/168943461-fcb681b0-c0bd-46a5-9950-854870a2dffe.jpg)

### Components
- [ATmega328P-MMHR (28-pin VQFN)](https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061B.pdf)
- [M41T62 (8 pin LCC) real time clock](https://www.st.com/resource/en/datasheet/m41t62.pdf)
- 34 multiplexed LEDs (18 red, 12 blue, 4 white)
- CR2016 coin cell battery (~90mAh) and battery holder
- 2 x SPST tactile switches

### Schematic
![image](https://user-images.githubusercontent.com/46733227/197370058-bacf566c-479a-4a38-be9c-4085ebf350f6.jpeg)

### PCB layers:
![Watch layers](https://user-images.githubusercontent.com/46733227/168943533-c018bb24-1415-47a0-b694-8574185afbfe.png)
