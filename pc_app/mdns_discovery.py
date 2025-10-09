#!/usr/bin/env python3
"""
mDNS Service Discovery for ESP32 Monitor

Discovers ESP32 Monitor devices on the local network using mDNS (DNS-SD).
Uses zeroconf library for cross-platform mDNS support.

Requirements:
- zeroconf: pip install zeroconf
"""

from zeroconf import ServiceBrowser, ServiceListener, Zeroconf
import socket
import time
from typing import List, Dict, Optional


class ESP32ServiceListener(ServiceListener):
    """Listens for ESP32 Monitor mDNS service announcements"""

    def __init__(self):
        self.devices = []
        self.device_dict = {}  # Track by name to avoid duplicates

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        """Called when a new service is discovered"""
        info = zc.get_service_info(type_, name)
        if info:
            # Extract device information
            device_name = name.split('.')[0]
            addresses = [socket.inet_ntoa(addr) for addr in info.addresses]
            port = info.port

            device_info = {
                'name': device_name,
                'hostname': info.server.rstrip('.'),
                'addresses': addresses,
                'port': port,
                'full_name': name
            }

            # Avoid duplicates
            if device_name not in self.device_dict:
                self.device_dict[device_name] = device_info
                self.devices.append(device_info)

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        """Called when a service is removed"""
        device_name = name.split('.')[0]
        if device_name in self.device_dict:
            device_info = self.device_dict[device_name]
            if device_info in self.devices:
                self.devices.remove(device_info)
            del self.device_dict[device_name]

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        """Called when a service is updated"""
        # For now, treat as add
        self.add_service(zc, type_, name)


class MDNSDiscovery:
    """Discovers ESP32 Monitor devices using mDNS"""

    SERVICE_TYPE = "_esp32monitor._udp.local."

    def __init__(self):
        self.zeroconf = None
        self.browser = None
        self.listener = None

    def discover(self, timeout: float = 3.0) -> List[Dict]:
        """
        Discover ESP32 Monitor devices on the network.

        Args:
            timeout: Time to wait for discovery in seconds

        Returns:
            List of device dictionaries with keys: name, hostname, addresses, port
        """
        try:
            # Create zeroconf instance
            self.zeroconf = Zeroconf()
            self.listener = ESP32ServiceListener()

            # Start browsing for services
            self.browser = ServiceBrowser(
                self.zeroconf,
                self.SERVICE_TYPE,
                self.listener
            )

            # Wait for discovery
            time.sleep(timeout)

            # Get discovered devices
            devices = self.listener.devices.copy()

            # Clean up
            self.close()

            return devices

        except Exception as e:
            print(f"mDNS discovery error: {e}")
            self.close()
            return []

    def discover_by_name(self, hostname: str, timeout: float = 3.0) -> Optional[Dict]:
        """
        Discover a specific ESP32 Monitor device by hostname.

        Args:
            hostname: mDNS hostname (without .local)
            timeout: Time to wait for discovery in seconds

        Returns:
            Device dictionary or None if not found
        """
        devices = self.discover(timeout)

        # Search for device by hostname
        for device in devices:
            if device['hostname'].startswith(hostname):
                return device

        return None

    def resolve_hostname(self, hostname: str, timeout: float = 3.0) -> Optional[str]:
        """
        Resolve a .local hostname to an IP address.

        Args:
            hostname: mDNS hostname (e.g., "esp32monitor.local" or "esp32monitor")
            timeout: Time to wait for resolution in seconds

        Returns:
            IP address string or None if not found
        """
        # Remove .local suffix if present
        if hostname.endswith('.local'):
            hostname = hostname[:-6]

        device = self.discover_by_name(hostname, timeout)

        if device and device['addresses']:
            return device['addresses'][0]

        return None

    def close(self):
        """Close the mDNS browser and zeroconf instance"""
        try:
            if self.browser:
                self.browser.cancel()
                self.browser = None
            if self.zeroconf:
                self.zeroconf.close()
                self.zeroconf = None
        except Exception:
            pass


def discover_devices(timeout: float = 3.0) -> List[Dict]:
    """
    Convenience function to discover ESP32 Monitor devices.

    Args:
        timeout: Time to wait for discovery in seconds

    Returns:
        List of device dictionaries
    """
    discovery = MDNSDiscovery()
    return discovery.discover(timeout)


def resolve_hostname(hostname: str, timeout: float = 3.0) -> Optional[str]:
    """
    Convenience function to resolve a .local hostname to IP address.

    Args:
        hostname: mDNS hostname (e.g., "esp32monitor.local" or "esp32monitor")
        timeout: Time to wait for resolution in seconds

    Returns:
        IP address string or None if not found
    """
    discovery = MDNSDiscovery()
    return discovery.resolve_hostname(hostname, timeout)


if __name__ == "__main__":
    # Test mDNS discovery
    print("Discovering ESP32 Monitor devices...")
    print("Service type:", MDNSDiscovery.SERVICE_TYPE)
    print()

    devices = discover_devices(timeout=5.0)

    if devices:
        print(f"Found {len(devices)} device(s):")
        for i, device in enumerate(devices):
            print(f"\n[{i}] {device['name']}")
            print(f"    Hostname: {device['hostname']}")
            print(f"    IP Address: {', '.join(device['addresses'])}")
            print(f"    Port: {device['port']}")
    else:
        print("No devices found.")
        print("\nTroubleshooting:")
        print("- Ensure ESP32 is connected to WiFi")
        print("- Check that device is on the same network")
        print("- Verify mDNS is enabled on ESP32")
        print("- Try increasing timeout value")
