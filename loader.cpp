#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h> 

typedef BOOL (*FnFindWoW)();
typedef BOOL (*FnStart)();
typedef void (*FnStop)();
typedef BOOL (*FnIsRunning)();

int main() {
    // Включаем UTF-8 для адекватного отображения текста
    SetConsoleOutputCP(CP_UTF8);
    printf("=== Combat Rogue Pixel Reader (Zero Delay) ===\n");
    printf("F9 - Старт/Стоп\nF10 - Выход\n");

    HMODULE dll = LoadLibraryA("roguerot.dll");
    if (!dll) {
        printf("ОШИБКА: roguerot.dll не найдена рядом с exe!\n");
        system("pause");
        return 1;
    }

    // Защита от MSVC Name Mangling (ищем оба варианта)
    auto FindWoW   = (FnFindWoW)GetProcAddress(dll, "FindWoW");
    if (!FindWoW) FindWoW = (FnFindWoW)GetProcAddress(dll, "_FindWoW");

    auto Start     = (FnStart)GetProcAddress(dll, "StartRotation");
    if (!Start) Start = (FnStart)GetProcAddress(dll, "_StartRotation");

    auto Stop      = (FnStop)GetProcAddress(dll, "StopRotation");
    if (!Stop) Stop = (FnStop)GetProcAddress(dll, "_StopRotation");

    auto IsRunning = (FnIsRunning)GetProcAddress(dll, "IsRunning");
    if (!IsRunning) IsRunning = (FnIsRunning)GetProcAddress(dll, "_IsRunning");

    if (!FindWoW || !Start || !Stop || !IsRunning) {
        printf("ОШИБКА: Экспорты не найдены! Ты точно скомпилировал НОВУЮ roguerot.dll?\n");
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
