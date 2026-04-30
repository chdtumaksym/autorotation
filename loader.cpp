#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

// Определяем типы функций из DLL v5
typedef BOOL (__stdcall *FnFind)();
typedef void (__stdcall *FnBind)(int, int, int, BYTE);
typedef BOOL (__stdcall *FnStart)();
typedef void (__stdcall *FnStop)();
typedef BOOL (__stdcall *FnIsRunning)();

int main() {
    // Установка кодировки для вывода в консоль
    SetConsoleOutputCP(1251);
    
    printf("===========================================\n");
    printf("  Combat Rogue Pixel Loader v5 (Зрячий)  \n");
    printf("===========================================\n");

    HINSTANCE hDll = LoadLibraryA("roguerot.dll");
    if (!hDll) { 
        printf("[ОШИБКА] Не удалось загрузить roguerot.dll!\n");
        printf("Убедись, что DLL лежит в той же папке, что и этот EXE.\n");
        system("pause");
        return 1; 
    }

    auto FindWoW = (FnFind)GetProcAddress(hDll, "FindWoWWindow");
    auto Bind    = (FnBind)GetProcAddress(hDll, "BindColor");
    auto Start   = (FnStart)GetProcAddress(hDll, "StartRotation");
    auto Stop    = (FnStop)GetProcAddress(hDll, "StopRotation");
    auto IsRunning = (FnIsRunning)GetProcAddress(hDll, "IsRunning");

    if (!FindWoW || !Bind || !Start || !Stop) {
        printf("[ОШИБКА] DLL не подходит! Ты используешь старую версию.\n");
        printf("Пересобери roguerot.cpp на GitHub (версия v5).\n");
        system("pause");
        return 1;
    }

    // --- НАСТРОЙКА БИНДОВ (Цвет в аддоне -> Кнопка в игре) ---
    // Формат: Bind(R, G, B, Клавиша)
    Bind(255, 0, 0,   '1');            // Красный -> Sinister Strike
    Bind(0, 255, 0,   '2');            // Зеленый -> Slice and Dice
    Bind(0, 0, 255,   'Q');            // Синий   -> Eviscerate (Твой выбор!)
    Bind(255, 255, 0, '4');            // Желтый  -> Killing Spree (Твой выбор!)
    Bind(0, 255, 255, VK_XBUTTON1);    // Циан    -> Adrenaline Rush (Mouse 4)
    Bind(255, 0, 255, '3');            // Розовый -> Blade Flurry (Твой выбор!)

    printf("\n[ГОТОВО] Бинды загружены в DLL.\n");
    printf("Sinister: 1 | SnD: 2 | Eviscerate: Q\n");
    printf("Blade Flurry: 3 | Killing Spree: 4 | Adrenaline: Mouse4\n");
    printf("-------------------------------------------\n");
    printf("УПРАВЛЕНИЕ:\n");
    printf("  F9  - Запуск / Пауза\n");
    printf("  F10 - Полный выход\n");
    printf("-------------------------------------------\n");

    bool active = false;
    while (!(GetAsyncKeyState(VK_F10) & 1)) {
        if (GetAsyncKeyState(VK_F9) & 1) {
            active = !active;
            if (active) {
                if (FindWoW()) { 
                    Start(); 
                    printf("[СТАТУС] Работаем. Бот видит пиксель.\n"); 
                } else { 
                    printf("[ОШИБКА] Окно WoW не найдено!\n"); 
                    active = false; 
                }
            } else { 
                Stop(); 
                printf("[СТАТУС] Пауза.\n"); 
            }
        }
        Sleep(100);
    }

    Stop();
    FreeLibrary(hDll);
    return 0;
}
