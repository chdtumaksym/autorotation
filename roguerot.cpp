#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>
#include <atomic>
#include <map>
#include <string>
#include <algorithm>

static std::atomic<bool> g_running(false);
static std::atomic<HWND> g_hwnd(nullptr);
static std::thread       g_thread;
static std::map<COLORREF, BYTE> g_binds;

// Функция для перевода строки в нижний регистр для поиска без учета регистра
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    char title[256];
    if (GetWindowTextA(hwnd, title, sizeof(title)) > 0) {
        std::string t = toLower(title);
        // Ищем ключевые слова: sirus, warcraft или lich king
        if (t.find("sirus") != std::string::npos || 
            t.find("warcraft") != std::string::npos || 
            t.find("lich king") != std::string::npos) {
            
            if (IsWindowVisible(hwnd)) {
                HWND* target = (HWND*)lParam;
                *target = hwnd;
                return FALSE; 
            }
        }
    }
    return TRUE;
}

void PressKey(BYTE vk) {
    HWND wow = g_hwnd.load();
    if (!wow || !IsWindow(wow)) return;
    PostMessage(wow, WM_KEYDOWN, vk, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    PostMessage(wow, WM_KEYUP, vk, 0);
}

void RotationLoop() {
    while (g_running.load()) {
        HWND wow = g_hwnd.load();
        if (!wow || !IsWindow(wow) || GetForegroundWindow() != wow) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        HDC hdc = GetDC(NULL); 
        if (hdc) {
            // Пиксель (1,1)
            COLORREF color = GetPixel(hdc, 1, 1);
            ReleaseDC(NULL, hdc);

            if (g_binds.count(color)) {
                PressKey(g_binds[color]);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

extern "C" {
    __declspec(dllexport) BOOL __stdcall FindWoWWindow() {
        HWND foundHwnd = nullptr;
        // Сначала пробуем стандартный класс
        foundHwnd = FindWindowA("GxWindowClass", nullptr);
        
        // Если не вышло (как в случае с Сирусом) - ищем по списку слов
        if (!foundHwnd) {
            EnumWindows(EnumWindowsProc, (LPARAM)&foundHwnd);
        }
        
        g_hwnd.store(foundHwnd);
        return (foundHwnd != nullptr);
    }

    __declspec(dllexport) void __stdcall BindColor(int r, int g, int b, BYTE vk) {
        g_binds[RGB(r, g, b)] = vk;
    }

    __declspec(dllexport) BOOL __stdcall StartRotation() {
        if (g_running.load()) return FALSE;
        g_running.store(true);
        g_thread = std::thread(RotationLoop);
        return TRUE;
    }

    __declspec(dllexport) void __stdcall StopRotation() {
        g_running.store(false);
        if (g_thread.joinable()) g_thread.join();
    }

    __declspec(dllexport) BOOL __stdcall IsRunning() { return g_running.load(); }
}

BOOL WINAPI DllMain(HMODULE h, DWORD r, LPVOID l) { return TRUE; }
