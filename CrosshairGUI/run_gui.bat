@echo off
REM Aurora Crosshair GUI Launcher

echo Installing required Python packages...
python -m pip install psutil -q

echo Starting Aurora Crosshair Controller...
python "%~dp0gui.py"

pause
