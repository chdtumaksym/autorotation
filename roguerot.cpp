#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>
#include <atomic>
#include <string>
#include <map>
#include <vector>

static std::atomic<bool> g_running(false);
static std::atomic<HWND> g_hwnd(nullptr);
static std::thread       g_thread;
static int               g_delay = 50;

struct AbilityColor {
    COLORREF color;
    std::string name;
};

static std::vector<AbilityColor> g_scheme = {
    { RGB(255, 0, 0),   "sinister_strike" },
    { RGB(0, 255, 0),   "slice_and_dice"  },
    { RGB(0, 0, 255),   "eviscerate"      },
    { RGB(255, 255, 0), "adrenaline_rush" },
    { RGB(255, 0, 255), "blade_flurry"    },
    { RGB(0, 255, 255), "killing_spree"   }
};

static std::map<std::string, BYTE> g_keyMap;

static void PressKey(BYTE vk) {
    HWND wow = g_hwnd.load();
    if (!wow) return;
    PostMessage(wow, WM_KEYDOWN, vk, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
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
            COLORREF color = GetPixel(hdc, 1, 1);
            ReleaseDC(NULL, hdc);
            int r = GetRValue(color);
            int g = GetGValue(color);
            int b = GetBValue(color);
            for (const auto& item : g_scheme) {
                if (abs(r - GetRValue(item.color)) < 15 &&
                    abs(g - GetGValue(item.color)) < 15 &&
                    abs(b - GetBValue(item.color)) < 15) 
                {
                    if (g_keyMap.count(item.name)) {
                        PressKey(g_keyMap[item.name]);
                        std::this_thread::sleep_for(std::chrono::milliseconds(g_delay + 80));
                        break;
                    }
                }
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
    __declspec(dllexport) void __stdcall SetKey(const char* name, BYTE vk) {
        g_keyMap[name] = vk;
    }
    __declspec(dllexport) void __stdcall SetDelay(int ms) {
        g_delay = ms;
    }
}
BOOL WINAPI DllMain(HMODULE h, DWORD r, LPVOID l) { return TRUE; }
