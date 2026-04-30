#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef BOOL (__stdcall *FnFind)();
typedef void (__stdcall *FnBind)(int, int, int, BYTE);
typedef BOOL (__stdcall *FnStart)();
typedef void (__stdcall *FnStop)();

int main() {
    SetConsoleOutputCP(1251);
    printf("--- Combat Rogue Loader v5.5 (Ultimate Attach) ---\n");

    HINSTANCE hDll = LoadLibraryA("roguerot.dll");
    if (!hDll) { printf("Ошибка: DLL не найдена!\n"); system("pause"); return 1; }

    auto FindWoW = (FnFind)GetProcAddress(hDll, "FindWoWWindow");
    auto Bind    = (FnBind)GetProcAddress(hDll, "BindColor");
    auto Start   = (FnStart)GetProcAddress(hDll, "StartRotation");
    auto Stop    = (FnStop)GetProcAddress(hDll, "StopRotation");

    if (!FindWoW || !Bind || !Start || !Stop) {
        printf("Ошибка: DLL старой версии!\n");
        system("pause"); return 1;
    }

    Bind(255, 0, 0,   '1');             // Sinister Strike
    Bind(0, 255, 0,   '2');             // Slice and Dice
    Bind(0, 0, 255,   'Q');             // Eviscerate
    Bind(255, 255, 0, '4');             // Killing Spree
    Bind(0, 255, 255, VK_XBUTTON1);     // Adrenaline Rush
    Bind(255, 0, 255, '3');             // Blade Flurry

    printf("\nИНСТРУКЦИЯ (ОБЯЗАТЕЛЬНО ПРОЧТИ):\n");
    printf("1. Зайди в игру WoW.\n");
    printf("2. Кликни мышкой по игре, чтобы она была активна.\n");
    printf("3. Прямо в игре нажми F9. Бот прицепится к активному окну.\n\n");
    printf("F9 - Старт/Стоп, F10 - Выход\n");

    bool run = false;
    while (!(GetAsyncKeyState(VK_F10) & 1)) {
        if (GetAsyncKeyState(VK_F9) & 1) {
            run = !run;
            if (run) {
                if (FindWoW()) { 
                    Start(); 
                    printf("[УСПЕХ] Бот прицепился к окну! Работаем...\n"); 
                }
                else { 
                    printf("[ОШИБКА] Кликни по окну ИГРЫ перед тем как жать F9!\n"); 
                    run = false; 
                }
            } else { 
                Stop(); 
                printf("[ПАУЗА] Остановлено.\n"); 
            }
        }
        Sleep(100);
    }
    Stop();
    return 0;
}
