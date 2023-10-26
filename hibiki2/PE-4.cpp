#include <iostream>
#include <Windows.h>
#include <string>

bool is_x32 = true;


//メモリーアドレスをファイルアドレスに返す
template <typename s>
size_t RvaToFileOffset(uint32_t rva, s* nt_header) {
    IMAGE_SECTION_HEADER* section_header = IMAGE_FIRST_SECTION(nt_header);

    for (size_t i = 0; i < nt_header->FileHeader.NumberOfSections; i++)
    {
        if (rva >= section_header->VirtualAddress && rva < section_header->VirtualAddress + section_header->Misc.VirtualSize) {
            //std::cout << "特定しました、" << section_header->Name << "です！" << std::endl;
            return rva - section_header->VirtualAddress + section_header->PointerToRawData;
        }
        section_header++;
    }

    return NULL;
}

template <typename T , typename M>
void processThunk(T* thunk, void* base_address, M* nt_header) {
    for (; thunk->u1.AddressOfData != 0; thunk++) {
        IMAGE_IMPORT_BY_NAME* import_data = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(reinterpret_cast<char*>(base_address) + RvaToFileOffset(thunk->u1.AddressOfData, nt_header));
        std::cout << "->" << import_data->Name << std::endl;
    }
}

template <typename G>
void printf_import(G* nt_header, LPVOID base_addres)
{
    IMAGE_SECTION_HEADER* section_header = IMAGE_FIRST_SECTION(nt_header);

    for (size_t i = 0; i < nt_header->FileHeader.NumberOfSections; i++)
    {
        std::cout << section_header->Name << std::endl;
        section_header++;
    }

    std::cout << nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress << std::endl;

    IMAGE_IMPORT_DESCRIPTOR* import_descriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
        reinterpret_cast<char*>(base_addres) + RvaToFileOffset(nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, nt_header)
        );


    for (; import_descriptor->Characteristics != 0; import_descriptor++)
    {
        std::cout << "DLL: " << reinterpret_cast<char*>(base_addres) + RvaToFileOffset(import_descriptor->Name, nt_header) << std::endl;
        IMAGE_THUNK_DATA* thunk = nullptr;

        if (is_x32) {
            IMAGE_THUNK_DATA32* thunk = reinterpret_cast<IMAGE_THUNK_DATA32*>(reinterpret_cast<char*>(base_addres) + RvaToFileOffset(import_descriptor->OriginalFirstThunk, nt_header));
            processThunk(thunk, base_addres, nt_header);
        }
        else {
            IMAGE_THUNK_DATA64* thunk = reinterpret_cast<IMAGE_THUNK_DATA64*>(reinterpret_cast<char*>(base_addres) + RvaToFileOffset(import_descriptor->OriginalFirstThunk, nt_header));
            processThunk(thunk, base_addres, nt_header);
        }

    }
}

int main()
{
    HANDLE file_handle = CreateFileA("C:\Users\tsukky\Desktop\hibiki2\hello.exe", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (file_handle == INVALID_HANDLE_VALUE) {
        std::cout << "ファイルハンドル取得失敗" << std::endl;
        exit(0);
    }
    HANDLE mapping_handle = CreateFileMappingA(file_handle, NULL, PAGE_READONLY, 0, 0, NULL);

    if (mapping_handle == INVALID_HANDLE_VALUE) {
        std::cout << "ファイルマッピング失敗" << std::endl;
        exit(0);
    }

    LPVOID base_addres = MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, 0);
    if (base_addres == NULL) {
        std::cout << "ファイルビュー失敗" << std::endl;
        exit(0);
    }

    char* mz_header = reinterpret_cast<char*>(base_addres);
    if (mz_header[0] != 'M' || mz_header[1] != 'Z') {
        std::cout << "PE形式のみ対応しています\n";
        exit(0);
    }

    _IMAGE_DOS_HEADER* dos_header = reinterpret_cast<_IMAGE_DOS_HEADER*>(base_addres); //baseアドレスが_IMAGE_DOS_HEADER型に強制変換
    //ここ問題！！！！
    IMAGE_NT_HEADERS32* nt_header = reinterpret_cast<IMAGE_NT_HEADERS32*>(reinterpret_cast<char*>(base_addres) + dos_header->e_lfanew); //e_lfanewにはnt_headerのoffsetが保持されているので(dos_header + e_lfanewこれで実際のメモリーアドレスを取得できます)、NTheaderに型変換します。

    if (nt_header->FileHeader.Machine == 0x14c) {
        std::cout << "oh is x32!" << std::endl;
        printf_import(nt_header,base_addres);
    }
    else if (nt_header->FileHeader.Machine == 0x8664) {
        std::cout << "is x64!" << std::endl;
        is_x32 = false;
        IMAGE_NT_HEADERS64* nt_header = reinterpret_cast<IMAGE_NT_HEADERS64*>(reinterpret_cast<char*>(base_addres) + dos_header->e_lfanew);//x64版に再構築
        printf_import(nt_header, base_addres);
    }

    return 0;
}