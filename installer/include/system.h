#pragma once

bool initFAT();
int reloadIOS(int ios, int* ahbprot);
void shutdown();
void bootDOL(const char* path, char* args);
