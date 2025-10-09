#!/usr/bin/env python3
"""
ESP32 System Monitor - GUI Application

PyQt-based GUI for monitoring PC system information and configuring ESP32 device.

Features:
- Start/Stop system monitoring
- Configure ESP32 device via serial CLI
- Real-time status display
- WiFi and BLE support
"""

import sys
import json
import threading
import time
from datetime import datetime
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QLabel, QPushButton, QLineEdit,
                             QComboBox, QSpinBox, QTextEdit, QTabWidget,
                             QGroupBox, QGridLayout, QMessageBox, QProgressBar,
                             QCheckBox, QDateTimeEdit)
from PyQt5.QtCore import QTimer, pyqtSignal, QObject, Qt, QThread, QDateTime
from PyQt5.QtGui import QFont, QColor

# Import from the existing monitor_client
import sys
import os
sys.path.append(os.path.dirname(__file__))
from monitor_client import SystemMonitor, WiFiSender

try:
    import serial
    import serial.tools.list_ports
    SERIAL_AVAILABLE = True
except ImportError:
    SERIAL_AVAILABLE = False

# Try to import mDNS discovery
try:
    from mdns_discovery import discover_devices as mdns_discover_devices, resolve_hostname
    MDNS_AVAILABLE = True
except ImportError:
    MDNS_AVAILABLE = False


class MonitorWorker(QThread):
    """Worker thread for system monitoring"""
    data_updated = pyqtSignal(dict)
    status_updated = pyqtSignal(str)
    error_occurred = pyqtSignal(str)

    def __init__(self, mode='wifi', host='192.168.1.100', port=8080, interval=1):
        super().__init__()
        self.mode = mode
        self.host = host
        self.port = port
        self.interval = interval
        self.running = False
        self.monitor = SystemMonitor()
        self.sender = None

    def run(self):
        """Main monitoring loop"""
        try:
            # Initialize sender
            if self.mode == 'wifi':
                self.sender = WiFiSender(self.host, self.port)
                self.status_updated.emit(f"Connected to {self.host}:{self.port}")
            else:
                self.error_occurred.emit("BLE mode not yet implemented in GUI")
                return

            self.running = True

            while self.running:
                try:
                    # Get system data
                    data = self.monitor.get_system_data()

                    # Send to ESP32
                    if self.sender.send(data):
                        self.data_updated.emit(data)

                    # Wait for next update
                    time.sleep(self.interval)

                except Exception as e:
                    self.error_occurred.emit(f"Error: {str(e)}")
                    time.sleep(self.interval)

        except Exception as e:
            self.error_occurred.emit(f"Failed to start monitoring: {str(e)}")
        finally:
            if self.sender:
                self.sender.close()

    def stop(self):
        """Stop monitoring"""
        self.running = False
        self.wait()


