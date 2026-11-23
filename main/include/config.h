/*
Config files are plain text, one "key = value" per line. Lines starting with # are comments.
Strings are quoted, doubles contain a '.', anything else is an int
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

        void parseValue(const std::string& key, const std::string& val);

    public:
        //Keys are stored with spaces and dashes replaced by underscores
        static std::string normalizeKey(const std::string& key);

        void open(const char* filepath);
        void save(const char* filepath);
        void save();
        void close();

        //Copy every entry of "over" into this config, replacing existing keys
        void merge(const Config& over);
        //Remove a key whatever its type
        void erase(const char* key);
        //Remove every entry whose key exists in "base" with the same type and value
        void removeEntriesEqualTo(const Config& base);
        bool hasKey(const char* key);

        bool getValue(const char* key, std::string* val);
        bool getValue(const char* key, int* val);
        bool getValue(const char* key, double* val);

        void setValue(const char* key, std::string val);
        void setValue(const char* key, int val);
        void setValue(const char* key, double val);
};
