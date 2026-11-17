#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

int main(int argc, char* argv[]) {
    // Numele pentru memoria partajata si semafor in Windows
    const char* shmName = "Local\\MemorieComunaLab6";
    const char* semName = "Local\\SemaforLab6";

    HANDLE hMapFile;
    int* mem;
    HANDLE hSemaphore;

    // SUNTEM IN PROCESUL COPIL (daca programul e rulat cu argumente in linia de comanda)
    if (argc > 1) {
        hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shmName);
        mem = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
        hSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, semName);
        
        srand(time(NULL) ^ GetCurrentProcessId());

        while (true) {
            WaitForSingleObject(hSemaphore, INFINITE); // REZERVAM SEMAFORUL
            
            if (*mem >= 1000) {
                ReleaseSemaphore(hSemaphore, 1, NULL);
                break;
            }
            
            int coin = rand() % 2 + 1;
            while (coin == 2 && *mem < 1000) {
                (*mem)++;
                cout << "[Proces 2] Banul a picat 2! Scrie in memorie: " << *mem << endl;
                coin = rand() % 2 + 1;
            }
            
            ReleaseSemaphore(hSemaphore, 1, NULL); // ELIBERAM SEMAFORUL
            Sleep(1);
        }
        
        UnmapViewOfFile(mem);
        CloseHandle(hMapFile);
        CloseHandle(hSemaphore);
        return 0;
    }

    // SUNTEM IN PROCESUL PARINTE
    hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int), shmName);
    mem = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
    *mem = 1;

    hSemaphore = CreateSemaphoreA(NULL, 1, 1, semName);

    // Cream procesul copil
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    string cmd = string(argv[0]) + " copil";
    char cmdLine[256];
    strcpy(cmdLine, cmd.c_str());

    CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    srand(time(NULL) ^ GetCurrentProcessId());

    while (true) {
        WaitForSingleObject(hSemaphore, INFINITE); // REZERVAM SEMAFORUL
        
        if (*mem >= 1000) {
            ReleaseSemaphore(hSemaphore, 1, NULL);
            break;
        }
        
        int coin = rand() % 2 + 1;
        while (coin == 2 && *mem < 1000) {
            (*mem)++;
            cout << "[Proces 1] Banul a picat 2! Scrie in memorie: " << *mem << endl;
            coin = rand() % 2 + 1;
        }
        
        ReleaseSemaphore(hSemaphore, 1, NULL); // ELIBERAM SEMAFORUL
        Sleep(1);
    }

    WaitForSingleObject(pi.hProcess, INFINITE); // Asteptam sa termine si copilul
    
    cout << "\nGata! Ambele procese au ajuns la 1000 folosind memoria partajata (Windows)." << endl;

    UnmapViewOfFile(mem);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}