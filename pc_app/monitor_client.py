#!/usr/bin/env python3
"""
ESP32 System Monitor - PC Client

Collects system information from the PC and sends it to the ESP32 monitor
via WiFi (UDP) or BLE.

Requirements:
- psutil: pip install psutil
- For BLE: pip install bleak
- For GPU monitoring: pip install gputil
- For Windows temperature monitoring: pip install wmi (requires OpenHardwareMonitor or LibreHardwareMonitor running)
"""

import json
import socket
import time
import argparse
import platform
import psutil


class SystemMonitor:
    """Collects system information from the PC"""

    def __init__(self):
        self.cpu_name = self._get_cpu_name()

    def _get_cpu_name(self):
        """Get CPU name"""
        try:
            if platform.system() == "Windows":
                import subprocess
                result = subprocess.check_output(
                    "wmic cpu get name", shell=True
                ).decode()
                return result.split('\n')[1].strip()
            elif platform.system() == "Linux":
                with open('/proc/cpuinfo', 'r') as f:
                    for line in f:
                        if 'model name' in line:
                            return line.split(':')[1].strip()
            elif platform.system() == "Darwin":  # macOS
                import subprocess
                result = subprocess.check_output(
                    "sysctl -n machdep.cpu.brand_string", shell=True
                ).decode()
                return result.strip()
        except Exception:
            pass
        return "Unknown CPU"

    def get_system_data(self):
        """Collect all system data"""
        # CPU info
        cpu_usage = psutil.cpu_percent(interval=0.5)
        cpu_temp = self._get_cpu_temp()

        # Memory info
        memory = psutil.virtual_memory()
        memory_used = memory.used / (1024**3)  # GB
        memory_total = memory.total / (1024**3)  # GB
        memory_percent = memory.percent

        # Disk info
        disk = psutil.disk_usage('/')
        disk_used = disk.used / (1024**3)  # GB
        disk_total = disk.total / (1024**3)  # GB
        disk_percent = disk.percent

        # Network info
        net_io = psutil.net_io_counters()
        # Calculate speed (simple approach, stores previous values)
        if not hasattr(self, '_prev_net_io'):
            self._prev_net_io = net_io
            self._prev_time = time.time()

        time_delta = time.time() - self._prev_time
        if time_delta > 0:
            upload_speed = (net_io.bytes_sent - self._prev_net_io.bytes_sent) / time_delta / 1024  # KB/s
            download_speed = (net_io.bytes_recv - self._prev_net_io.bytes_recv) / time_delta / 1024  # KB/s
        else:
            upload_speed = 0
            download_speed = 0

        self._prev_net_io = net_io
        self._prev_time = time.time()

        # GPU info (optional, may not be available)
        gpu_usage = 0
        gpu_temp = 0
        try:
            import GPUtil
            gpus = GPUtil.getGPUs()
            if gpus:
                gpu_usage = gpus[0].load * 100
                gpu_temp = gpus[0].temperature
        except ImportError:
            pass

        # Get all temperature sensors
        all_temps = self._get_all_temperatures()

        data = {
            "cpu": {
                "usage": round(cpu_usage, 1),
                "temp": round(cpu_temp, 1),
                "name": self.cpu_name
            },
            "memory": {
                "used": round(memory_used, 1),
                "total": round(memory_total, 1),
                "percent": round(memory_percent, 1)
            },
            "disk": {
                "used": round(disk_used, 1),
                "total": round(disk_total, 1),
                "percent": round(disk_percent, 1)
            },
            "network": {
                "upload": round(upload_speed, 2),
                "download": round(download_speed, 2)
            },
            "gpu": {
                "usage": round(gpu_usage, 1),
                "temp": round(gpu_temp, 1)
            },
            "temperatures": {
                "cpu": round(all_temps['cpu'], 1),
                "gpu": round(all_temps['gpu'], 1),
                "motherboard": round(all_temps['motherboard'], 1),
                "disks": all_temps['disk']
            }
        }

        return data

    def _get_cpu_temp(self):
        """Get CPU temperature (platform dependent)"""
        try:
            if platform.system() == "Linux":
                temps = psutil.sensors_temperatures()
                if 'coretemp' in temps:
                    # Average all core temperatures
                    core_temps = [t.current for t in temps['coretemp'] if 'Core' in t.label or 'Package' in t.label]
                    if core_temps:
                        return sum(core_temps) / len(core_temps)
                    return temps['coretemp'][0].current
                elif 'cpu_thermal' in temps:
                    return temps['cpu_thermal'][0].current
                elif 'k10temp' in temps:  # AMD CPUs
                    return temps['k10temp'][0].current
            elif platform.system() == "Darwin":  # macOS
                # Try using smc tool for temperature
                try:
                    import subprocess
                    result = subprocess.check_output(
                        ["sudo", "powermetrics", "-n", "1", "-i", "1", "--samplers", "smc"],
                        stderr=subprocess.DEVNULL
                    ).decode()
                    for line in result.split('\n'):
                        if 'CPU die temperature' in line:
                            temp_str = line.split(':')[1].strip().replace('C', '')
                            return float(temp_str)
                except:
                    pass
            elif platform.system() == "Windows":
                # Try using OpenHardwareMonitor or LibreHardwareMonitor WMI
                try:
                    import wmi
                    w = wmi.WMI(namespace="root\\OpenHardwareMonitor")
                    temperature_infos = w.Sensor()
                    cpu_temps = []
                    for sensor in temperature_infos:
                        if sensor.SensorType == 'Temperature' and 'CPU' in sensor.Name:
                            cpu_temps.append(sensor.Value)
                    if cpu_temps:
                        return sum(cpu_temps) / len(cpu_temps)
                except:
                    pass

                # Try LibreHardwareMonitor
                try:
                    import wmi
                    w = wmi.WMI(namespace="root\\LibreHardwareMonitor")
                    temperature_infos = w.Sensor()
                    cpu_temps = []
                    for sensor in temperature_infos:
                        if sensor.SensorType == 'Temperature' and 'CPU' in sensor.Name:
                            cpu_temps.append(sensor.Value)
                    if cpu_temps:
                        return sum(cpu_temps) / len(cpu_temps)
                except:
                    pass
        except Exception:
            pass
        return 0.0

    def _get_all_temperatures(self):
        """Get all available temperature sensors"""
        temps = {
            'cpu': 0.0,
            'gpu': 0.0,
            'motherboard': 0.0,
            'disk': []
        }

        try:
            # Get CPU temp
            temps['cpu'] = self._get_cpu_temp()

            # Get GPU temp
            try:
                import GPUtil
                gpus = GPUtil.getGPUs()
                if gpus:
                    temps['gpu'] = gpus[0].temperature
            except:
                pass

            # Get all sensors (Linux)
            if platform.system() == "Linux":
                try:
                    all_temps = psutil.sensors_temperatures()

                    # Motherboard temperature
                    if 'acpitz' in all_temps:
                        temps['motherboard'] = all_temps['acpitz'][0].current

                    # Disk temperatures
                    for name, entries in all_temps.items():
                        if 'nvme' in name.lower() or 'sata' in name.lower() or 'hdd' in name.lower():
                            for entry in entries:
                                temps['disk'].append({
                                    'name': name,
                                    'temp': entry.current
                                })
                except:
                    pass
        except Exception:
            pass

        return temps


