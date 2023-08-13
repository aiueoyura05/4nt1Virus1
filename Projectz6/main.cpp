#include <stdio.h>
#include <string.h>
#include <windows.h>


void RoadToExe() {

}

// ç\ë¢ëÃÇÃíËã`
typedef struct InjectData {

	wchar_t path[256]

}InjectData;

void LoadDll() {
	
	InjectData Data;
	wcscpy(Data.path,L"C:\\Users\\inayu\\source\\repos\\Projectz6\\x64\\Debug\\z6dll.dll");

	HMODULE Hkernel32 = LoadLibraryA("kernel32.dll");
	LPVOID load_library_addr = GetProcAddress( Hkernel32 ,"LoadLibrary");


	CreateProcessW(L" C:\\Users\\inayu\\source\\repos\\Projectz6\\x64\\Debug\\", );

	VirtualAllocEx("");

	WriteProcessMemory(, , );

	CreateRemoteThread();

	CloseHandle();

	ExitProcess();


	FreeLibrary(hmodule);

}//ã@äBåÍÇ…ïœä∑

int main() {

	FreeLibrary(hmodule);

}