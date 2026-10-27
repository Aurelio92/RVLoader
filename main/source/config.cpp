#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fstream>
#include <string>
#include <unordered_map>

#include "config.h"

static std::string trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && isspace((unsigned char)s[start])) start++;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    return s.substr(start, end - start);
}

void Config::parseValue(const std::string& key, const std::string& val) {
    // Case 1: String (Starts and ends with quotes)
    if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
        stringEntries[key] = val.substr(1, val.size() - 2);
    }
    // Case 2: Double (Contains a decimal point)
    else if (val.find('.') != std::string::npos) {
        try {
            doubleEntries[key] = std::stod(val);
        } catch (...) {}
    }
    // Case 3: Integer
    else {
        try {
            intEntries[key] = std::stoi(val);
        } catch (...) {}
    }
}

void Config::open(const char* filepath) {
    if (!filepath)
        return;
    lastPath = filepath;
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) continue;

        std::string key = trim(line.substr(0, delimiterPos));
        std::string value = trim(line.substr(delimiterPos + 1));

        parseValue(key, value);
    }
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
