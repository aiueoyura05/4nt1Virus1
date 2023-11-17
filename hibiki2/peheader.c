#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <stdlib.h>

int main(){
    //1,ファイル読み込み
    BYTE buf[10000];
    FILE *ph;
    ph = fopen("C:/Users/throot/Desktop/hibiki2/4nt1Virus1/hibiki2/hello.exe", "rb");
    
    if(ph == NULL){
        printf("File not found\n");
        return 0;
    }
    if (fread(buf,1,9999,ph)){
       buf[1000] = '\0'; 
    }
    //
    for (int i = 0; i < 10000; i++) {
        printf("%02X ", buf[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    //ファイル読み込み時のエラー処理
    
    //2,DOSheaderの読み込み(NTheaderへのオフセットの取得)
    IMAGE_DOS_HEADER dosHeader;
    fseek(ph, 0, SEEK_SET);
    if (fread(&dosHeader, sizeof(IMAGE_DOS_HEADER), 1, ph) != 1) {
        fprintf(stderr, "Couldn't read DOS header\n");
        return 1;
    }
    printf("e_lfanew: 0x%X\n", dosHeader.e_lfanew);
    
    //3,NT_HEADERへ
    IMAGE_NT_HEADERS ntheader;
    IMAGE_NT_HEADERS64 ntheader64;
    
    fseek(ph,dosHeader.e_lfanew,SEEK_SET);
    
    if (fread(&ntheader, sizeof(IMAGE_NT_HEADERS64), 1, ph) != 1){
        fprintf(stderr, "Couldn't read NT header\n");
        return 1;
    }
    printf("2");
    printf("signature: 0x%X\n", ntheader.OptionalHeader);
    printf("signature: 0x%X\n", ntheader64.OptionalHeader);

    //NTheaderからオプショナルヘッダに
   
    

    //ファイルclose処理
    fclose(ph);
    return 0;
}
