# DIY Arduino Data Logging for Ecohydraulic Workshop

Welcome to the GitHub repository for the DIY Arduino Data Logging in Ecohydraulic Workshops! This repository contains a series of Arduino sketches designed to demonstrate various data logging techniques using ESP32, different sensors like DS18B20 (temperature sensor) and JS-SR04T (ultrasound sensor), SD card logging, web server functionality, and Real-Time Clock (RTC) modules.

## Project Overview

This repository is structured into multiple examples that showcase basic to advanced data logging setups suited for different environmental and ecological monitoring purposes using Arduino-based systems.

### Directory Structure

- **Advanced Examples**
  - **Advance_example_Logger_DS18B20**: Integrates an ESP32 with a DS18B20 temperature sensor, featuring SD card logging, web server functionality, and RTC for accurate timestamping. Supports data logging and remote monitoring over WiFi.
  - **Advance_example_Logger_JS_SR04T**: Similar setup as above but uses a JS-SR04T ultrasound sensor to measure distances.
- **Basic Examples**
  - **Basic_example_DS18B20**: Simple sketch for reading and printing temperature from a DS18B20 sensor every second.
  - **Basic_example_JS_SR04T**: Simple sketch for reading and printing distance measurements from a JS-SR04T ultrasound sensor every 10 seconds.
  - **Basic_example_Logger_DS18B20**: A basic data logging sketch with a DS18B20 temperature sensor, SD card logging, and RTC timestamping.
  - **Basic_example_Logger_JS_SR04T**: Features similar to the above but with a JS-SR04T ultrasound sensor.
  - **Basic_example_RTC_DS3231**: Demonstrates the usage of the DS3231 RTC module with ESP32, covering time setting, alarm configuration, and handling.
  - **Basic_example_SD**: Shows how to save data from a DS18B20 temperature sensor to an SD card.

## Getting Started

To get started with these examples, clone this repository to your local machine or download the ZIP file.

```bash
git clone https://github.com/MyCircuitsTV/DIY_Ecohydraulics.git
```
Each example has its associated libraries that need to be copied into your Arduino library folder (usually located at C:\Users\your_user\Documents\Arduino\libraries)

### Prerequisites

Ensure you have the following installed and set up:
- Arduino IDE: https://www.arduino.cc/en/software
- ESP32 board definitions installed in the Arduino IDE.

### Adding ESP32 Board to the Arduino IDE

To program the ESP32 using the Arduino IDE, you need to install the ESP32 board definitions. Follow these steps to add the ESP32 board to your Arduino IDE:

1. **Open Arduino IDE**: Launch the Arduino IDE on your computer.

2. **Access Preferences**: Navigate to `File` > `Preferences` in the Arduino IDE.

3. **Add ESP32 Board Manager URL**:
    - In the Preferences window, find the "Additional Board Manager URLs" field.
    - Paste the following URL into the field:  
      `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    - If there are already URLs present, click the icon on the right side of the field (a small window will open). Add the new URL on a new line.

4. **Install ESP32 Platform**:
    - Open the Board Manager by going to `Tools` > `Board` > `Boards Manager...`
    - In the Boards Manager, type "ESP32" into the search bar.
    - Find the entry for "esp32 by Espressif Systems" and click the "Install" button to install the ESP32 platform.

5. **Select Your ESP32 Board**:
    - After installation, go to `Tools` > `Board` and select the ESP32 board model you are using from the list under "ESP32 Arduino".
	- For the workshop, the board will be: `ESP32 Dev Module`

6. **Verify Installation**:
    - Connect your ESP32 board to the computer via a USB cable.
    - Select the correct port under `Tools` > `Port`.
    - You can now upload sketches to your ESP32 board using the Arduino IDE.

Now, your Arduino IDE is set up to work with ESP32 boards, and you can proceed with uploading your sketches to the board. 

## License

This project is licensed under the GNU License - see the [LICENSE.md](LICENSE) file for details.

## Contact

If you have any questions or want to give feedback, please [open an issue](https://github.com/MyCircuitsTV/DIY_Ecohydraulics/issues) here on GitHub.