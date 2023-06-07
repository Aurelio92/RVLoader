#include <stdio.h>
#include <string.h>
#include <lua.hpp>
#include "config.h"

void Config::open(const char* filepath) {
    if (!filepath)
        return;

    printf("Config: Opening %s\n", filepath);

    lastPath = filepath;

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, filepath);

    //Grab all the globally defined variables
    lua_pushglobaltable(L);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (lua_isstring(L, -2)) { //key must be string to be valid
            const char* key = lua_tostring(L, -2);

            //Skip the LUA defined global
            if (strcmp(key, "_VERSION")) {
                lua_getglobal(L, key);

                //lua_isstring returns 1 also for numbers (because convertible to string)
                if (lua_isinteger(L, -1)) {
                    int value = lua_tointeger(L, -1);
                    intEntries.insert(std::pair<std::string, int>(key, value));
                    printf("%s: (integer): %d\n", key, value);
                } else if (lua_isnumber(L, -1)) {
                    double value = lua_tonumber(L, -1);
                    doubleEntries.insert(std::pair<std::string, double>(key, value));
                    printf("%s: (double): %lf\n", key, value);
                } else if (lua_isstring(L, -1)) {
                    const char* value = lua_tostring(L, -1);
                    stringEntries.insert(std::pair<std::string, std::string>(key, value));
                    printf("%s: (string): %s\n", key, value);
                }

                lua_pop(L, 1);
            }
        }
      lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_close(L);
}

void Config::save(const char* filepath) {
    FILE* fp = fopen(filepath, "w");
    if (!fp)
        return;

    for (auto& val : stringEntries)
        fprintf(fp, "%s = \"%s\"\n", val.first.c_str(), val.second.c_str());

    for (auto& val : intEntries)
        fprintf(fp, "%s = %d\n", val.first.c_str(), val.second);

    for (auto& val : doubleEntries)
        fprintf(fp, "%s = %lf\n", val.first.c_str(), val.second);

    fclose(fp);
}

void Config::save() {
    save(lastPath.c_str());
}

void Config::close() {
    stringEntries.clear();
    intEntries.clear();
    doubleEntries.clear();
}

bool Config::getValue(const char* key, std::string* val) {
    char tempKey[256];
    strcpy(tempKey, key);
    while (char* c = strchr(tempKey, ' '))
        *c = '_';
    while (char* c = strchr(tempKey, '-'))
        *c = '_';

    std::unordered_map<std::string, std::string>::iterator it = stringEntries.find(tempKey);

    if (it != stringEntries.end()) {
        if (val != NULL)
            *val = it->second;
        return true;
    }

    return false;
}

bool Config::getValue(const char* key, int* val) {
    char tempKey[256];
    strcpy(tempKey, key);
    while (char* c = strchr(tempKey, ' '))
        *c = '_';
    while (char* c = strchr(tempKey, '-'))
        *c = '_';

    std::unordered_map<std::string, int>::iterator it = intEntries.find(tempKey);

    if (it != intEntries.end()) {
        if (val != NULL)
            *val = it->second;
        return true;
    }

    return false;
}

bool Config::getValue(const char* key, double* val) {
    char tempKey[256];
    strcpy(tempKey, key);
    while (char* c = strchr(tempKey, ' '))
        *c = '_';
    while (char* c = strchr(tempKey, '-'))
        *c = '_';

    std::unordered_map<std::string, double>::iterator it = doubleEntries.find(tempKey);

    if (it != doubleEntries.end()) {
        if (val != NULL)
            *val = it->second;
        return true;
    }

    return false;
}

void Config::setValue(const char* key, std::string val) {
    char tempKey[256];
    strcpy(tempKey, key);
    while (char* c = strchr(tempKey, ' '))
        *c = '_';
    while (char* c = strchr(tempKey, '-'))
        *c = '_';

    stringEntries[tempKey] = val;
}

void Config::setValue(const char* key, int val) {
    char tempKey[256];
    strcpy(tempKey, key);
    while (char* c = strchr(tempKey, ' '))
        *c = '_';
    while (char* c = strchr(tempKey, '-'))
        *c = '_';

    intEntries[tempKey] = val;
}

void Config::setValue(const char* key, double val) {
    char tempKey[256];
    strcpy(tempKey, key);
    while (char* c = strchr(tempKey, ' '))
        *c = '_';
    while (char* c = strchr(tempKey, '-'))
        *c = '_';

    doubleEntries[tempKey] = val;
}