class WiFiSender:
    """Sends data via WiFi (UDP)"""

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        print(f"WiFi sender initialized: {host}:{port}")

    def send(self, data):
        """Send JSON data via UDP"""
        try:
            json_data = json.dumps(data)
            self.sock.sendto(json_data.encode(), (self.host, self.port))
            return True
        except Exception as e:
            print(f"Error sending data: {e}")
            return False

    def close(self):
        self.sock.close()


class BLESender:
    """Sends data via BLE"""

    def __init__(self, device_name):
        self.device_name = device_name
        self.client = None
        self.characteristic_uuid = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
        print(f"BLE sender initialized for device: {device_name}")
        print("Note: BLE support requires 'bleak' library (pip install bleak)")

    async def connect(self):
        """Connect to BLE device"""
        from bleak import BleakScanner, BleakClient

        print("Scanning for BLE devices...")
        devices = await BleakScanner.discover()

        device_address = None
        for device in devices:
            if self.device_name in (device.name or ""):
                device_address = device.address
                print(f"Found device: {device.name} ({device.address})")
                break

        if not device_address:
            print(f"Device '{self.device_name}' not found!")
            return False

        self.client = BleakClient(device_address)
        await self.client.connect()
        print("Connected to BLE device")
        return True

    async def send(self, data):
        """Send JSON data via BLE"""
        try:
            if not self.client or not self.client.is_connected:
                print("Not connected to BLE device")
                return False

            json_data = json.dumps(data)
            await self.client.write_gatt_char(
                self.characteristic_uuid,
                json_data.encode()
            )
            return True
        except Exception as e:
            print(f"Error sending data: {e}")
            return False

    async def close(self):
        if self.client and self.client.is_connected:
            await self.client.disconnect()


