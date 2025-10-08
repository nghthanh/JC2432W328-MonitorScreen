@echo off
REM ESP32 System Monitor GUI Launcher (Windows)

echo Starting ESP32 System Monitor GUI...
echo.

REM Check if Python is installed
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python is not installed or not in PATH
    echo Please install Python 3.7 or later from https://www.python.org/
    pause
    exit /b 1
)

REM Check if PyQt5 is installed
python -c "import PyQt5" >nul 2>&1
if errorlevel 1 (
    echo PyQt5 not found. Installing dependencies...
    echo.
    pip install -r requirements_gui.txt
    echo.
)

REM Run the GUI
python monitor_gui.py

if errorlevel 1 (
    echo.
    echo ERROR: Failed to start GUI
    pause
)
