#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

bool matchStr(const char* str1, const char* str2) {
    if (!str1 || !str2)
        return false;

    char c1, c2;
    do {
        c1 = *str1++;
        c2 = *str2++;
        if (c1 != c2) return false;
    } while (c1 && c2);

    return true;
}

int readFile(const char* path, uint8_t** buffer, uint32_t* fileSize) {
    if (path == NULL || buffer == NULL) {
        return -1;
    }
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("Error while reading file %s\n", path);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    uint32_t size = ftell(fp);
    rewind(fp);
    *buffer = (uint8_t*)malloc(size);

    if (*buffer == NULL) {
        fclose(fp);
        return -1;
    }
    
    fread(*buffer, 1, size, fp);
    fclose(fp);

    if (fileSize != NULL) {
        *fileSize = size;
    }

    return 0;
}

int writeFile(const char* path, void* buffer, uint32_t fileSize) {
    if (path == NULL || buffer == NULL) {
        return -1;
    }
    FILE* fp = fopen(path, "wb");
    if (fp == NULL) {
        printf("Error while writing file %s\n", path);
        return -1;
    }

    fwrite(buffer, 1, fileSize, fp);
    fclose(fp);

    return 0;
}
