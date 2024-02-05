#include <windows.h>
#include <tlhelp32.h>
#include <unordered_set>
#include <string>
#include <iostream>
#include <thread>

std::unordered_set<std::string> appRunning = {
    "FortniteClient-Win64-Shipping.exe",
    "RainbowSix_Vulkan.exe",
    "RainbowSix.exe",
    "cs2.exe",
    "GTA5.exe"
};

DEVMODE originalResolution;
DWORD originalRefreshRate;

void SaveCurrentResolution() {
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &originalResolution);
    originalRefreshRate = originalResolution.dmDisplayFrequency;
}

bool IsProcessRunning(const std::string& processName) {
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes) {
        if (appRunning.find(pEntry.szExeFile) != appRunning.end()) {
            CloseHandle(hSnapShot);
            return true;
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
    return false;
}

void ChangeResolution(int width, int height, int frequency) {
    if (width == originalResolution.dmPelsWidth && height == originalResolution.dmPelsHeight && frequency == originalResolution.dmDisplayFrequency) {
        return;
    }

    DEVMODE dmScreenSettings = {0};
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
    dmScreenSettings.dmDriverExtra = 0;
    dmScreenSettings.dmPelsWidth = width;
    dmScreenSettings.dmPelsHeight = height;
    dmScreenSettings.dmDisplayFrequency = frequency;
    dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

    LONG result = ChangeDisplaySettings(&dmScreenSettings, CDS_UPDATEREGISTRY);
}

void RestoreResolution() {
    ChangeDisplaySettings(&originalResolution, 0);
}

int main() {
    SaveCurrentResolution();

    int width = 0, height = 0;

    while (true) {
        for (const auto& app : appRunning) {
            bool isRunning = IsProcessRunning(app);
            if (isRunning) {
                if (width == 0 && height == 0) { // Only show the MessageBox once
                    int msgboxID = MessageBoxA(
                        NULL,
                        "Do you want to change the resolution?\n"
                        "Yes: 1080x1080\n"
                        "No: 1440x1080\n"
                        "Cancel: No change",
                        "Resolution Change",
                        MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2
                    );

                    switch (msgboxID)
                    {
                    case IDYES:
                        width = 1080;
                        height = 1080;
                        break;
                    case IDNO:
                        width = 1440;
                        height = 1080;
                        break;
                    case IDCANCEL:
                        width = originalResolution.dmPelsWidth;
                        height = originalResolution.dmPelsHeight;
                        break;
                    }
                }

                ChangeResolution(width, height, 144);
            } else {
                RestoreResolution();
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}