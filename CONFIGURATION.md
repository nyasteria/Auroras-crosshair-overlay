# Configuration Guide

## Customizing Your Overlay

### Basic Settings (in `src/main.cpp`)

Modify the crosshair configuration in the `wWinMain` function:

```cpp
// Set initial position (center of screen)
crosshairManager.SetPosition(960, 540);

// Set scale (1.0 = original size)
crosshairManager.SetScale(1.5f);

// Set color (R, G, B, Alpha)
// White: (1.0, 1.0, 1.0, 1.0)
// Red: (1.0, 0.0, 0.0, 1.0)
// Green: (0.0, 1.0, 0.0, 1.0)
// Blue: (0.0, 0.0, 1.0, 1.0)
crosshairManager.SetColor(0.0f, 1.0f, 0.0f, 1.0f);  // Green
```

### Creating Custom Crosshairs

#### Requirements
- Format: PNG image file
- Color: Any color (will be colored by the RGB settings)
- Background: **Must be transparent** (RGBA format with alpha channel)
- Size: Recommended 256x256 pixels minimum
- File location: `resources/crosshairs/`

#### Creating a Simple Crosshair

1. **Using Photoshop/GIMP**:
   - Create a new 256x256 image with transparent background
   - Draw your crosshair design (use white or any solid color)
   - Export as PNG with transparency enabled
   - Place in `resources/crosshairs/`

2. **Using Paint.NET**:
   - New image (256x256, transparent)
   - Use brush tool to draw crosshair
   - File > Export As > PNG
   - Place in `resources/crosshairs/`

3. **Using Online Tools**:
   - Visit: https://www.photopea.com/ (Photoshop clone)
   - Create transparent PNG
   - Download and place in `resources/crosshairs/`

#### Popular Crosshair Designs

**Plus Sign** (Simple):
```
        |
        |
   -----+-----
        |
        |
```

**Circle** (Centered):
```
      ° °
    °     °
   °   •   °
    °     °
      ° °
```

**Dot with Corners**:
```
*       *


    ●


*       *
```

### Window Settings

Edit `src/Window.cpp` to customize the overlay window:

```cpp
// Window size (1920x1080 = fullscreen 1080p)
bool Window::Create(const std::wstring& title, unsigned int width, unsigned int height)
```

Change values like:
- `1920` → width in pixels
- `1080` → height in pixels

### Display Resolution Matching

Match your window size to your monitor resolution:

| Resolution | Width | Height |
|-----------|-------|--------|
| 1080p     | 1920  | 1080   |
| 1440p     | 2560  | 1440   |
| 4K (2160p)| 3840  | 2160   |
| 1080p (24") | 1920| 1080   |

## Advanced Configuration

### Changing Render Target Position

In `DXRenderer.cpp`, modify the projection matrix to offset the rendering:

```cpp
// Current centered at (960, 540)
float screenX = 960.0f;
float screenY = 540.0f;

renderer.RenderCrosshair(screenX, screenY, scale, r, g, b, a);
```

### Alpha Blending

The alpha value (0.0 to 1.0) controls transparency:
- `1.0` = fully opaque
- `0.5` = 50% transparent
- `0.0` = fully transparent

Example for semi-transparent red:
```cpp
crosshairManager.SetColor(1.0f, 0.0f, 0.0f, 0.7f);  // 70% opacity
```

## Multiple Crosshairs

To support multiple crosshair styles:

1. Place multiple PNG files in `resources/crosshairs/`:
   - `crosshair_dot.png`
   - `crosshair_plus.png`
   - `crosshair_circle.png`

2. Modify `main.cpp` to load selected crosshair:
```cpp
if (crosshairManager.GetCrosshairList().size() > 0)
{
    // Select different index (0, 1, 2, etc.)
    crosshairManager.SelectCrosshair(0);  // First crosshair
    
    const auto& config = crosshairManager.GetCurrentConfig();
    renderer.LoadPNGTexture(config.filePath.c_str());
}
```

## Troubleshooting Configuration

### Crosshair appears stretched
- Use square PNG images (256x256, 512x512, etc.)
- Adjust scale value if too large/small

### Crosshair position off-center
- Ensure your PNG has crosshair centered
- Adjust `SetPosition()` values
- Try `SetPosition(960, 540)` for exact center

### Crosshair colors look wrong
- Verify PNG is not already colored (should be white/grayscale)
- Check alpha channel is enabled in PNG
- Ensure `SetColor()` uses values between 0.0 and 1.0

### Transparency not working
- **Most common issue**: PNG file doesn't have alpha channel
- Re-export PNG with "Save transparency" or "Alpha channel" enabled
- Use PNG-24 or PNG-32 format (not PNG-8)

## Performance Notes

- The overlay uses minimal CPU/GPU resources
- Average frame time: <1ms at 60 FPS
- Memory usage: ~50-100MB
- Works well alongside games and applications

## Reverting Changes

To restore defaults, edit `src/main.cpp`:

```cpp
// Default position: center screen
crosshairManager.SetPosition(960, 540);

// Default scale: 1.0
crosshairManager.SetScale(1.0f);

// Default color: white
crosshairManager.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
```

Rebuild the solution after making changes.
