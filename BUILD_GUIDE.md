# Build & Deployment Guide

## Prerequisites

### System Requirements
- Windows 10 or Windows 11
- GPU with DirectX 11 support (virtually all modern GPUs)
- 4GB RAM minimum
- 100MB free disk space

### Software Requirements
- **Visual Studio 2019** or **2022** (Community Edition is free)
  - C++ Desktop Development workload
  - Windows 10/11 SDK
- **DirectX 11 SDK** (usually included with Windows SDK)

## Installation

### Step 1: Visual Studio Setup

1. Download Visual Studio from: https://visualstudio.microsoft.com/
2. Run the installer
3. Select "Desktop development with C++"
4. Install required components:
   - MSVC v14x compiler toolset
   - Windows 10/11 SDK
   - CMake tools for Windows (optional)

### Step 2: Clone/Download Project

```powershell
# Clone if using Git
git clone <your-repo-url> AuroraCrosshairDX
cd AuroraCrosshairDX

# Or download and extract ZIP
```

### Step 3: Prepare Crosshairs

Create the crosshairs folder and add PNG files:
```
AuroraCrosshairDX/
└── resources/
    └── crosshairs/
        ├── default.png
        ├── dot.png
        └── circle.png
```

## Building

### Option 1: Visual Studio IDE

1. Open `AuroraCrosshairDX.sln` in Visual Studio
2. Select configuration: **Release | x64**
3. Build menu → Build Solution (Ctrl+Shift+B)
4. Output: `AuroraCrosshairDX\x64\Release\AuroraCrosshairDX.exe`

### Option 2: Command Line

```powershell
# Using MSBuild
cd C:\path\to\AuroraCrosshairDX
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
  AuroraCrosshairDX.sln ^
  /p:Configuration=Release ^
  /p:Platform=x64 ^
  /p:PlatformToolset=v143
```

### Build Configurations

| Config | Usage | Optimization |
|--------|-------|--------------|
| Debug | Development | None (debug symbols) |
| Release | Production | Optimized for speed |
| x64 | 64-bit Windows | Recommended |
| Win32 | 32-bit Windows | Legacy systems |

## Running the Application

### From Visual Studio

1. Press `F5` (Start Debugging) or `Ctrl+F5` (Start Without Debugging)
2. Application runs as overlay
3. Press `ESC` to exit

### From Command Line

```powershell
cd "C:\Users\aurora\Documents\Crosshair overlay\AuroraCrosshairDX\x64\Release"
.\AuroraCrosshairDX.exe
```

### Desktop Shortcut

1. Create shortcut to: `AuroraCrosshairDX.exe`
2. Right-click → Properties → Advanced → Check "Run as administrator" (optional)
3. Place on desktop for quick access

## Deployment

### Distribution Package

Create a distributable package:

```
AuroraCrosshairDX-v1.0/
├── AuroraCrosshairDX.exe
├── resources/
│   └── crosshairs/
│       ├── default.png
│       └── sample.png
├── README.md
├── CONFIGURATION.md
└── GETTING_STARTED.md
```

### Installer (Optional)

Use **Inno Setup** for creating an installer:

1. Download: http://www.jrsoftware.org/isdl.php
2. Create `installer.iss`:
```ini
[Setup]
AppName=Aurora Crosshair DX
AppVersion=1.0
DefaultDirName={pf}\AuroraCrosshairDX
DefaultGroupName=Aurora Crosshair DX
OutputDir=Output

[Files]
Source: "x64\Release\AuroraCrosshairDX.exe"; DestDir: "{app}"
Source: "resources\crosshairs\*"; DestDir: "{app}\resources\crosshairs"
Source: "README.md"; DestDir: "{app}"

[Icons]
Name: "{group}\Aurora Crosshair DX"; Filename: "{app}\AuroraCrosshairDX.exe"
Name: "{group}\Uninstall"; Filename: "{uninstaller}"
```

3. Build installer in Inno Setup
4. Distribute the `.exe` installer file

## Troubleshooting Build Issues

### Error: Cannot find DirectX headers
**Solution**: Install Windows SDK
- Open Visual Studio Installer
- Modify → Workloads → Check "Desktop development with C++"
- Select Windows 11 SDK (or your version)

### Error: Linker can't find d3d11.lib
**Solution**: Verify library paths
1. Right-click project → Properties
2. VC++ Directories
3. Ensure Library Directories includes DirectX SDK paths

### Error: C1083 Cannot open include file
**Solution**: Check include directories
1. Project → Properties → VC++ Directories
2. Include Directories: Add `$(ProjectDir)include`

### Visual Studio crashes on build
**Solution**: Clean and rebuild
```powershell
# Close Visual Studio
# Delete build artifacts
Remove-Item -Path ".\AuroraCrosshairDX\x64" -Recurse -Force
Remove-Item -Path ".\AuroraCrosshairDX\.vs" -Recurse -Force

# Reopen and rebuild
```

## Testing

### Unit Test Scenarios

1. **Window Creation**
   - Verify overlay appears on screen
   - Check it's always on top
   - Confirm clicks pass through

2. **DirectX Initialization**
   - Monitor GPU memory usage
   - Check frame rate (should be smooth)
   - Verify no device lost errors

3. **Crosshair Loading**
   - Test with different PNG sizes
   - Verify transparency works
   - Test color customization

4. **Performance**
   - Monitor CPU usage (<5%)
   - Monitor GPU usage (<2%)
   - Check memory (stable, no leaks)

### Performance Profiling

Use **Visual Studio Profiler**:
1. Debug → Performance Profiler (Alt+F2)
2. Select "CPU Usage"
3. Start recording
4. Run application
5. Stop and analyze results

## Version Control

### Git Setup

```powershell
# Initialize repository
git init
git config user.name "Your Name"
git config user.email "your.email@example.com"

# Add files
git add .
git commit -m "Initial commit: AuroraCrosshairDX v1.0"

# Create .gitignore
echo @"
*.exe
*.pdb
x64/
.vs/
*.obj
*.ilk
"@ | Out-File .gitignore
```

### Release Checklist

- [ ] Code compiles without warnings
- [ ] All features tested
- [ ] Documentation updated
- [ ] Version number incremented
- [ ] Changelog updated
- [ ] Git tags created
- [ ] Release notes written

## Continuous Integration (Optional)

Setup GitHub Actions for automatic builds:

Create `.github/workflows/build.yml`:
```yaml
name: Build
on: [push, pull_request]
jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: |
          "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" ^
            AuroraCrosshairDX.sln /p:Configuration=Release /p:Platform=x64
      - name: Upload
        uses: actions/upload-artifact@v2
        with:
          name: AuroraCrosshairDX-Release
          path: AuroraCrosshairDX/x64/Release/
```

## Uninstalling

### Cleanup

```powershell
# Stop the application
# Delete the executable
Remove-Item -Path "C:\path\to\AuroraCrosshairDX.exe" -Force

# Delete configuration/cache
Remove-Item -Path "$env:APPDATA\AuroraCrosshairDX" -Recurse -Force -ErrorAction SilentlyContinue
```

## Support & Updates

- Check releases: GitHub Releases page
- Report issues: GitHub Issues
- Request features: GitHub Discussions
- Email: your.email@example.com
