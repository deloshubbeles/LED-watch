# LED Watch

### Objective
To display the time and day of week on multiplexed LEDs with button press.

**Other intended functionality**
- Flash low battery warning LED when battery voltage drops too low
- Display LEDs at different brightnesses depending on time of day or button pressed (e.g., brighter in daylight, dimmer at night)
- Stopwatch

### Main components
- [ATmega328P-MMHR (28-pin VQFN)](https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061B.pdf)
- [M41T62 (8 pin LCC) real time clock](https://www.st.com/resource/en/datasheet/m41t62.pdf)
- 34 multiplexed LEDs (18 red, 12 blue, 4 white)
- CR2016 coin cell battery, ~90mAh
- 2 x SPST tactile switches

### Schematic
![image](https://user-images.githubusercontent.com/46733227/168943393-ea5b9d96-fc0a-4f28-aba8-48e624d1ed2b.png)

### PCB layers:
![Watch layers](https://user-images.githubusercontent.com/46733227/168943533-c018bb24-1415-47a0-b694-8574185afbfe.png)

### Displaying the time
![time](https://user-images.githubusercontent.com/46733227/168943461-fcb681b0-c0bd-46a5-9950-854870a2dffe.jpg)
