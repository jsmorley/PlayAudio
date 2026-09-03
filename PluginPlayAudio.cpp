#include <Windows.h>
#include <string>
#include <sstream>
#include "../../API/RainmeterAPI.h"

#pragma comment(lib, "winmm.lib")

struct Measure
{
    std::wstring file;
    int volume = 100;           // 0–100 (user-facing)
    std::wstring alias;
    bool opened = false;
};

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
    Measure* measure = new Measure;
    *data = measure;

    // Unique alias based on measure name
    LPCWSTR name = RmGetMeasureName(rm);
    measure->alias = L"PlayMP3_";
    measure->alias += name;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
    Measure* measure = (Measure*)data;

    // Read File path
    LPCWSTR file = RmReadPath(rm, L"File", L"");
    measure->file = file ? file : L"";

    // Read Volume (0–100)
    measure->volume = RmReadInt(rm, L"Volume", 100);
    if (measure->volume < 0) measure->volume = 0;
    if (measure->volume > 100) measure->volume = 100;

    // Close previous device if open
    if (measure->opened)
    {
        std::wstring cmd = L"close " + measure->alias;
        mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
        measure->opened = false;
    }
}

PLUGIN_EXPORT double Update(void* data)
{
    // Do nothing – play only on command
    return 0.0;
}

PLUGIN_EXPORT void Finalize(void* data)
{
    Measure* measure = (Measure*)data;

    if (measure->opened)
    {
        std::wstring cmd = L"close " + measure->alias;
        mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
    }

    delete measure;
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
    Measure* measure = (Measure*)data;

    if (measure->file.empty())
        return;

    std::wstring arg = args ? args : L"";

    // --- Open the file if needed ---
    if (!measure->opened)
    {
        std::wstringstream ss;
        ss << L"open \"" << measure->file << L"\" type mpegvideo alias " << measure->alias;
        if (mciSendStringW(ss.str().c_str(), nullptr, 0, nullptr) != 0)
            return;  // failed to open

        measure->opened = true;

        // Convert 0–100 → 0–1000 for MCI
        int mciVolume = measure->volume * 10;

        ss.str(L"");
        ss << L"setaudio " << measure->alias << L" volume to " << mciVolume;
        mciSendStringW(ss.str().c_str(), nullptr, 0, nullptr);
    }

    // --- Handle commands ---
    if (_wcsicmp(arg.c_str(), L"Play") == 0)
    {
        std::wstring cmd = L"play " + measure->alias + L" from 0";
        mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
    }
    else if (_wcsicmp(arg.c_str(), L"Stop") == 0)
    {
        std::wstring cmd = L"stop " + measure->alias;
        mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
    }
    else if (_wcsicmp(arg.c_str(), L"Pause") == 0)
    {
        std::wstring cmd = L"pause " + measure->alias;
        mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
    }
    else if (_wcsicmp(arg.c_str(), L"Resume") == 0)
    {
        std::wstring cmd = L"resume " + measure->alias;
        mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
    }
    else if (_wcsicmp(arg.c_str(), L"Close") == 0)
    {
        std::wstring cmd = L"close " + measure->alias;
        mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
        measure->opened = false;
    }
    // Empty argument also plays
    else if (arg.empty())
    {
        std::wstring cmd = L"play " + measure->alias + L" from 0";
        mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
    }
}
