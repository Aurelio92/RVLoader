/*
Config files are nothing more than LUA scripts where only global variables are defined.
Only string, doubles and int entries are supported
*/

#pragma once

#include <string>
#include <unordered_map>
#include <gccore.h>

class Config {
    private:
        std::string lastPath;

        std::unordered_map<std::string, std::string> stringEntries;
        std::unordered_map<std::string, int> intEntries;
        std::unordered_map<std::string, double> doubleEntries;

    public:
        void open(const char* filepath);
        void save(const char* filepath);
        void save();
        void close();

        bool getValue(const char* key, std::string* val);
        bool getValue(const char* key, int* val);
        bool getValue(const char* key, double* val);

        void setValue(const char* key, std::string val);
        void setValue(const char* key, int val);
        void setValue(const char* key, double val);
};
