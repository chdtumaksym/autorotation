#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>
#include <atomic>
#include <map>

// ============================================================
// roguerot.cpp — "Зрячая" DLL v5
// Читает пиксель (1,1) и нажимает кнопки по цветам
// ============================================================

static std::atomic<bool> g_running(false);
static std::atomic<HWND> g_hwnd(nullptr);
static std::thread       g_thread;
static std::map<COLORREF, BYTE> g_binds;

void PressKey(BYTE vk) {
    HWND wow = g_hwnd.load();
    if (!wow) return;
    PostMessage(wow, WM_KEYDOWN, vk, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    PostMessage(wow, WM_KEYUP, vk, 0);
}

void RotationLoop() {
    while (g_running.load()) {
        HWND wow = g_hwnd.load();
        if (!wow || GetForegroundWindow() != wow) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        HDC hdc = GetDC(NULL); 
        if (hdc) {
            // Читаем команду из пикселя (1,1)
            COLORREF color = GetPixel(hdc, 1, 1);
            ReleaseDC(NULL, hdc);

            // Если цвет совпал с биндом — жмём
            if (g_binds.count(color)) {
                PressKey(g_binds[color]);
                // Задержка, чтобы аддон успел сменить цвет в игре
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

extern "C" {
    __declspec(dllexport) BOOL __stdcall FindWoWWindow() {
        HWND h = FindWindowA("GxWindowClass", nullptr);
        g_hwnd.store(h);
        return (h != nullptr);
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

    __declspec(dllexport) BOOL __stdcall IsRunning() {
        return g_running.load();
    }
}

BOOL WINAPI DllMain(HMODULE h, DWORD r, LPVOID l) { return TRUE; }