class SerialCLI(QObject):
    """Serial communication with ESP32 CLI"""
    response_received = pyqtSignal(str)
    error_occurred = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.serial = None
        self.port = None

    def connect(self, port, baudrate=115200):
        """Connect to serial port"""
        try:
            if self.serial and self.serial.is_open:
                self.serial.close()

            self.serial = serial.Serial(port, baudrate, timeout=1)
            self.port = port
            time.sleep(2)  # Wait for ESP32 to reset
            return True
        except Exception as e:
            self.error_occurred.emit(f"Serial connection failed: {str(e)}")
            return False

    def send_command(self, command, timeout_seconds=20):
        """Send CLI command and get response"""
        if not self.serial or not self.serial.is_open:
            self.error_occurred.emit("Not connected to serial port")
            return None

        try:
            # Clear input buffer
            self.serial.reset_input_buffer()

            # Send command
            print(f"[Serial] Sending command: {command}")
            self.serial.write(f"{command}\n".encode())
            time.sleep(0.2)

            # Read response
            response = ""
            timeout = time.time() + timeout_seconds
            last_data_time = time.time()
            line_count = 0
            got_substantial_data = False

            print(f"[Serial] Reading response (timeout: {timeout_seconds}s)...")

            while time.time() < timeout:
                if self.serial.in_waiting:
                    line = self.serial.readline().decode('utf-8', errors='ignore').strip()
                    line_count += 1
                    print(f"[Serial] Line {line_count}: '{line}'")

                    if line and not line.startswith('>'):
                        response += line + "\n"
                        last_data_time = time.time()

                        # Consider data substantial after we get more than just status messages
                        # (scanwifi sends "Scanning..." then pauses, then sends results)
                        if line_count > 5 or "Found" in line or "SSID" in line or ")" in line:
                            got_substantial_data = True
                else:
                    # Only apply idle timeout if we got substantial data (actual results)
                    # For commands like scanwifi, initial messages come quick, then long pause, then results
                    if got_substantial_data and (time.time() - last_data_time) > 2.0:
                        print(f"[Serial] Idle timeout reached (2.0s with no data after getting results)")
                        break

                    # If waiting too long with no substantial data, just use what we have
                    if not got_substantial_data and line_count > 0 and (time.time() - last_data_time) > 12.0:
                        print(f"[Serial] Long idle without results - command may not return data")
                        break

                    time.sleep(0.05)

            print(f"[Serial] Finished reading. Total lines: {line_count}, Response length: {len(response)}")
            return response.strip()

        except Exception as e:
            print(f"[Serial] Exception: {str(e)}")
            self.error_occurred.emit(f"Command failed: {str(e)}")
            return None

    def disconnect(self):
        """Disconnect from serial port"""
        if self.serial and self.serial.is_open:
            self.serial.close()

    @staticmethod
    def list_ports():
        """List available serial ports"""
        if not SERIAL_AVAILABLE:
            return []
        return [port.device for port in serial.tools.list_ports.comports()]


class DiscoveryWorker(QThread):
    """Worker thread for device discovery"""
    finished = pyqtSignal(list)
    error = pyqtSignal(str)

    def __init__(self, timeout=5.0):
        super().__init__()
        self.timeout = timeout

    def run(self):
        """Run discovery"""
        print("Discovery thread started")
        try:
            print("Calling mdns_discover_devices...")
            devices = mdns_discover_devices(timeout=self.timeout)
            print(f"mdns_discover_devices returned: {devices}")
            self.finished.emit(devices)
        except Exception as e:
            import traceback
            error_msg = str(e)
            print(f"Discovery exception: {error_msg}")
            print(traceback.format_exc())
            self.error.emit(error_msg)


class CommandWorker(QThread):
    """Worker thread for serial commands"""
    finished = pyqtSignal(str, str)  # command, response
    error = pyqtSignal(str)

    def __init__(self, cli, command):
        super().__init__()
        self.cli = cli
        self.command = command

    def run(self):
        """Run command"""
        print(f"Command thread started: {self.command}")
        try:
            response = self.cli.send_command(self.command)
            print(f"Command response received: {len(response) if response else 0} chars")
            self.finished.emit(self.command, response)
        except Exception as e:
            import traceback
            error_msg = str(e)
            print(f"Command exception: {error_msg}")
            print(traceback.format_exc())
            self.error.emit(error_msg)


