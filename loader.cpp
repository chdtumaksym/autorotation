// loader.cpp
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

typedef BOOL (__stdcall *FnFind)();
typedef void (__stdcall *FnBind)(int, int, int, BYTE);
typedef BOOL (__stdcall *FnStart)();
typedef void (__stdcall *FnStop)();

int main() {
    SetConsoleOutputCP(1251);
    printf("--- Combat Rogue Loader v5 (Pixel) ---\n");

    HINSTANCE hDll = LoadLibraryA("roguerot.dll");
    if (!hDll) { printf("Ошибка: DLL не найдена!\n"); system("pause"); return 1; }

    auto FindWoW = (FnFind)GetProcAddress(hDll, "FindWoWWindow");
    auto Bind    = (FnBind)GetProcAddress(hDll, "BindColor");
    auto Start   = (FnStart)GetProcAddress(hDll, "StartRotation");
    auto Stop    = (FnStop)GetProcAddress(hDll, "StopRotation");

    if (!FindWoW || !Bind || !Start || !Stop) {
        printf("Ошибка: DLL старой версии! Удали хлам и скачай v5.\n");
        system("pause"); return 1;
    }

    Bind(255, 0, 0,   '1');             // Sinister Strike (1)
    Bind(0, 255, 0,   '2');             // Slice and Dice (2)
    Bind(0, 0, 255,   'Q');             // Eviscerate (Q)
    Bind(255, 255, 0, '4');             // Killing Spree (4)
    Bind(0, 255, 255, VK_XBUTTON1);     // Adrenaline Rush (Mouse 4)
    Bind(255, 0, 255, '3');             // Blade Flurry (3)

    printf("Настроено: SS=1, SnD=2, Evis=Q, BF=3, KS=4, AR=M4\n");
    printf("F9 - Старт, F10 - Выход\n");

    bool run = false;
    while (!(GetAsyncKeyState(VK_F10) & 1)) {
        if (GetAsyncKeyState(VK_F9) & 1) {
            run = !run;
            if (run) {
                if (FindWoW()) { Start(); printf("РАБОТАЕТ\n"); }
                else { printf("WoW не найден!\n"); run = false; }
            } else { Stop(); printf("СТОП\n"); }
        }
        Sleep(100);
    }
    Stop();
    return 0;
}
