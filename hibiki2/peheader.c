#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <stdlib.h>


IMAGE_DOS_HEADER dosHeader;
    if (fread(&dosHeader, sizeof(IMAGE_DOS_HEADER), 1, file) != 1) {
        fprintf(stderr, "DOSヘッダを読み込めませんでした。\n");
        fclose(file);
        return 1;
    }
    printf("e_lfanew: 0x%X\n", dosHeader.e_lfanew);
    
    fclose(file);
    return 0;

int main(){
    FILE *fopen("C:\Users\tsukky\Desktop\hibiki2\hello.exe", "rb");
    if(fp == NULL){
        printf("File not found\n");
        return 0;
    }
    //エラー処理
}