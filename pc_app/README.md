# ESP32 System Monitor - PC Client

Python application that collects system information from your PC and sends it to the ESP32 monitor display.

## Features

- **System Monitoring**:
  - CPU usage and temperature (multi-core average)
  - Memory usage (used/total/percentage)
  - Disk usage (used/total/percentage)
  - Network speed (upload/download in KB/s)
  - GPU usage and temperature (if available)
  - Additional temperature sensors (motherboard, disk drives)

- **Communication**:
  - WiFi mode (UDP packets)
  - BLE mode (Bluetooth Low Energy)

## Installation

### Basic Requirements

```bash
pip install psutil
```

### Optional Requirements

For GPU monitoring:
```bash
pip install gputil
```

For BLE support:
```bash
pip install bleak
```

For Windows temperature monitoring:
```bash
pip install wmi
```

**Note**: Windows temperature monitoring requires [OpenHardwareMonitor](https://openhardwaremonitor.org/) or [LibreHardwareMonitor](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor) to be running in the background.

## Usage

### WiFi Mode (Default)

```bash
python monitor_client.py --mode wifi --host 192.168.1.100 --port 8080
```

### BLE Mode

```bash
python monitor_client.py --mode ble --device ESP32_Monitor
```

### Command Line Arguments

- `--mode`: Communication mode (`wifi` or `ble`, default: `wifi`)
- `--host`: ESP32 IP address for WiFi mode (default: `192.168.1.100`)
- `--port`: UDP port for WiFi mode (default: `8080`)
- `--device`: BLE device name for BLE mode (default: `ESP32_Monitor`)
- `--interval`: Update interval in seconds (default: `1`)

### Examples

Send data every 2 seconds via WiFi:
```bash
python monitor_client.py --interval 2 --host 192.168.1.24 --port 8080
```

Connect via BLE with custom device name:
```bash
python monitor_client.py --mode ble --device MyESP32 --interval 1
```

## Temperature Monitoring

The application now collects comprehensive temperature data:

### CPU Temperature
- **Linux**: Reads from `coretemp` (Intel), `k10temp` (AMD), or `cpu_thermal` (ARM)
- **macOS**: Uses `powermetrics` command (requires sudo)
- **Windows**: Requires OpenHardwareMonitor or LibreHardwareMonitor + WMI library

### GPU Temperature
- Requires `gputil` library
- Automatically detected if NVIDIA GPU is present

### Additional Sensors
- **Motherboard**: System board temperature (Linux only)
- **Disk Drives**: NVMe/SATA drive temperatures (Linux only)

## Data Format

The application sends JSON data with the following structure:

```json
{
  "cpu": {
    "usage": 45.2,
    "temp": 55.3,
    "name": "Intel Core i7-9700K"
  },
  "memory": {
    "used": 8.5,
    "total": 16.0,
    "percent": 53.1
  },
  "disk": {
    "used": 250.3,
    "total": 500.0,
    "percent": 50.1
  },
  "network": {
    "upload": 12.34,
    "download": 56.78
  },
  "gpu": {
    "usage": 30.5,
    "temp": 60.2
  },
  "temperatures": {
    "cpu": 55.3,
    "gpu": 60.2,
    "motherboard": 45.0,
    "disks": [
      {"name": "nvme0", "temp": 42.0}
    ]
  }
}
```

## Troubleshooting

### No CPU Temperature (Windows)
1. Install WMI library: `pip install wmi`
2. Download and run [OpenHardwareMonitor](https://openhardwaremonitor.org/)
3. Run OpenHardwareMonitor as Administrator
4. Restart the monitor_client.py script

### No GPU Data
1. Install gputil: `pip install gputil`
2. Ensure NVIDIA GPU drivers are installed
3. Only NVIDIA GPUs are supported via GPUtil

### BLE Connection Failed
1. Ensure Bluetooth is enabled on your PC
2. Install bleak: `pip install bleak`
3. Make sure ESP32 is in BLE mode (set via CLI)
4. Check that the device name matches (use `--device` parameter)

### Network Speed Shows Zero
- Network speed is calculated after the first interval
- Wait for at least 2 update cycles to see accurate speeds

## Platform Support

| Feature | Linux | macOS | Windows |
|---------|-------|-------|---------|
| CPU Usage | ✓ | ✓ | ✓ |
| CPU Temp | ✓ | ✓* | ✓** |
| Memory | ✓ | ✓ | ✓ |
| Disk | ✓ | ✓ | ✓ |
| Network | ✓ | ✓ | ✓ |
| GPU | ✓ | ✓ | ✓ |
| Motherboard Temp | ✓ | ✗ | ✗ |
| Disk Temp | ✓ | ✗ | ✗ |

\* macOS requires sudo for temperature access
\*\* Windows requires OpenHardwareMonitor/LibreHardwareMonitor
