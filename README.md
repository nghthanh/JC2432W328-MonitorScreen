# ESP32 System Monitor

A comprehensive PC system monitoring solution using ESP32 microcontroller with ST7789 TFT display (240x320). Monitor your computer's CPU, memory, disk, and network usage in real-time with multiple display themes and alert notifications.

## Features

### Hardware
- **ESP32 microcontroller** based
- **ST7789 TFT display** (240x320 resolution)
- Serial interface for CLI configuration
- LED support for visual alerts

### Communication
- **WiFi Mode**: UDP communication with PC
- **BLE Mode**: Bluetooth Low Energy communication
- Selectable interface via CLI or web configuration
- JSON data format for extensibility

### Display
- **Multiple Themes**:
  - Default: Progress bars with detailed information
  - Minimal: Large numbers, clean layout
  - Graph: Historical data visualization
  - Compact: Dense information with small graphs
- **Date/Time Display**:
  - Real-time clock display on all screens
  - Manual time setting via CLI/Web
  - Automatic NTP synchronization (WiFi mode)
  - Timezone configuration support
- **Idle Timeout**:
  - Configurable timeout (default 30s)
  - Returns to idle screen when no data received
  - Continues to show date/time on idle screen
- **Alert System**: Visual alerts for:
  - High CPU temperature
  - Low memory
  - Low disk space
- **Graph History**: 60-point historical data for trends
- **Smooth Updates**: Partial screen updates to prevent flickering

### Configuration
- **CLI Interface**: Serial-based command-line configuration
- **Web Interface**: Browser-based configuration (WiFi mode only)
- **Persistent Storage**: Settings saved to flash memory
- **Extensible Commands**: Easy to add new CLI commands

## Project Structure

```
MonitorScreen/
├── MonitorScreen.ino          # Main Arduino sketch
├── User_Setup.h               # TFT_eSPI configuration (copy to library)
│
├── Config.h / Config.cpp      # Configuration management
├── CLI.h / CLI.cpp            # Command-line interface
├── CLICommands.h / CLICommands.cpp
├── SystemData.h               # System data structures
├── CommInterface.h / CommInterface.cpp
├── CommManager.h / CommManager.cpp
├── WiFiComm.h / WiFiComm.cpp  # WiFi communication
├── BLEComm.h / BLEComm.cpp    # BLE communication
├── Display.h / Display.cpp    # Display interface
├── WebServer.h / WebServer.cpp # Web server
│
├── pc_app/                    # Python PC application
│   ├── monitor_client.py
│   └── requirements.txt
│
└── Doc/
    ├── requirements.txt       # Project requirements
    ├── QUICK_START.md        # Quick start guide
    └── HARDWARE_CONFIG.md    # Hardware configuration
```

## Hardware Setup

### Pin Connections (ST7789)

**Configured pin mapping (as set in `User_Setup.h`):**

```
TFT_MISO = 12  // Master In Slave Out (not always needed)
TFT_MOSI = 13  // SDA - Serial Data
TFT_SCLK = 14  // SCL - Serial Clock
TFT_CS   = 15  // Chip Select
TFT_DC   = 2   // Data/Command
TFT_RST  = -1  // Reset (not used - tied to VCC or ESP32 reset)
TFT_BL   = 27  // Backlight control
```

**Physical Connections:**
```
ST7789          ESP32
------          -----
VCC    -------> 3.3V
GND    -------> GND
SCL    -------> GPIO 14
SDA    -------> GPIO 13
CS     -------> GPIO 15
DC     -------> GPIO 2
RST    -------> 3.3V or ESP32 EN
BL     -------> GPIO 27
```

### Required Components
- ESP32 development board
- ST7789 TFT display (240x320)
- Connecting wires
- Power supply (USB or external)

## Software Setup

### ESP32 Firmware (Arduino IDE)

