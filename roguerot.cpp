// ============================================================
// roguerot.cpp — Настоящий пиксель-ридер (Zero Delay Edition)
// Читает цвета от ComRogue.lua и мгновенно жмет кнопки
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <string>

std::atomic<bool> g_running(false);
HWND g_wowHwnd = NULL;
std::thread g_thread;
std::ofstream g_log;
int g_press_delay = 10; // 10 мс удержание кнопки (чтобы игра успела зарегать)

// Функция для высокоточной записи в лог
void LogAction(const std::string& action) {
    if (!g_log.is_open()) return;
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    char buffer[256];
    sprintf(buffer, "[%02d:%02d:%02d.%03d] %s\n", st.wHour, st.wMinute, st.wSecond, (int)ms.count(), action.c_str());
    g_log << buffer;
    g_log.flush();
}

// Эмуляция нажатия клавиатуры
void SendKey(WORD vk) {
    INPUT inp[2] = {0};
    inp[0].type = INPUT_KEYBOARD;
    inp[0].ki.wVk = vk;
    
    inp[1].type = INPUT_KEYBOARD;
    inp[1].ki.wVk = vk;
    inp[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &inp[0], sizeof(INPUT));
    Sleep(g_press_delay); 
    SendInput(1, &inp[1], sizeof(INPUT));
}

// Эмуляция боковой кнопки мыши (Mouse4)
void SendMouse4() {
    INPUT inp[2] = {0};
    inp[0].type = INPUT_MOUSE;
    inp[0].mi.dwFlags = MOUSEEVENTF_XDOWN;
    inp[0].mi.mouseData = XBUTTON1;

    inp[1].type = INPUT_MOUSE;
    inp[1].mi.dwFlags = MOUSEEVENTF_XUP;
    inp[1].mi.mouseData = XBUTTON1;

    SendInput(1, &inp[0], sizeof(INPUT));
    Sleep(g_press_delay);
    SendInput(1, &inp[1], sizeof(INPUT));
}

// Основной цикл сканирования
void RotationThread() {
    LogAction("ПОТОК РОТАЦИИ ЗАПУЩЕН. Подключение к окну...");
    
    HDC hdc = GetDC(g_wowHwnd);
    if (!hdc) {
        LogAction("ОШИБКА: Не удалось получить контекст окна (HDC)!");
        g_running.store(false);
        return;
    }

    int last_action = 0;

    while (g_running.load()) {
        // Читаем пиксель 0,0
        COLORREF color = GetPixel(hdc, 0, 0);
        int r = GetRValue(color);
        int g = GetGValue(color);
        int b = GetBValue(color);

        // Расшифровка цвета в кнопку (пороги > 200 защищают от искажений гаммы)
        int current_action = 0; // 0 = NONE
        
        if (r > 200 && g < 50 && b < 50) current_action = 1;         // SS (1)
        else if (r < 50 && g > 200 && b < 50) current_action = 2;    // SND (2)
        else if (r < 50 && g < 50 && b > 200) current_action = 3;    // EVIS (Q - 0x51)
        else if (r > 200 && g > 200 && b < 50) current_action = 4;   // KS (4)
        else if (r < 50 && g > 200 && b > 200) current_action = 5;   // AR (Mouse4)
        else if (r > 200 && g < 50 && b > 200) current_action = 6;   // BF (3)

        // Если бот просит нажать что-то новое, жмем
        if (current_action != 0 && current_action != last_action) {
            char logMsg[128];
            sprintf(logMsg, "Детект RGB(%d,%d,%d) -> Действие ID: %d", r, g, b, current_action);
            LogAction(logMsg);

            switch (current_action) {
                case 1: SendKey(0x31); break; // 1
                case 2: SendKey(0x32); break; // 2
                case 3: SendKey(0x51); break; // Q
                case 4: SendKey(0x34); break; // 4
                case 5: SendMouse4();  break; // Mouse4
                case 6: SendKey(0x33); break; // 3
            }
        }
        
        last_action = current_action;
        Sleep(2); // Квантовая пауза в 2мс чтобы не сжечь процессор
    }

    ReleaseDC(g_wowHwnd, hdc);
    LogAction("ПОТОК РОТАЦИИ ОСТАНОВЛЕН.");
}

// Экспорты для loader.cpp
extern "C" {
    __declspec(dllexport) BOOL __stdcall FindWoW() {
        g_wowHwnd = FindWindowA(NULL, "World of Warcraft");
        if (g_wowHwnd) {
            g_log.open("RogueBot_Log.txt", std::ios::app);
            LogAction("=== WOW НАЙДЕН. БОТ ИНИЦИАЛИЗИРОВАН ===");
            return TRUE;
        }
        return FALSE;
    }

    __declspec(dllexport) BOOL __stdcall StartRotation() {
        if (g_running.load()) return FALSE;
        g_running.store(true);
        g_thread = std::thread(RotationThread);
        return TRUE;
    }

    __declspec(dllexport) void __stdcall StopRotation() {
        g_running.store(false);
        if (g_thread.joinable()) g_thread.join();
        if (g_log.is_open()) g_log.close();
    }

    __declspec(dllexport) BOOL __stdcall IsRunning() {
        return g_running.load() ? TRUE : FALSE;
    }
}
