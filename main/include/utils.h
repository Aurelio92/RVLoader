#pragma once

#define ALIGNED(x) __attribute__((aligned(x)))

bool matchStr(const char* str1, const char* str2);
int readFile(const char* path, uint8_t** buffer, uint32_t* fileSize);
int writeFile(const char* path, void* buffer, uint32_t fileSize);
bool fileExists(const char* path);
bool fileExists(const std::string path);