async def run_ble_mode(device_name, interval):
    """Run in BLE mode"""
    try:
        from bleak import BleakClient
    except ImportError:
        print("Error: bleak library not installed. Install with: pip install bleak")
        return

    monitor = SystemMonitor()
    sender = BLESender(device_name)

    if not await sender.connect():
        return

    print(f"\nSending system data via BLE every {interval} seconds...")
    print("Press Ctrl+C to stop\n")

    try:
        while True:
            data = monitor.get_system_data()
            if await sender.send(data):
                print(f"Sent: CPU={data['cpu']['usage']}%, "
                      f"MEM={data['memory']['percent']}%, "
                      f"DISK={data['disk']['percent']}%")
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        await sender.close()


def run_wifi_mode(host, port, interval):
    """Run in WiFi mode"""
    monitor = SystemMonitor()
    sender = WiFiSender(host, port)

    print(f"\nSending system data via WiFi every {interval} seconds...")
    print("Press Ctrl+C to stop\n")

    try:
        while True:
            data = monitor.get_system_data()
            if sender.send(data):
                print(f"Sent: CPU={data['cpu']['usage']}%, "
                      f"MEM={data['memory']['percent']}%, "
                      f"DISK={data['disk']['percent']}%")
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        sender.close()


def main():
    parser = argparse.ArgumentParser(description='ESP32 System Monitor - PC Client')
    parser.add_argument('--mode', choices=['wifi', 'ble'], default='wifi',
                        help='Communication mode (default: wifi)')
    parser.add_argument('--host', default='192.168.1.100',
                        help='ESP32 IP address (WiFi mode, default: 192.168.1.100)')
    parser.add_argument('--port', type=int, default=8080,
                        help='ESP32 UDP port (WiFi mode, default: 8080)')
    parser.add_argument('--device', default='ESP32_Monitor',
                        help='BLE device name (BLE mode, default: ESP32_Monitor)')
    parser.add_argument('--interval', type=int, default=1,
                        help='Update interval in seconds (default: 1)')

    args = parser.parse_args()

    print("=" * 50)
    print("ESP32 System Monitor - PC Client")
    print("=" * 50)
    print(f"Mode: {args.mode.upper()}")
    print(f"Interval: {args.interval} seconds")

    if args.mode == 'wifi':
        print(f"Target: {args.host}:{args.port}")
        run_wifi_mode(args.host, args.port, args.interval)
    else:
        print(f"Device: {args.device}")
        import asyncio
        asyncio.run(run_ble_mode(args.device, args.interval))


if __name__ == '__main__':
    main()
