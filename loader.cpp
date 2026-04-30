#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h> // <-- Обязательно для system()

typedef BOOL  (__stdcall *FnFindWoW)();
typedef BOOL  (__stdcall *FnStart)();
typedef void  (__stdcall *FnStop)();
typedef BOOL  (__stdcall *FnIsRunning)();

int main() {
    SetConsoleOutputCP(1251);
    printf("=== Combat Rogue Pixel Reader (Zero Delay) ===\n");
    printf("F9 - Старт/Стоп\nF10 - Выход\n");

    HMODULE dll = LoadLibraryA("roguerot.dll");
    if (!dll) {
        printf("ОШИБКА: roguerot.dll не найдена рядом с exe!\n");
        system("pause");
        return 1;
    }

    auto FindWoW   = (FnFindWoW)   GetProcAddress(dll, "FindWoW");
    auto Start     = (FnStart)     GetProcAddress(dll, "StartRotation");
    auto Stop      = (FnStop)      GetProcAddress(dll, "StopRotation");
    auto IsRunning = (FnIsRunning) GetProcAddress(dll, "IsRunning");

    if (!FindWoW || !Start || !Stop || !IsRunning) {
        printf("ОШИБКА: Загружена старая версия roguerot.dll! Обнови файл.\n");
        system("pause");
        return 1;
    }

    if (FindWoW()) printf("WoW найден. Жми F9 для старта.\n");
    else printf("WoW не найден. Запусти игру и жми F9.\n");

    while (true) {
        if (GetAsyncKeyState(VK_F9) & 1) {
            if (!IsRunning()) {
                if (FindWoW() && Start()) printf("[СТАРТ] Читаю пиксели. Лог пишется в RogueBot_Log.txt\n");
            } else {
                Stop();
                printf("[СТОП] Ротация остановлена.\n");
            }
        }
        if (GetAsyncKeyState(VK_F10) & 1) {
            if (IsRunning()) Stop();
            break;
        }
        Sleep(50);
    }
    FreeLibrary(dll);
    return 0;
}
