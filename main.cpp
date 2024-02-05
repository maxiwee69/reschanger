#include <windows.h>
#include <tlhelp32.h>
#include <map>
#include <string>
#include <iostream>
#include <thread>
 
std::map<std::string, bool> appRunning = {
    {"FortniteClient-Win64-Shipping.exe", false},
    {"RainbowSix_Vulkan.exe", false},
    {"RainbowSix.exe", false},
    {"cs2.exe", false}
};
 
DEVMODE originalResolution;
DWORD originalRefreshRate;
 
void SaveCurrentResolution() {
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &originalResolution);
    originalRefreshRate = originalResolution.dmDisplayFrequency;
}
 
bool IsProcessRunning(const char *processName) {
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes) {
        if (strcmp(pEntry.szExeFile, processName) == 0) {
            CloseHandle(hSnapShot);
            return true;
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
    return false;
}
 
void ChangeResolution(int width, int height, int frequency) {
    DEVMODE dmScreenSettings = {0};
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
    dmScreenSettings.dmDriverExtra = 0;
    dmScreenSettings.dmPelsWidth = width;
    dmScreenSettings.dmPelsHeight = height;
    dmScreenSettings.dmDisplayFrequency = frequency;
    dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
 
    LONG result = ChangeDisplaySettings(&dmScreenSettings, CDS_UPDATEREGISTRY);
    if (result != DISP_CHANGE_SUCCESSFUL) {
        MessageBoxW(NULL, L"Failed to change display settings. The requested graphics mode may not be supported.", L"Error", MB_OK | MB_ICONERROR);
    }
}
 
void RestoreResolution() {
    ChangeDisplaySettings(&originalResolution, 0);
}
 
int main() {
    SaveCurrentResolution();
 
    while (true) {
        for (auto &app : appRunning) {
            bool isRunning = IsProcessRunning(app.first.c_str());
            if (isRunning && !app.second) {
                ChangeResolution(1440, 1080, 144);
                app.second = true;
            } else if (!isRunning && app.second) {
                RestoreResolution();
                app.second = false;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
 
    return 0;
}