#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>
#include <atomic>
#include <map>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdio.h>

static std::atomic<bool> g_running(false);
static std::atomic<HWND> g_hwnd(nullptr);
static std::thread       g_thread;
static std::map<COLORREF, BYTE> g_binds;

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    char title[256];
    if (GetWindowTextA(hwnd, title, sizeof(title)) > 0) {
        std::string t = toLower(title);
        if (t.find("sirus") != std::string::npos || 
            t.find("warcraft") != std::string::npos || 
            t.find("warmane") != std::string::npos || 
            t.find("wow") != std::string::npos || 
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

    // Если это боковые кнопки мыши, используем специфические оконные сообщения
    if (vk == VK_XBUTTON1 || vk == VK_XBUTTON2) {
        WORD btn = (vk == VK_XBUTTON1) ? XBUTTON1 : XBUTTON2;
        WPARAM wParam = MAKEWPARAM(0, btn);
        // Отправляем сигнал о нажатии и отпускании доп. кнопки мыши
        PostMessage(wow, WM_XBUTTONDOWN, wParam, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        PostMessage(wow, WM_XBUTTONUP, wParam, 0);
    } 
    // Для всех остальных кнопок используем стандартное нажатие клавиши
    else {
        PostMessage(wow, WM_KEYDOWN, vk, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        PostMessage(wow, WM_KEYUP, vk, 0);
    }
}

// Функция для проверки цвета с учетом искажений рендера WoW
bool IsColorMatch(COLORREF c1, COLORREF c2, int tolerance = 45) {
    int r = std::abs((int)GetRValue(c1) - (int)GetRValue(c2));
    int g = std::abs((int)GetGValue(c1) - (int)GetGValue(c2));
    int b = std::abs((int)GetBValue(c1) - (int)GetBValue(c2));
    return (r <= tolerance && g <= tolerance && b <= tolerance);
}

void RotationLoop() {
    while (g_running.load()) {
        HWND wow = g_hwnd.load();
        if (!wow || !IsWindow(wow) || GetForegroundWindow() != wow) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        // БЕРЕМ КОНТЕКСТ ИМЕННО ОКНА WOW, А НЕ ВСЕГО ЭКРАНА
        HDC hdc = GetDC(wow); 
        if (hdc) {
            // Читаем самый крайний пиксель клиентской части окна (0,0)
            COLORREF color = GetPixel(hdc, 0, 0);
            ReleaseDC(wow, hdc);

            // Если пиксель практически черный (ничего не нужно жать), пропускаем
            if (GetRValue(color) < 20 && GetGValue(color) < 20 && GetBValue(color) < 20) {
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
                continue;
            }

            // Перебираем все наши бинды и ищем похожий цвет
            for (const auto& bind : g_binds) {
                if (IsColorMatch(color, bind.first)) {
                    
                    // --- ЛОГИРОВАНИЕ В КОНСОЛЬ ---
                    printf("[LOG] Цвет экрана: R:%3d G:%3d B:%3d | Жму кнопку: ", 
                           GetRValue(color), GetGValue(color), GetBValue(color));
                    
                    if (bind.second == VK_XBUTTON1) printf("Mouse4\n");
                    else printf("%c\n", bind.second);
                    
                    fflush(stdout); // Принудительно выводим текст
                    // -----------------------------

                    PressKey(bind.second);
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    break; // Нажали кнопку, ждем следующий тик
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
}

extern "C" {
    __declspec(dllexport) BOOL FindWoWWindow() {
        HWND foundHwnd = nullptr;
        foundHwnd = FindWindowA("GxWindowClass", nullptr);
        
        if (!foundHwnd) {
            EnumWindows(EnumWindowsProc, (LPARAM)&foundHwnd);
        }
        
        if (!foundHwnd) {
            HWND active = GetForegroundWindow();
            char title[256] = {0};
            GetWindowTextA(active, title, sizeof(title));
            std::string t = toLower(title);
            if (t.find("loader") == std::string::npos && t.find("cmd") == std::string::npos) {
                foundHwnd = active;
            }
        }
        
        g_hwnd.store(foundHwnd);
        return (foundHwnd != nullptr);
    }

    __declspec(dllexport) void BindColor(int r, int g, int b, BYTE vk) {
        g_binds[RGB(r, g, b)] = vk;
    }

    __declspec(dllexport) BOOL StartRotation() {
        if (g_running.load()) return FALSE;
        g_running.store(true);
        g_thread = std::thread(RotationLoop);
        return TRUE;
    }

    __declspec(dllexport) void StopRotation() {
        g_running.store(false);
        if (g_thread.joinable()) g_thread.join();
    }

    __declspec(dllexport) BOOL IsRunning() { return g_running.load(); }
}

BOOL WINAPI DllMain(HMODULE h, DWORD r, LPVOID l) { return TRUE; }
