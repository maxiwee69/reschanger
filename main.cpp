#include <windows.h>
#include <tlhelp32.h>
#include <unordered_set>
#include <string>
#include <iostream>
#include <thread>
#include <fstream>

std::unordered_set<std::string> appRunning = {
    "FortniteClient-Win64-Shipping.exe",
    "RainbowSix_Vulkan.exe",
    "RainbowSix.exe",
    "cs2.exe",
    "GTA5.exe"
};

std::unordered_set<std::string> promptedApps;

DEVMODE originalResolution;
DWORD originalRefreshRate;

void LogMessage(const std::string& message) {
    std::ofstream logFile;
    logFile.open("log.txt", std::ios_base::app); // Append to the log file
    logFile << message << std::endl;
    logFile.close();
}

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
        if (strcmp(pEntry.szExeFile, processName.c_str()) == 0) {
            CloseHandle(hSnapShot);
            return true;
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
    return false;
}

void ChangeResolution(const std::string& appName) {
    if (promptedApps.find(appName) != promptedApps.end()) {
        return;
    }

    promptedApps.insert(appName);

    int width = 0, height = 0;

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

    if (width == originalResolution.dmPelsWidth && height == originalResolution.dmPelsHeight) {
        return;
    }

    DEVMODE dmScreenSettings = {0};
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
    dmScreenSettings.dmDriverExtra = 0;
    dmScreenSettings.dmPelsWidth = width;
    dmScreenSettings.dmPelsHeight = height;
    dmScreenSettings.dmDisplayFrequency = 144;
    dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

    LONG result = ChangeDisplaySettings(&dmScreenSettings, CDS_UPDATEREGISTRY);
    if (result != DISP_CHANGE_SUCCESSFUL) {
        MessageBoxW(NULL, L"Failed to change display settings. The requested graphics mode may not be supported.", L"Error", MB_OK | MB_ICONERROR);
        LogMessage("Failed to change display settings for " + appName);
    } else {
        LogMessage("Changed display settings for " + appName);
    }
}

void RestoreResolution() {
    ChangeDisplaySettings(&originalResolution, 0);
    LogMessage("Restored original display settings");
}

int main() {
    SaveCurrentResolution();

    while (true) {
        for (const auto& app : appRunning) {
            bool isRunning = IsProcessRunning(app);
            if (isRunning) {
                ChangeResolution(app);
            } else if (!isRunning && promptedApps.find(app) != promptedApps.end()) {
                promptedApps.erase(app);
                RestoreResolution();
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}