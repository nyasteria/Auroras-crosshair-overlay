#include "CrosshairManager.h"
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

CrosshairManager::CrosshairManager()
{
    currentConfig.scale = 1.0f;
    currentConfig.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    currentConfig.posX = 960;
    currentConfig.posY = 540;
}

CrosshairManager::~CrosshairManager()
{
}

bool CrosshairManager::LoadCrosshairsFromDirectory(const std::wstring& directory)
{
    crosshairList.clear();

    try
    {
        if (!fs::exists(directory))
            return false;

        for (const auto& entry : fs::directory_iterator(directory))
        {
            if (entry.is_regular_file())
            {
                std::wstring extension = entry.path().extension().wstring();
                if (extension == L".png" || extension == L".PNG")
                {
                    crosshairList.push_back(entry.path().filename().wstring());
                }
            }
        }

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool CrosshairManager::SelectCrosshair(size_t index)
{
    if (index >= crosshairList.size())
        return false;

    currentConfig.filePath = crosshairList[index];
    return true;
}
