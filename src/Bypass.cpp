#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>

uintptr_t MadiumBase = 0;
uintptr_t MadiumSize = 0;

uintptr_t OrigPresent = 0;
uintptr_t HookPresent = 0;
uintptr_t OrigResize = 0;
uintptr_t HookResize = 0;

// Проверка, откуда был вызван оригинальный метод
bool IsInsideMadium(uintptr_t addr) {
    return (addr >= MadiumBase && addr < (MadiumBase + MadiumSize));
}

// Обработчик исключений VEH
LONG CALLBACK VehHandler(PEXCEPTION_POINTERS ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
        
        // Обработка Trap Flag (после того как мы шагнули 1 инструкцию оригинала)
        if (ExceptionInfo->ContextRecord->Dr6 & 0x4000) { 
            // Возвращаем HWBP обратно
            ExceptionInfo->ContextRecord->Dr0 = OrigPresent;
            ExceptionInfo->ContextRecord->Dr1 = OrigResize;
            ExceptionInfo->ContextRecord->EFlags &= ~0x100; // Очищаем Trap Flag
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        // Сработал HWBP на Present (Dr0)
        if (ExceptionInfo->ContextRecord->Rip == OrigPresent) {
            uintptr_t retAddr = *(uintptr_t*)(ExceptionInfo->ContextRecord->Rsp);
            
            if (IsInsideMadium(retAddr)) {
                // Если вызвал сам Madium (из HookPresent) -> Разрешаем выполнить оригинал.
                // Временно отключаем Dr0 и ставим Trap Flag, чтобы перехватить управление на следующей инструкции
                ExceptionInfo->ContextRecord->Dr0 = 0; 
                ExceptionInfo->ContextRecord->EFlags |= 0x100; 
                return EXCEPTION_CONTINUE_EXECUTION;
            } else {
                // Вызвал движок игры -> Перенаправляем на хук Madium
                ExceptionInfo->ContextRecord->Rip = HookPresent;
                return EXCEPTION_CONTINUE_EXECUTION;
            }
        }
        
        // Сработал HWBP на ResizeBuffers (Dr1)
        if (ExceptionInfo->ContextRecord->Rip == OrigResize) {
            uintptr_t retAddr = *(uintptr_t*)(ExceptionInfo->ContextRecord->Rsp);
            
            if (IsInsideMadium(retAddr)) {
                ExceptionInfo->ContextRecord->Dr1 = 0;
                ExceptionInfo->ContextRecord->EFlags |= 0x100;
                return EXCEPTION_CONTINUE_EXECUTION;
            } else {
                ExceptionInfo->ContextRecord->Rip = HookResize;
                return EXCEPTION_CONTINUE_EXECUTION;
            }
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// Установка Hardware Breakpoints на все потоки игры
void SetHWBP(DWORD pid) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(hSnap, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (hThread) {
                    SuspendThread(hThread);
                    CONTEXT ctx = { 0 };
                    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
                    GetThreadContext(hThread, &ctx);
                    
                    ctx.Dr0 = OrigPresent;
                    ctx.Dr1 = OrigResize;
                    ctx.Dr7 = 0x405; // Включаем L0 (для Dr0) и L1 (для Dr1)
                    
                    SetThreadContext(hThread, &ctx);
                    ResumeThread(hThread);
                    CloseHandle(hThread);
                }
            }
        } while (Thread32Next(hSnap, &te));
    }
    CloseHandle(hSnap);
}

// Фоновый поток инициализации
DWORD WINAPI InitBypass(LPVOID) {
    HMODULE hMadium = nullptr;
    // Ждем, пока Madium.exe заинжектит пропатченный Madium-Module.dll
    while (!(hMadium = GetModuleHandleA("Madium-Module.dll"))) {
        Sleep(100);
    }

    MadiumBase = (uintptr_t)hMadium;
    
    // Получаем размер модуля Madium
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)MadiumBase;
    PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(MadiumBase + pDos->e_lfanew);
    MadiumSize = pNt->OptionalHeader.SizeOfImage;

    // Ждем, пока Madium запишет оригинальные адреса Present и ResizeBuffers в секцию .data
    uintptr_t* pOrigPresent = (uintptr_t*)(MadiumBase + 0x7821a8);
    while (*pOrigPresent == 0) {
        Sleep(100);
    }

    // Считываем адреса
    OrigPresent = *pOrigPresent;
    OrigResize = *(uintptr_t*)(MadiumBase + 0x7821d8);
    HookPresent = MadiumBase + 0x79ec0;
    HookResize = MadiumBase + 0x7c330;

    // Регистрируем обработчик VEH и вешаем бряки
    AddVectoredExceptionHandler(1, VehHandler);
    SetHWBP(GetCurrentProcessId());
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CloseHandle(CreateThread(nullptr, 0, InitBypass, nullptr, 0, nullptr));
    }
    return TRUE;
}
