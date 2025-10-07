# Arduino IDE Setup Guide

Complete setup guide for ESP32 System Monitor using Arduino IDE.

## Prerequisites

- Windows, macOS, or Linux computer
- ESP32 development board
- ST7789 TFT display (240x320)
- USB cable for programming
- Internet connection

## Step 1: Install Arduino IDE

1. Download Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
   - Version 1.8.19 or later (or Arduino IDE 2.x)

2. Install the IDE following the installer instructions

## Step 2: Install ESP32 Board Support

1. Open Arduino IDE

2. Go to **File → Preferences**

3. In "Additional Board Manager URLs" field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

4. Click **OK**

5. Go to **Tools → Board → Boards Manager**

6. Search for "**esp32**"

7. Install "**esp32 by Espressif Systems**" (version 2.0.x or later)

8. Wait for installation to complete

## Step 3: Install Required Libraries

Go to **Sketch → Include Library → Manage Libraries** and install:

### 1. TFT_eSPI
- **Search for:** TFT_eSPI
- **Author:** Bodmer
- **Version:** 2.5.43 or later
- Click **Install**

### 2. ArduinoJson
- **Search for:** ArduinoJson
- **Author:** Benoit Blanchon
- **Version:** 6.21.3 or later
- Click **Install**

### 3. NimBLE-Arduino
- **Search for:** NimBLE-Arduino
- **Author:** h2zero
- **Version:** 1.4.1 or later
- Click **Install**

## Step 4: Configure TFT_eSPI Library

The TFT_eSPI library needs to be configured for the ST7789 display.

### Option A: Copy Provided Configuration (Recommended)

1. Locate your Arduino libraries folder:
   - **Windows:** `C:\Users\YourName\Documents\Arduino\libraries\TFT_eSPI`
   - **macOS:** `~/Documents/Arduino/libraries/TFT_eSPI`
   - **Linux:** `~/Arduino/libraries/TFT_eSPI`

2. **Backup** the existing `User_Setup.h` file (rename it to `User_Setup.h.backup`)

3. **Copy** the `User_Setup.h` file from the MonitorScreen project folder to the TFT_eSPI library folder

### Option B: Manual Configuration

1. Open the TFT_eSPI library folder (location above)

2. Open `User_Setup.h` in a text editor

3. Find and **uncomment** (remove `//`) this line:
   ```cpp
   #define ST7789_DRIVER
   ```

4. Find and **comment out** (add `//`) any other driver definitions like:
   ```cpp
   // #define ILI9341_DRIVER
   // #define ST7735_DRIVER
   ```

5. Find the pin definitions section and set:
   ```cpp
   #define TFT_MOSI 23
   #define TFT_SCLK 18
   #define TFT_CS   15
   #define TFT_DC   2
   #define TFT_RST  4
   ```

6. Set display resolution:
   ```cpp
   #define TFT_WIDTH  240
   #define TFT_HEIGHT 320
   ```

7. **Save** the file

## Step 5: Open and Configure the Sketch

1. Navigate to the MonitorScreen project folder

2. Open `MonitorScreen.ino` in Arduino IDE
   - The IDE will automatically load all .cpp and .h files in the same folder

3. Go to **Tools** menu and configure:
   - **Board:** "ESP32 Dev Module"
   - **Upload Speed:** 921600
   - **CPU Frequency:** 240MHz (WiFi/BT)
   - **Flash Frequency:** 80MHz
   - **Flash Mode:** QIO
   - **Flash Size:** 4MB (32Mb)
   - **Partition Scheme:** Default 4MB with spiffs
   - **Core Debug Level:** None
   - **Port:** Select your ESP32's COM port

## Step 6: Verify Compilation

1. Click the **Verify** button (checkmark icon) to compile

2. Wait for compilation to complete

3. Check for any errors in the output window
   - If you see errors about missing libraries, verify Step 3
   - If you see TFT_eSPI errors, verify Step 4

## Step 7: Upload to ESP32

1. Connect your ESP32 board via USB

2. Press and hold the **BOOT** button on ESP32 (if required by your board)

3. Click the **Upload** button (right arrow icon)

4. Wait for upload to complete

5. The ESP32 will restart automatically

## Step 8: Verify Operation

1. Open **Tools → Serial Monitor**

2. Set baud rate to **115200**

3. You should see:
   ```
   === ESP32 System Monitor ===
   Type 'help' for available commands
   ```

4. The display should show "Waiting for data..."

## Troubleshooting

### Compilation Errors

**Error: "TFT_eSPI.h: No such file or directory"**
- Solution: Install TFT_eSPI library (Step 3)

**Error: "ArduinoJson.h: No such file or directory"**
- Solution: Install ArduinoJson library (Step 3)

**Error: Multiple definition errors**
- Solution: Make sure all .cpp and .h files are in the same folder as the .ino file

### Upload Errors

**Error: "Failed to connect to ESP32"**
- Press and hold BOOT button while uploading
- Check USB cable and port selection
- Try a different USB cable or port

**Error: "A fatal error occurred: Timed out waiting for packet header"**
- Press BOOT button on ESP32
- Reduce upload speed to 115200

### Display Issues

**Display shows nothing**
- Verify TFT_eSPI configuration (Step 4)
- Check wiring connections
- Verify 3.3V power supply

**Display shows garbage**
- Verify ST7789_DRIVER is defined
- Check pin definitions match your wiring
- Try reducing SPI_FREQUENCY in User_Setup.h

**Wrong colors**
- Check TFT_INVERSION_ON setting in User_Setup.h
- Verify ST7789 driver (not ST7735)

## Next Steps

1. Configure WiFi credentials via Serial Monitor:
   ```
   setwifi YourNetworkName YourPassword
   ```

2. Set up the PC application (see main README.md)

3. Start monitoring your system!

## Additional Resources

- [Arduino ESP32 Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [TFT_eSPI Documentation](https://github.com/Bodmer/TFT_eSPI)
- [ArduinoJson Documentation](https://arduinojson.org/)
- Main project README.md for usage instructions
