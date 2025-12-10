#pragma once

#include <vector>
#include <string>
#include <DirectXMath.h>

using namespace DirectX;

struct CrosshairConfig
{
    std::wstring filePath;
    float scale;
    XMFLOAT4 color;
    int posX;
    int posY;
};

class CrosshairManager
{
public:
    CrosshairManager();
    ~CrosshairManager();

    bool LoadCrosshairsFromDirectory(const std::wstring& directory);
    const std::vector<std::wstring>& GetCrosshairList() const { return crosshairList; }
    
    bool SelectCrosshair(size_t index);
    const CrosshairConfig& GetCurrentConfig() const { return currentConfig; }
    
    void SetScale(float scale) { currentConfig.scale = scale; }
    void SetColor(float r, float g, float b, float a = 1.0f) 
    { 
        currentConfig.color = XMFLOAT4(r, g, b, a); 
    }
    void SetPosition(int x, int y) 
    { 
        currentConfig.posX = x; 
        currentConfig.posY = y; 
    }

private:
    std::vector<std::wstring> crosshairList;
    CrosshairConfig currentConfig;
};
