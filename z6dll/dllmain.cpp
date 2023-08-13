/*#include <Windows.h>

BOOL WINAPI DLLMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {
	return TRUE;
}
*/
#include <windows.h>
#include <detours.h>

static LONG dwSlept = 0;

// Target pointer for the uninstrumented Sleep API.
//
static VOID(WINAPI* TrueSleep)(DWORD dwMilliseconds) = Sleep;
typedef BOOL (WINAPI* OrigCreateProcessW)(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);
typedef HANDLE (WINAPI* OrigCreateRemoteThread)(
    HANDLE                 hProcess,
    LPSECURITY_ATTRIBUTES  lpThreadAttributes,
    SIZE_T                 dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID                 lpParameter,
    DWORD                  dwCreationFlags,
    LPDWORD                lpThreadId
);
typedef BOOL (WINAPI* OrigWriteProcessMemory)(
    HANDLE  hProcess,
    LPVOID  lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T  nSize,
    SIZE_T* lpNumberOfBytesWritten
);
typedef LPVOID (WINAPI* OrigVirtualAllocEx)(
    HANDLE hProcess,
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD  flAllocationType,
    DWORD  flProtect
);

PVOID origCreateProcessW     = GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateProcessW");
PVOID origCreateRemoteThread = GetProcAddress(GetModuleHandleA("kernel32.dll"), "CreateRemoteThread");
PVOID origWriteProcessMemory = GetProcAddress(GetModuleHandleA("kernel32.dll"), "WriteProessMemory");
PVOID origVirtualAllocEx     = GetProcAddress(GetModuleHandleA("kernel32.dll"), "VirtualAllocEx");

HANDLE HTargetProcess = INVALID_HANDLE_VALUE;
BOOL FLAG_CreateProcess = 0;
BOOL FLAG_VirtualAllocEx = 0;
BOOL FLAG_WriteProcessmemory = 0;
BOOL FLAG_CreateRemoteThread = 0;


BOOL HookCreateProcessW(
    LPCWSTR               lpApplicationName,
    LPWSTR                lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL                  bInheritHandles,
    DWORD                 dwCreationFlags,
    LPVOID                lpEnvironment,
    LPCWSTR               lpCurrentDirectory,
    LPSTARTUPINFOW        lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) {
    FLAG_CreateProcess = 1;

    BOOL result = ((OrigCreateProcessW)origCreateProcessW)(
        lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation
    );
    HTargetProcess = lpProcessInformation->hProcess;
    return result;
}

HANDLE HookCreateRemoteThread(
    HANDLE                 hProcess,
    LPSECURITY_ATTRIBUTES  lpThreadAttributes,
    SIZE_T                 dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID                 lpParameter,
    DWORD                  dwCreationFlags,
    LPDWORD                lpThreadId
) {
    if (HTargetProcess == hProcess&&FLAG_CreateProcess && FLAG_VirtualAllocEx && FLAG_WriteProcessmemory && FLAG_CreateRemoteThread) {
        OutputDebugStringW(L"Detected");
        ExitProcess(-1);
    }

    return ((OrigCreateRemoteThread)origCreateRemoteThread)(
        hProcess,
        lpThreadAttributes,
        dwStackSize,
        lpStartAddress,
        lpParameter,
        dwCreationFlags,
        lpThreadId
        );

}

BOOL HookWriteProcessMemory(
    HANDLE  hProcess,
    LPVOID  lpBaseAddress,
    LPCVOID lpBuffer,
    SIZE_T  nSize,
    SIZE_T* lpNumberOfBytesWritten
) {
    if (HTargetProcess == hProcess) {
        FLAG_WriteProcessmemory = 1;
    }
    return ((OrigWriteProcessMemory)origWriteProcessMemory)(
        hProcess,
        lpBaseAddress,
        lpBuffer,
        nSize,
        lpNumberOfBytesWritten
        );
}

LPVOID HookVirtualAllocEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    SIZE_T dwSize,
    DWORD  flAllocationType,
    DWORD  flProtect
) {
    if (HTargetProcess == hProcess) {
        FLAG_VirtualAllocEx = 1;
    }
    return ((OrigVirtualAllocEx)origVirtualAllocEx)(
        hProcess,
        lpAddress,
        dwSize,
        flAllocationType,
        flProtect
        );
}




// Detour function that replaces the Sleep API.
//
VOID WINAPI TimedSleep(DWORD dwMilliseconds)
{
    // Save the before and after times around calling the Sleep API.
    DWORD dwBeg = GetTickCount();
    TrueSleep(dwMilliseconds);
    DWORD dwEnd = GetTickCount();

    InterlockedExchangeAdd(&dwSlept, dwEnd - dwBeg);
}

// DllMain function attaches and detaches the TimedSleep detour to the
// Sleep target function.  The Sleep target function is referred to
// through the TrueSleep target pointer.
//
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        // DetourAttach(&(PVOID&)TrueSleep, TimedSleep);
        DetourAttach(&origCreateProcessW,     (PVOID)HookCreateProcessW);
        DetourAttach(&origCreateRemoteThread, (PVOID)HookCreateRemoteThread);
        DetourAttach(&origWriteProcessMemory, (PVOID)HookWriteProcessMemory);
        DetourAttach(&origVirtualAllocEx,     (PVOID)HookVirtualAllocEx);
        
        DetourTransactionCommit();
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueSleep, TimedSleep);
        DetourTransactionCommit();
    }
    return TRUE;
}
