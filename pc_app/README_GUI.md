# ESP32 System Monitor - GUI Application

A comprehensive PyQt5-based GUI application for monitoring PC system information and configuring your ESP32 monitor device.

![ESP32 Monitor GUI](https://via.placeholder.com/800x600?text=ESP32+Monitor+GUI)

## Features

### Monitoring Tab
- **Start/Stop Controls**: Easy one-click monitoring control
- **Live Data Display**: Real-time system metrics with progress bars
  - CPU usage and temperature
  - Memory usage
  - Disk usage
  - Network upload/download speed
- **Connection Settings**: Configure WiFi/BLE mode, IP address, port, and update interval
- **Activity Log**: View monitoring events and status messages

### Device Configuration Tab
- **Serial CLI Integration**: Direct access to all ESP32 CLI commands
- **WiFi Configuration**:
  - Scan for available networks
  - Set WiFi credentials with SSID and password
- **Display Settings**:
  - Change display theme (Default, Minimal, Graph, Compact)
  - Adjust brightness (0-255)
  - Configure idle timeout
- **Date/Time Management**:
  - Set date/time manually
  - Configure NTP server and timezone
  - One-click NTP synchronization
- **Advanced Settings**:
  - Configure alert thresholds (CPU temp, memory, disk)
  - Device reset option

### Status Tab
- **Device Status**: View complete ESP32 system status
- **Real-time Information**: Check WiFi connection, display settings, alerts, and more

## Installation

### Prerequisites

- Python 3.7 or later
- ESP32 System Monitor hardware (see main README.md)

### Install Dependencies

```bash
cd pc_app
pip install -r requirements_gui.txt
```

**For Windows temperature monitoring:**
```bash
pip install wmi
```

**For BLE support (future):**
```bash
pip install bleak
```

## Usage

### Starting the GUI

```bash
python monitor_gui.py
```

Or on Windows, you can double-click `monitor_gui.py` if Python is associated with .py files.

### Quick Start Guide

#### 1. Start Monitoring

1. Open the **Monitor** tab
2. Enter your ESP32's IP address (e.g., `192.168.1.100`)
3. Verify the port matches your ESP32 configuration (default: `8080`)
4. Click **Start Monitoring**
5. Watch live system data appear in real-time!

#### 2. Configure Your Device

1. Connect ESP32 to your PC via USB
2. Open the **Device Config** tab
3. Select the correct serial port from the dropdown
4. Click **Connect**
5. Use the sub-tabs to configure:
   - **WiFi**: Set network credentials
   - **Display**: Change theme and brightness
   - **Date/Time**: Set time or sync with NTP
   - **Advanced**: Configure alerts and advanced settings

#### 3. Check Device Status

1. Open the **Status** tab
2. Click **Get Device Status**
3. View complete device information

## Detailed Features

### Monitor Tab

**Connection Modes:**
- **WiFi**: Recommended for desktop monitoring (default)
- **BLE**: For Bluetooth Low Energy connection (coming soon)

**Live Metrics:**
- CPU usage percentage with visual progress bar
- CPU temperature in Celsius
- Memory usage percentage and progress bar
- Disk usage percentage and progress bar
- Network upload/download speeds in KB/s

**Update Interval:**
- Configurable from 1 to 60 seconds
- Faster updates = more responsive display
- Slower updates = less network/CPU usage

### Device Configuration

**WiFi Setup:**
1. Click "Scan WiFi" to see available networks
2. Enter SSID and password
3. Click "Set WiFi"
4. Device will restart automatically

**Display Customization:**
- **Theme 0 (Default)**: Progress bars with detailed information
- **Theme 1 (Minimal)**: Large numbers, clean layout
- **Theme 2 (Graph)**: Historical data visualization
- **Theme 3 (Compact)**: Dense information with mini graphs

**Date/Time Options:**
- **Manual**: Set specific date and time
- **NTP**: Automatically sync with internet time servers
  - Supports timezone offset configuration
  - Configurable NTP server (default: pool.ntp.org)

**Alert Thresholds:**
- CPU Temperature High (°C)
- Memory Low (%)
- Disk Low (%)

### Serial Communication

The GUI uses serial communication to send CLI commands to your ESP32 device. This allows you to:
- Configure WiFi without using the Serial Monitor
- Change display settings instantly
- Set date/time and sync with NTP
- View device status in a friendly format
- Reset device to factory defaults

**Serial Connection Tips:**
- Ensure no other programs (like Arduino Serial Monitor) are using the port
- If connection fails, try clicking "Refresh" to update the port list
- On some systems, you may need administrator privileges

## Keyboard Shortcuts

- `Ctrl+Tab` / `Ctrl+Shift+Tab`: Switch between tabs
- `F5`: Refresh serial ports (when in Device Config tab)

## Troubleshooting

### GUI Won't Start

**Error: "No module named PyQt5"**
```bash
pip install PyQt5
```

**Error: "No module named serial"**
```bash
pip install pyserial
```

### Serial Connection Issues

**Port not listed:**
- Install USB-to-Serial drivers for your ESP32 board
- Check that ESP32 is connected via USB
- Click "Refresh" to update port list

**Connection failed:**
- Close Arduino IDE Serial Monitor if open
- Try a different USB port
- Restart the ESP32 device

**No response to commands:**
- Verify ESP32 is running the correct firmware
- Check baudrate is 115200 (default)
- Try disconnecting and reconnecting

### Monitoring Issues

**"Failed to start monitoring":**
- Verify ESP32 IP address is correct
- Check that ESP32 is connected to WiFi
- Ensure port 8080 (or configured port) is accessible
- Check firewall settings on PC

**No data showing:**
- Verify ESP32 is receiving data (check its display)
- Ensure WiFi mode is selected if using WiFi
- Check network connectivity
- Try using the command-line client first to verify connection

**Data stops updating:**
- Check ESP32 is still powered on
- Verify network connection is stable
- Check the Activity Log for error messages
- Try stopping and restarting monitoring

### Display/Configuration Issues

**Commands not working:**
- Ensure serial connection is established (button shows "Disconnect")
- Check Response area for error messages
- Verify ESP32 is responding (try "Get Device Status")

**WiFi setup fails:**
- Use quotes for SSIDs with spaces (automatically handled by GUI)
- Verify password is correct
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Check Response area for specific error messages

## Advanced Usage

### Running as Background Process

You can minimize the GUI and it will continue monitoring in the background.

### Multiple Instances

You can run multiple instances to monitor different ESP32 devices:
1. Start first instance, configure for device #1
2. Start second instance, configure for device #2
3. Each instance operates independently

### Automation with Scripts

While the GUI is running, you can still use the command-line `monitor_client.py` for automation or scripting purposes.

## Comparison: GUI vs Command-Line

| Feature | GUI | Command-Line |
|---------|-----|--------------|
| Ease of Use | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| Visual Feedback | ⭐⭐⭐⭐⭐ | ⭐ |
| Device Config | ⭐⭐⭐⭐⭐ | ⭐⭐ (via Serial Monitor) |
| Automation | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| Resource Usage | Higher | Lower |
| Silent Running | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

## Known Limitations

- BLE mode not yet implemented in GUI (use command-line client)
- Serial connection requires exclusive access (close other serial programs)
- Windows may require administrator privileges for serial access
- Live data updates limited to WiFi mode

## Future Enhancements

- [ ] BLE mode support
- [ ] Multiple device monitoring in single window
- [ ] Data logging and export (CSV, JSON)
- [ ] Historical data graphs
- [ ] System tray integration
- [ ] Auto-reconnect on connection loss
- [ ] Device discovery (auto-find ESP32 on network)
- [ ] Custom alert notifications
- [ ] Dark/light theme toggle

## Tips and Best Practices

1. **Serial Configuration**: Use the GUI's serial connection feature to configure your ESP32 once, then disconnect and use WiFi monitoring for normal operation.

2. **Network Stability**: For best results, use a wired Ethernet connection on your PC or ensure strong WiFi signal.

3. **Update Interval**: Start with 1-second intervals for testing, increase to 2-5 seconds for normal use to reduce network traffic.

4. **Firewall**: If monitoring fails, temporarily disable firewall or add exception for Python/this application.

5. **First-Time Setup**: Connect via serial first to configure WiFi, then switch to wireless monitoring.

## Contributing

Feel free to submit issues, feature requests, or pull requests to improve the GUI application.

## License

This GUI application is part of the ESP32 System Monitor project and shares the same open-source license.

## Support

For issues or questions:
1. Check this README
2. Check the main project README.md
3. Review the command-line client documentation
4. Check the Activity Log in the GUI for error messages
5. Submit an issue to the project repository

---

**Enjoy monitoring your system with a beautiful GUI! 🖥️📊**