class MonitorGUI(QMainWindow):
    """Main GUI application"""

    def __init__(self):
        super().__init__()
        self.worker = None
        self.cli = SerialCLI() if SERIAL_AVAILABLE else None
        self.discovery_worker = None
        self.init_ui()

    def init_ui(self):
        """Initialize user interface"""
        self.setWindowTitle("ESP32 System Monitor - GUI Control")
        self.setGeometry(100, 100, 800, 600)

        # Create central widget and layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)

        # Create tab widget
        tabs = QTabWidget()
        layout.addWidget(tabs)

        # Add tabs
        tabs.addTab(self.create_monitor_tab(), "Monitor")
        tabs.addTab(self.create_config_tab(), "Device Config")
        tabs.addTab(self.create_status_tab(), "Status")

        # Status bar
        self.statusBar().showMessage("Ready")

    def create_monitor_tab(self):
        """Create monitoring control tab"""
        tab = QWidget()
        layout = QVBoxLayout(tab)

        # Connection settings group
        conn_group = QGroupBox("Connection Settings")
        conn_layout = QGridLayout()
        conn_group.setLayout(conn_layout)

        # Mode selection
        conn_layout.addWidget(QLabel("Mode:"), 0, 0)
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(["WiFi", "BLE"])
        conn_layout.addWidget(self.mode_combo, 0, 1)

        # Device discovery (mDNS)
        if MDNS_AVAILABLE:
            conn_layout.addWidget(QLabel("Discover:"), 1, 0)
            discover_layout = QHBoxLayout()
            self.device_combo = QComboBox()
            self.device_combo.addItem("No devices discovered")
            self.device_combo.currentIndexChanged.connect(self.on_device_selected)
            discover_layout.addWidget(self.device_combo)

            self.discover_btn = QPushButton("Scan Network")
            self.discover_btn.clicked.connect(self.discover_devices)
            discover_layout.addWidget(self.discover_btn)

            discover_widget = QWidget()
            discover_widget.setLayout(discover_layout)
            conn_layout.addWidget(discover_widget, 1, 1)

        # Host/IP
        conn_layout.addWidget(QLabel("ESP32 IP:"), 2, 0)
        self.host_edit = QLineEdit("192.168.1.100")
        conn_layout.addWidget(self.host_edit, 2, 1)

        # Port
        conn_layout.addWidget(QLabel("Port:"), 3, 0)
        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65535)
        self.port_spin.setValue(8080)
        conn_layout.addWidget(self.port_spin, 3, 1)

        # Update interval
        conn_layout.addWidget(QLabel("Interval (s):"), 4, 0)
        self.interval_spin = QSpinBox()
        self.interval_spin.setRange(1, 60)
        self.interval_spin.setValue(1)
        conn_layout.addWidget(self.interval_spin, 4, 1)

        layout.addWidget(conn_group)

        # Control buttons
        button_layout = QHBoxLayout()
        self.start_btn = QPushButton("Start Monitoring")
        self.start_btn.clicked.connect(self.start_monitoring)
        self.stop_btn = QPushButton("Stop Monitoring")
        self.stop_btn.clicked.connect(self.stop_monitoring)
        self.stop_btn.setEnabled(False)

        button_layout.addWidget(self.start_btn)
        button_layout.addWidget(self.stop_btn)
        layout.addLayout(button_layout)

        # Live data display
        data_group = QGroupBox("Live System Data")
        data_layout = QGridLayout()
        data_group.setLayout(data_layout)

        # CPU
        data_layout.addWidget(QLabel("CPU Usage:"), 0, 0)
        self.cpu_label = QLabel("-- %")
        self.cpu_progress = QProgressBar()
        data_layout.addWidget(self.cpu_label, 0, 1)
        data_layout.addWidget(self.cpu_progress, 0, 2)

        # CPU Temp
        data_layout.addWidget(QLabel("CPU Temp:"), 1, 0)
        self.cpu_temp_label = QLabel("-- °C")
        data_layout.addWidget(self.cpu_temp_label, 1, 1)

        # Memory
        data_layout.addWidget(QLabel("Memory:"), 2, 0)
        self.mem_label = QLabel("-- %")
        self.mem_progress = QProgressBar()
        data_layout.addWidget(self.mem_label, 2, 1)
        data_layout.addWidget(self.mem_progress, 2, 2)

        # Disk
        data_layout.addWidget(QLabel("Disk:"), 3, 0)
        self.disk_label = QLabel("-- %")
        self.disk_progress = QProgressBar()
        data_layout.addWidget(self.disk_label, 3, 1)
        data_layout.addWidget(self.disk_progress, 3, 2)

        # Network
        data_layout.addWidget(QLabel("Network:"), 4, 0)
        self.net_label = QLabel("↑ -- KB/s  ↓ -- KB/s")
        data_layout.addWidget(self.net_label, 4, 1, 1, 2)

        layout.addWidget(data_group)

        # Log area
        layout.addWidget(QLabel("Log:"))
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setMaximumHeight(150)
        layout.addWidget(self.log_text)

        layout.addStretch()
        return tab

    def create_config_tab(self):
        """Create device configuration tab"""
        tab = QWidget()
        layout = QVBoxLayout(tab)

        if not SERIAL_AVAILABLE:
            layout.addWidget(QLabel("Serial communication not available.\nInstall pyserial: pip install pyserial"))
            return tab

        # Serial connection group
        serial_group = QGroupBox("Serial Connection")
        serial_layout = QHBoxLayout()
        serial_group.setLayout(serial_layout)

        serial_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.refresh_ports()
        serial_layout.addWidget(self.port_combo)

        refresh_btn = QPushButton("Refresh")
        refresh_btn.clicked.connect(self.refresh_ports)
        serial_layout.addWidget(refresh_btn)

        self.connect_btn = QPushButton("Connect")
        self.connect_btn.clicked.connect(self.toggle_serial_connection)
        serial_layout.addWidget(self.connect_btn)

        layout.addWidget(serial_group)

        # Configuration tabs
        config_tabs = QTabWidget()
        config_tabs.addTab(self.create_wifi_config(), "WiFi")
        config_tabs.addTab(self.create_display_config(), "Display")
        config_tabs.addTab(self.create_datetime_config(), "Date/Time")
        config_tabs.addTab(self.create_advanced_config(), "Advanced")

        layout.addWidget(config_tabs)

        # Command response area
        layout.addWidget(QLabel("Response:"))
        self.response_text = QTextEdit()
        self.response_text.setReadOnly(True)
        self.response_text.setMaximumHeight(100)
        layout.addWidget(self.response_text)

        return tab

    def create_wifi_config(self):
        """Create WiFi configuration panel"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # WiFi credentials
        form_layout = QGridLayout()

        form_layout.addWidget(QLabel("SSID:"), 0, 0)
        self.ssid_edit = QLineEdit()
        form_layout.addWidget(self.ssid_edit, 0, 1)

        form_layout.addWidget(QLabel("Password:"), 1, 0)
        self.wifi_pass_edit = QLineEdit()
        self.wifi_pass_edit.setEchoMode(QLineEdit.Password)
        form_layout.addWidget(self.wifi_pass_edit, 1, 1)

        layout.addLayout(form_layout)

        # Buttons
        btn_layout = QHBoxLayout()

        scan_btn = QPushButton("Scan WiFi")
        scan_btn.clicked.connect(self.scan_wifi)
        btn_layout.addWidget(scan_btn)

        set_wifi_btn = QPushButton("Set WiFi")
        set_wifi_btn.clicked.connect(self.set_wifi)
        btn_layout.addWidget(set_wifi_btn)

        layout.addLayout(btn_layout)
        layout.addStretch()

        return widget

    def create_display_config(self):
        """Create display configuration panel"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        form_layout = QGridLayout()

        # Theme
        form_layout.addWidget(QLabel("Theme:"), 0, 0)
        self.theme_combo = QComboBox()
        self.theme_combo.addItems(["Default", "Minimal", "Graph", "Compact"])
        form_layout.addWidget(self.theme_combo, 0, 1)

        # Brightness
        form_layout.addWidget(QLabel("Brightness:"), 1, 0)
        self.brightness_spin = QSpinBox()
        self.brightness_spin.setRange(0, 255)
        self.brightness_spin.setValue(128)
        form_layout.addWidget(self.brightness_spin, 1, 1)

        # Idle timeout
        form_layout.addWidget(QLabel("Idle Timeout (s):"), 2, 0)
        self.idle_timeout_spin = QSpinBox()
        self.idle_timeout_spin.setRange(0, 65535)
        self.idle_timeout_spin.setValue(30)
        self.idle_timeout_spin.setSpecialValueText("Disabled")
        form_layout.addWidget(self.idle_timeout_spin, 2, 1)

        layout.addLayout(form_layout)

        # Apply button
        apply_btn = QPushButton("Apply Display Settings")
        apply_btn.clicked.connect(self.apply_display_settings)
        layout.addWidget(apply_btn)

        layout.addStretch()
        return widget

    def create_datetime_config(self):
        """Create date/time configuration panel"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # Manual date/time
        manual_group = QGroupBox("Manual Date/Time")
        manual_layout = QVBoxLayout()
        manual_group.setLayout(manual_layout)

        self.datetime_edit = QDateTimeEdit()
        self.datetime_edit.setCalendarPopup(True)
        self.datetime_edit.setDateTime(QDateTime.currentDateTime())
        self.datetime_edit.setDisplayFormat("yyyy-MM-dd HH:mm:ss")
        manual_layout.addWidget(self.datetime_edit)

        set_datetime_btn = QPushButton("Set Date/Time")
        set_datetime_btn.clicked.connect(self.set_datetime)
        manual_layout.addWidget(set_datetime_btn)

        layout.addWidget(manual_group)

        # NTP configuration
        ntp_group = QGroupBox("NTP Synchronization")
        ntp_layout = QGridLayout()
        ntp_group.setLayout(ntp_layout)

        ntp_layout.addWidget(QLabel("NTP Server:"), 0, 0)
        self.ntp_server_edit = QLineEdit("pool.ntp.org")
        ntp_layout.addWidget(self.ntp_server_edit, 0, 1)

        ntp_layout.addWidget(QLabel("Timezone (s):"), 1, 0)
        self.timezone_spin = QSpinBox()
        self.timezone_spin.setRange(-43200, 43200)
        self.timezone_spin.setValue(0)
        self.timezone_spin.setSingleStep(3600)
        ntp_layout.addWidget(self.timezone_spin, 1, 1)

        sync_btn = QPushButton("Sync with NTP")
        sync_btn.clicked.connect(self.sync_ntp)
        ntp_layout.addWidget(sync_btn, 2, 0, 1, 2)

        layout.addWidget(ntp_group)

        layout.addStretch()
        return widget

    def create_advanced_config(self):
        """Create advanced configuration panel"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # Alerts
        alert_group = QGroupBox("Alert Thresholds")
        alert_layout = QGridLayout()
        alert_group.setLayout(alert_layout)

        alert_layout.addWidget(QLabel("CPU Temp High (°C):"), 0, 0)
        self.cpu_alert_spin = QSpinBox()
        self.cpu_alert_spin.setRange(0, 150)
        self.cpu_alert_spin.setValue(80)
        alert_layout.addWidget(self.cpu_alert_spin, 0, 1)

        alert_layout.addWidget(QLabel("Memory Low (%):"), 1, 0)
        self.mem_alert_spin = QSpinBox()
        self.mem_alert_spin.setRange(0, 100)
        self.mem_alert_spin.setValue(20)
        alert_layout.addWidget(self.mem_alert_spin, 1, 1)

        alert_layout.addWidget(QLabel("Disk Low (%):"), 2, 0)
        self.disk_alert_spin = QSpinBox()
        self.disk_alert_spin.setRange(0, 100)
        self.disk_alert_spin.setValue(10)
        alert_layout.addWidget(self.disk_alert_spin, 2, 1)

        apply_alerts_btn = QPushButton("Apply Alert Settings")
        apply_alerts_btn.clicked.connect(self.apply_alert_settings)
        alert_layout.addWidget(apply_alerts_btn, 3, 0, 1, 2)

        layout.addWidget(alert_group)

        # Reset button
        reset_btn = QPushButton("Reset ESP32")
        reset_btn.clicked.connect(self.reset_device)
        layout.addWidget(reset_btn)

        layout.addStretch()
        return widget

    def create_status_tab(self):
        """Create status display tab"""
        tab = QWidget()
        layout = QVBoxLayout(tab)

        # Get status button
        status_btn = QPushButton("Get Device Status")
        status_btn.clicked.connect(self.get_device_status)
        layout.addWidget(status_btn)

        # Status display
        self.status_text = QTextEdit()
        self.status_text.setReadOnly(True)
        layout.addWidget(self.status_text)

        return tab

    # Device discovery methods
    def discover_devices(self):
        """Discover ESP32 devices on the network using mDNS"""
        if not MDNS_AVAILABLE:
            QMessageBox.warning(self, "mDNS Not Available",
                              "mDNS discovery requires zeroconf library.\n"
                              "Install with: pip install zeroconf")
            return

        # Don't start new discovery if one is already running
        if self.discovery_worker and self.discovery_worker.isRunning():
            return

        self.discover_btn.setEnabled(False)
        self.discover_btn.setText("Scanning...")
        self.log_message("Discovering devices...")

        # Create and start discovery worker
        self.discovery_worker = DiscoveryWorker(timeout=5.0)
        self.discovery_worker.finished.connect(self.on_discovery_complete)
        self.discovery_worker.error.connect(self.on_discovery_error)
        self.discovery_worker.start()

    def on_discovery_complete(self, devices):
        """Handle discovery completion"""
        print(f"on_discovery_complete called with {len(devices) if devices else 0} devices")
        self.discover_btn.setEnabled(True)
        self.discover_btn.setText("Scan Network")

        self.device_combo.clear()

        if devices:
            self.log_message(f"Found {len(devices)} device(s)")
            # Store devices for later use
            self.discovered_devices = devices

            for device in devices:
                # Format: "name (IP:port)"
                display_text = f"{device['name']} ({device['addresses'][0]}:{device['port']})"
                print(f"Adding device to combo: {display_text}")
                self.device_combo.addItem(display_text)
        else:
            self.device_combo.addItem("No devices found")
            self.log_message("No devices found")

    def on_discovery_error(self, error_msg):
        """Handle discovery error"""
        self.discover_btn.setEnabled(True)
        self.discover_btn.setText("Scan Network")
        self.device_combo.clear()
        self.device_combo.addItem("Discovery failed")
        self.show_error(f"Device discovery failed: {error_msg}")

    def on_device_selected(self, index):
        """Handle device selection from discovery dropdown"""
        if not hasattr(self, 'discovered_devices') or not self.discovered_devices:
            return

        if index < 0 or index >= len(self.discovered_devices):
            return

        device = self.discovered_devices[index]
        # Update host and port fields
        self.host_edit.setText(device['addresses'][0])
        self.port_spin.setValue(device['port'])
        self.log_message(f"Selected device: {device['name']}")

    # Monitor control methods
    def start_monitoring(self):
        """Start system monitoring"""
        if self.worker and self.worker.isRunning():
            return

        mode = 'wifi' if self.mode_combo.currentText() == 'WiFi' else 'ble'
        host = self.host_edit.text()
        port = self.port_spin.value()
        interval = self.interval_spin.value()

        self.worker = MonitorWorker(mode, host, port, interval)
        self.worker.data_updated.connect(self.update_live_data)
        self.worker.status_updated.connect(self.log_message)
        self.worker.error_occurred.connect(self.show_error)
        self.worker.start()

        self.start_btn.setEnabled(False)
        self.stop_btn.setEnabled(True)
        self.log_message("Monitoring started")

    def stop_monitoring(self):
        """Stop system monitoring"""
        if self.worker:
            self.worker.stop()
            self.worker = None

        self.start_btn.setEnabled(True)
        self.stop_btn.setEnabled(False)
        self.log_message("Monitoring stopped")

    def update_live_data(self, data):
        """Update live data display"""
        # CPU
        cpu_usage = data['cpu']['usage']
        self.cpu_label.setText(f"{cpu_usage:.1f} %")
        self.cpu_progress.setValue(int(cpu_usage))

        cpu_temp = data['cpu']['temp']
        self.cpu_temp_label.setText(f"{cpu_temp:.1f} °C")

        # Memory
        mem_percent = data['memory']['percent']
        self.mem_label.setText(f"{mem_percent:.1f} %")
        self.mem_progress.setValue(int(mem_percent))

        # Disk
        disk_percent = data['disk']['percent']
        self.disk_label.setText(f"{disk_percent:.1f} %")
        self.disk_progress.setValue(int(disk_percent))

        # Network
        upload = data['network']['upload']
        download = data['network']['download']
        self.net_label.setText(f"↑ {upload:.2f} KB/s  ↓ {download:.2f} KB/s")

    # Serial/CLI methods
    def refresh_ports(self):
        """Refresh serial port list"""
        if not self.cli:
            return
        self.port_combo.clear()
        ports = SerialCLI.list_ports()
        self.port_combo.addItems(ports)

    def toggle_serial_connection(self):
        """Toggle serial connection"""
        if not self.cli:
            return

        if self.cli.serial and self.cli.serial.is_open:
            self.cli.disconnect()
            self.connect_btn.setText("Connect")
            self.log_message("Serial disconnected")
        else:
            port = self.port_combo.currentText()
            if self.cli.connect(port):
                self.connect_btn.setText("Disconnect")
                self.log_message(f"Connected to {port}")
            else:
                self.show_error("Failed to connect to serial port")

    def send_cli_command(self, command):
        """Send CLI command and display response"""
        print(f"send_cli_command called: '{command}'")
        if not self.cli or not self.cli.serial or not self.cli.serial.is_open:
            self.show_error("Not connected to serial port")
            print("Serial port not connected!")
            return None

        print("Creating CommandWorker...")
        # Create and start command worker
        worker = CommandWorker(self.cli, command)

        # Connect signals
        worker.finished.connect(self.on_command_response)
        worker.error.connect(lambda msg: self.show_error(f"Command error: {msg}"))

        # Keep reference so worker isn't garbage collected
        if not hasattr(self, 'command_workers'):
            self.command_workers = []
        self.command_workers.append(worker)

        # Cleanup on completion
        def cleanup():
            print(f"Cleaning up worker for command: {command}")
            if worker in self.command_workers:
                self.command_workers.remove(worker)

        worker.finished.connect(cleanup)
        worker.error.connect(cleanup)

        print("Starting CommandWorker...")
        worker.start()

        return None

    def on_command_response(self, command, response):
        """Handle command response"""
        print(f"on_command_response called - command: '{command}', response length: {len(response) if response else 0}")
        if response:
            print(f"Response content: {response[:200]}")  # First 200 chars
            self.response_text.append(f"> {command}\n{response}\n")
            self.log_message(f"Command '{command}' completed")
        else:
            self.log_message(f"Command '{command}' completed (no response)")
            self.response_text.append(f"> {command}\n(No response)\n")

    # Configuration methods
    def scan_wifi(self):
        """Scan for WiFi networks"""
        self.log_message("Scanning for WiFi networks...")
        self.send_cli_command("scanwifi")

    def set_wifi(self):
        """Set WiFi credentials"""
        ssid = self.ssid_edit.text()
        password = self.wifi_pass_edit.text()

        if not ssid:
            self.show_error("SSID cannot be empty")
            return

        # Use quotes if SSID contains spaces
        if ' ' in ssid:
            command = f'setwifi "{ssid}" {password}'
        else:
            command = f'setwifi {ssid} {password}'

        self.send_cli_command(command)

    def apply_display_settings(self):
        """Apply display settings"""
        theme = self.theme_combo.currentIndex()
        brightness = self.brightness_spin.value()
        idle_timeout = self.idle_timeout_spin.value()

        self.send_cli_command(f"settheme {theme}")
        self.send_cli_command(f"setbrightness {brightness}")
        self.send_cli_command(f"setidletimeout {idle_timeout}")

    def set_datetime(self):
        """Set date/time on device"""
        dt = self.datetime_edit.dateTime()
        datetime_str = dt.toString("yyyy-MM-dd HH:mm:ss")
        self.send_cli_command(f"setdatetime {datetime_str}")

    def sync_ntp(self):
        """Synchronize time with NTP"""
        server = self.ntp_server_edit.text()
        timezone = self.timezone_spin.value()

        self.send_cli_command(f"setntpserver {server}")
        self.send_cli_command(f"settimezone {timezone}")
        self.send_cli_command("syncntp")

    def apply_alert_settings(self):
        """Apply alert threshold settings"""
        cpu_temp = self.cpu_alert_spin.value()
        mem_low = self.mem_alert_spin.value()
        disk_low = self.disk_alert_spin.value()

        self.send_cli_command(f"setalert cpu {cpu_temp}")
        self.send_cli_command(f"setalert mem {mem_low}")
        self.send_cli_command(f"setalert disk {disk_low}")

    def get_device_status(self):
        """Get device status"""
        response = self.send_cli_command("status")
        if response:
            self.status_text.setText(response)

    def reset_device(self):
        """Reset ESP32 device"""
        reply = QMessageBox.question(self, 'Confirm Reset',
                                     'Are you sure you want to reset the device to defaults?',
                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)

        if reply == QMessageBox.Yes:
            self.send_cli_command("reset")

    # Utility methods
    def log_message(self, message):
        """Log a message"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.append(f"[{timestamp}] {message}")
        self.statusBar().showMessage(message)

    def show_error(self, message):
        """Show error message"""
        self.log_message(f"ERROR: {message}")
        QMessageBox.warning(self, "Error", message)

    def closeEvent(self, event):
        """Handle window close event"""
        if self.worker and self.worker.isRunning():
            self.worker.stop()
        if self.cli:
            self.cli.disconnect()
        event.accept()


def main():
    app = QApplication(sys.argv)

    # Set application style
    app.setStyle('Fusion')

    # Create and show main window
    window = MonitorGUI()
    window.show()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
