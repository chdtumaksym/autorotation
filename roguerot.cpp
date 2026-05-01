// ============================================================
// roguerot.cpp — Настоящий пиксель-ридер (Config Edition)
// Читает цвета от Lua и жмет кнопки, указанные в config.ini
// ============================================================

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <string>

#ifndef XBUTTON1
#define XBUTTON1 0x0001
#define XBUTTON2 0x0002
#endif
#ifndef MOUSEEVENTF_XDOWN
#define MOUSEEVENTF_XDOWN 0x0080
#define MOUSEEVENTF_XUP   0x0100
#endif

std::atomic<bool> g_running(false);
HWND g_wowHwnd = NULL;
std::thread g_thread;
std::ofstream g_log;
int g_press_delay = 10;

// Переменные для биндов
int k_color1, k_color2, k_color3, k_color4, k_color5, k_color6;

void LogAction(const std::string& action) {
    if (!g_log.is_open()) return;
    auto now = std::chrono::system_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    char buffer[256];
    sprintf(buffer, "[%02d:%02d:%02d.%03d] %s\n", st.wHour, st.wMinute, st.wSecond, (int)millis, action.c_str());
    g_log << buffer;
    g_log.flush();
}

// Универсальная функция: жмет и клавиатуру, и боковые кнопки мыши
void SendKey(WORD vk) {
    INPUT inp[2] = {0};
    
    if (vk == 0x05 || vk == 0x06) { // VK_XBUTTON1 (Mouse4) или VK_XBUTTON2 (Mouse5)
        inp[0].type = INPUT_MOUSE;
        inp[0].mi.dwFlags = MOUSEEVENTF_XDOWN;
        inp[0].mi.mouseData = (vk == 0x05) ? XBUTTON1 : XBUTTON2;

        inp[1].type = INPUT_MOUSE;
        inp[1].mi.dwFlags = MOUSEEVENTF_XUP;
        inp[1].mi.mouseData = (vk == 0x05) ? XBUTTON1 : XBUTTON2;
    } else {
        inp[0].type = INPUT_KEYBOARD;
        inp[0].ki.wVk = vk;
        
        inp[1].type = INPUT_KEYBOARD;
        inp[1].ki.wVk = vk;
        inp[1].ki.dwFlags = KEYEVENTF_KEYUP;
    }

    SendInput(1, &inp[0], sizeof(INPUT));
    Sleep(g_press_delay); 
    SendInput(1, &inp[1], sizeof(INPUT));
}

void LoadConfig() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string conf = std::string(path);
    conf = conf.substr(0, conf.find_last_of("\\/")) + "\\config.ini";

    char profile[32];
    GetPrivateProfileStringA("Settings", "ActiveProfile", "Rogue", profile, sizeof(profile), conf.c_str());

    k_color1 = GetPrivateProfileIntA(profile, "Color1", 49, conf.c_str()); 
    k_color2 = GetPrivateProfileIntA(profile, "Color2", 50, conf.c_str()); 
    k_color3 = GetPrivateProfileIntA(profile, "Color3", 81, conf.c_str()); 
    k_color4 = GetPrivateProfileIntA(profile, "Color4", 52, conf.c_str()); 
    k_color5 = GetPrivateProfileIntA(profile, "Color5", 5, conf.c_str());  
    k_color6 = GetPrivateProfileIntA(profile, "Color6", 51, conf.c_str()); 
    g_press_delay = GetPrivateProfileIntA("Settings", "Delay", 10, conf.c_str());
    
    char logMsg[256];
    sprintf(logMsg, "Конфиг загружен. Профиль: %s. C1=%d, C2=%d, C3=%d, C4=%d, C5=%d, C6=%d", 
            profile, k_color1, k_color2, k_color3, k_color4, k_color5, k_color6);
    LogAction(logMsg);
}

void RotationThread() {
    LogAction("ПОТОК РОТАЦИИ ЗАПУЩЕН.");
    HDC hdc = GetDC(g_wowHwnd);
    if (!hdc) {
        LogAction("ОШИБКА: Не удалось получить HDC!");
        g_running.store(false);
        return;
    }

    int last_action = 0;

    while (g_running.load()) {
        COLORREF color = GetPixel(hdc, 0, 0);
        int r = GetRValue(color);
        int g = GetGValue(color);
        int b = GetBValue(color);

        int current_action = 0; 
        
        if (r > 200 && g < 50 && b < 50) current_action = 1;         // Цвет 1 (Красный)
        else if (r < 50 && g > 200 && b < 50) current_action = 2;    // Цвет 2 (Зеленый)
        else if (r < 50 && g < 50 && b > 200) current_action = 3;    // Цвет 3 (Синий)
        else if (r > 200 && g > 200 && b < 50) current_action = 4;   // Цвет 4 (Желтый)
        else if (r < 50 && g > 200 && b > 200) current_action = 5;   // Цвет 5 (Голубой)
        else if (r > 200 && g < 50 && b > 200) current_action = 6;   // Цвет 6 (Фиолетовый)

        if (current_action != 0 && current_action != last_action) {
            char logMsg[128];
            sprintf(logMsg, "Детект RGB(%d,%d,%d) -> ID: %d", r, g, b, current_action);
            LogAction(logMsg);

            switch (current_action) {
                case 1: SendKey(k_color1); break;
                case 2: SendKey(k_color2); break;
                case 3: SendKey(k_color3); break;
                case 4: SendKey(k_color4); break;
                case 5: SendKey(k_color5); break;
                case 6: SendKey(k_color6); break;
            }
        }
        
        last_action = current_action;
        Sleep(2); 
    }

    ReleaseDC(g_wowHwnd, hdc);
    LogAction("ПОТОК РОТАЦИИ ОСТАНОВЛЕН.");
}

extern "C" {
    __declspec(dllexport) BOOL FindWoW() {
        g_wowHwnd = FindWindowA(NULL, "World of Warcraft");
        if (g_wowHwnd) {
            g_log.open("RogueBot_Log.txt", std::ios::app);
            LogAction("=== WOW НАЙДЕН. БОТ ИНИЦИАЛИЗИРОВАН ===");
            LoadConfig();
            return TRUE;
        }
        return FALSE;
    }

    __declspec(dllexport) BOOL StartRotation() {
        if (g_running.load()) return FALSE;
        g_running.store(true);
        g_thread = std::thread(RotationThread);
        return TRUE;
    }

    __declspec(dllexport) void StopRotation() {
        g_running.store(false);
        if (g_thread.joinable()) g_thread.join();
        if (g_log.is_open()) g_log.close();
    }

    __declspec(dllexport) BOOL IsRunning() {
        return g_running.load() ? TRUE : FALSE;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
