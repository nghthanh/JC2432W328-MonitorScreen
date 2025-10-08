#!/bin/bash
# ESP32 System Monitor GUI Launcher (Linux/Mac)

echo "Starting ESP32 System Monitor GUI..."
echo ""

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 is not installed"
    echo "Please install Python 3.7 or later"
    exit 1
fi

# Check if PyQt5 is installed
python3 -c "import PyQt5" &> /dev/null
if [ $? -ne 0 ]; then
    echo "PyQt5 not found. Installing dependencies..."
    echo ""
    pip3 install -r requirements_gui.txt
    echo ""
fi

# Run the GUI
python3 monitor_gui.py

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Failed to start GUI"
    read -p "Press Enter to continue..."
fi