1. **Install Arduino IDE** (version 1.8.x or 2.x)
   - Download from [arduino.cc](https://www.arduino.cc/en/software)

2. **Install ESP32 board support**
   - Open Arduino IDE
   - Go to File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search for "esp32" and install "esp32 by Espressif Systems"

3. **Install required libraries**
   - Go to Sketch → Include Library → Manage Libraries
   - Install the following libraries:
     - **TFT_eSPI** by Bodmer (version 2.5.x or later)
     - **ArduinoJson** by Benoit Blanchon (version 6.x)
     - **NimBLE-Arduino** by h2zero (version 1.4.x or later)

4. **Configure TFT_eSPI library**
   - Locate your Arduino libraries folder:
     - Windows: `Documents\Arduino\libraries\TFT_eSPI`
     - Linux/Mac: `~/Arduino/libraries/TFT_eSPI`
   - Copy the provided `User_Setup.h` file to the TFT_eSPI library folder
   - **OR** edit the existing `User_Setup.h` file with the pin definitions from the provided file

5. **Open and upload the sketch**
   - Open `MonitorScreen.ino` in Arduino IDE
   - Select your board: Tools → Board → ESP32 Arduino → "ESP32 Dev Module"
   - Select the correct COM port: Tools → Port
   - Click Upload button

### PC Application

1. Navigate to `pc_app` directory
2. Install Python dependencies:
   ```bash
   pip install -r requirements.txt
   ```

## Configuration

### First-Time Setup

1. Connect to ESP32 via Serial Monitor (115200 baud)
2. Configure WiFi credentials:
   ```
   setwifi <SSID> <password>
   ```
3. Set communication interface:
   ```
   setinterface wifi
   ```
4. Restart the device:
   ```
   reset
   ```

### CLI Commands

#### General Commands
| Command | Description | Example |
|---------|-------------|---------|
| `help` | Show all commands | `help` |
| `status` | Display system status | `status` |
| `reset` | Reset to defaults | `reset` |

#### Network Commands
| Command | Description | Example |
|---------|-------------|---------|
| `scanwifi` | Scan for WiFi networks | `scanwifi` |
| `selectwifi` | Select WiFi by index | `selectwifi 0 MyPassword` |
| `setwifi` | Set WiFi credentials | `setwifi "My Network" MyPassword` |
| `setinterface` | Set comm interface | `setinterface wifi` or `setinterface ble` |
| `setblename` | Set BLE device name | `setblename MyMonitor` |
| `setport` | Set server port | `setport 8080` |

#### Display Commands
| Command | Description | Example |
|---------|-------------|---------|
| `settheme` | Set display theme (0-3) | `settheme 2` |
| `setbrightness` | Set brightness (0-255) | `setbrightness 200` |
| `setalert` | Set alert threshold | `setalert cpu 85` |
| `setidletimeout` | Set idle timeout (seconds) | `setidletimeout 60` |

#### Date/Time Commands
| Command | Description | Example |
|---------|-------------|---------|
| `setdatetime` | Set date and time | `setdatetime 2025-10-08 14:30:00` |
| `getdatetime` | Get current date/time | `getdatetime` |
| `syncntp` | Sync time with NTP server | `syncntp` |
| `setntpserver` | Set NTP server | `setntpserver pool.ntp.org` |
| `settimezone` | Set timezone offset | `settimezone 28800` (UTC+8) |

**Notes:**
- For SSIDs with spaces, use quotes: `setwifi "My WiFi Network" password`
- All CLI output uses CRLF line endings for compatibility
- Timezone offset is in seconds (e.g., 3600 = UTC+1, -18000 = UTC-5)

### Web Configuration

When using WiFi mode:

1. Connect to the same network as ESP32
2. Find the ESP32 IP address (shown in Serial Monitor)
3. Open browser and navigate to: `http://<ESP32_IP>`
4. Use the web interface to:
   - View real-time system data
   - Change configuration settings
   - Restart the device

## Usage

### WiFi Mode

1. Configure ESP32 for WiFi (see Configuration)
2. Note the IP address shown in Serial Monitor
3. On your PC, run:
   ```bash
   python pc_app/monitor_client.py --mode wifi --host <ESP32_IP> --port 8080
   ```

### BLE Mode

1. Configure ESP32 for BLE:
   ```
   setinterface ble
   reset
   ```
2. On your PC, run:
   ```bash
   python pc_app/monitor_client.py --mode ble --device ESP32_Monitor
   ```

### PC Client Options

```bash
python monitor_client.py --help

Options:
  --mode {wifi,ble}      Communication mode (default: wifi)
  --host HOST            ESP32 IP address (WiFi mode)
  --port PORT            ESP32 UDP port (WiFi mode, default: 8080)
  --device DEVICE        BLE device name (BLE mode)
  --interval INTERVAL    Update interval in seconds (default: 1)
  --log                  Enable logging output (disabled by default)
  --quiet                Disable all logging output
```

**Examples:**

Run silently (no output):
```bash
python monitor_client.py --host 192.168.1.100 --port 8080
```

Run with logging enabled:
```bash
python monitor_client.py --host 192.168.1.100 --port 8080 --log
```

### Display Themes

- **Theme 0 (Default)**: Progress bars with detailed stats
- **Theme 1 (Minimal)**: Large percentage displays
- **Theme 2 (Graph)**: Real-time graphs with historical data
- **Theme 3 (Compact)**: Dense layout with mini graphs

Change theme via CLI:
```
settheme 2
```

Or via web interface.

## Alert System

Configure alert thresholds:

```bash
setalert cpu 80     # Alert if CPU temp > 80°C
setalert mem 20     # Alert if memory < 20% free
setalert disk 10    # Alert if disk < 10% free
```

Visual alerts will appear at the top of the display when thresholds are exceeded.

## Date/Time Features

### Manual Time Setting

Set date and time via CLI:
```bash
setdatetime 2025-10-08 14:30:00
```

Or via web interface (WiFi mode only).

### Automatic NTP Synchronization

Configure timezone (in seconds):
```bash
settimezone 28800        # UTC+8
settimezone -18000       # UTC-5 (EST)
settimezone -18000 3600  # UTC-5 with DST
```

Set NTP server (optional):
```bash
setntpserver pool.ntp.org
```

Sync time:
```bash
syncntp
```

### Time Display

- **Idle Screen**: Shows full date (YYYY-MM-DD) and time (HH:MM)
- **Monitor Screens**:
  - Default/Graph themes: Time (HH:MM) in top-right corner
  - Minimal/Compact themes: Full date-time centered at top
- Updates every second, even when no data is received

## Idle Timeout Feature

The display automatically returns to the idle screen when no data is received for a configured period.

### Configuration

Via CLI:
```bash
setidletimeout 60    # Set to 60 seconds
setidletimeout 0     # Disable idle timeout
```

Via web interface:
- Navigate to `/config`
- Set "Idle Timeout" field
- 0 = disabled, 1-65535 = seconds

### Behavior

1. **Data Received** → Display shows monitoring data
2. **No Data for 5s** → Warning message shown
3. **No Data for timeout period** (default 30s) → Returns to idle screen
4. **Data Received Again** → Automatically exits idle mode

The idle screen continues to show the current date/time.

## Data Format

The PC client sends JSON data in this format:

```json
{
  "cpu": {
    "usage": 45.2,
    "temp": 55.0,
    "name": "Intel Core i7-9700K"
  },
  "memory": {
    "used": 12.5,
    "total": 16.0,
    "percent": 78.1
  },
  "disk": {
    "used": 450.0,
    "total": 500.0,
    "percent": 90.0
  },
  "network": {
    "upload": 125.5,
    "download": 1024.3
  },
  "gpu": {
    "usage": 35.0,
    "temp": 60.0
  }
}
```

## Extending the Project

### Adding New CLI Commands

1. Add function declaration in `CLICommands.h`:
   ```cpp
   void cmdMyCommand(int argc, char* argv[]);
   ```

2. Implement in `CLICommands.cpp`:
   ```cpp
   void cmdMyCommand(int argc, char* argv[]) {
       CLI& cli = CLI::getInstance();
       // Your implementation
   }
   ```

3. Register in `registerCLICommands()`:
   ```cpp
   cli.registerCommand("mycommand", "Description", cmdMyCommand);
   ```

### Adding New Display Themes

1. Add theme enum in `Config.h`:
   ```cpp
   enum DisplayTheme {
       THEME_DEFAULT = 0,
       THEME_MINIMAL = 1,
       THEME_GRAPH = 2,
       THEME_COMPACT = 3,
       THEME_CUSTOM = 4  // Your new theme
   };
   ```

2. Implement render function in `Display.cpp`:
   ```cpp
   void Display::renderThemeCustom(const SystemData& data) {
       // Your rendering code
   }
   ```

3. Add case to `update()` switch statement

### Adding New Data Fields

1. Add fields to `SystemData` struct in `SystemData.h`
2. Update JSON parsing in `CommInterface.cpp`
3. Update PC client to send new data
4. Update display rendering to show new data

## Troubleshooting

### Display not working
- Check pin connections
- Verify TFT_eSPI configuration
- Test with TFT_eSPI example sketches

### WiFi connection fails
- Verify SSID and password
- Check router compatibility (2.4GHz required)
- Monitor Serial output for error messages

### No data received
- Verify PC client is running (use `--log` to see output)
- Check IP address and port match
- Verify firewall settings on PC
- Check network connectivity
- Display will return to idle screen after timeout period

### BLE connection issues
- Ensure BLE is enabled on PC
- Check BLE device name matches
- On Windows, may require additional BLE drivers
- Maximum distance ~10 meters

## Performance

- **Update Rate**: Configurable (default 1 second)
- **Display Refresh**: 500ms to prevent flickering
- **Time Update**: 1 second (independent of data updates)
- **Data Warning Timeout**: 5 seconds
- **Idle Timeout**: Configurable (default 30 seconds)
- **Memory Usage**: ~50KB RAM
- **WiFi Latency**: < 50ms typical
- **BLE Latency**: < 100ms typical

## License

This project is open source and available for modification and distribution.

## Credits

- TFT_eSPI library by Bodmer
- ArduinoJson library by Benoit Blanchon
- psutil library for Python
- ESP32 Arduino Core

## Support

For issues, questions, or contributions, please refer to the project repository.

## Recent Enhancements

- [x] Date/time display on all screens
- [x] NTP time synchronization
- [x] Timezone configuration
- [x] Configurable idle timeout
- [x] Silent mode for PC client
- [x] CRLF line endings in CLI

## Future Enhancements

- [ ] OTA (Over-The-Air) firmware updates
- [ ] Multiple PC monitoring support
- [ ] Custom alert actions
- [ ] Data logging to SD card
- [ ] Mobile app for configuration
- [ ] MQTT support for home automation
- [ ] Custom widgets and layouts
- [ ] Additional clock formats (12/24 hour)
