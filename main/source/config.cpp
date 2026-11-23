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

std::string Config::normalizeKey(const std::string& key) {
    std::string normalized = key;
    for (char& c : normalized) {
        if (c == ' ' || c == '-')
            c = '_';
    }
    return normalized;
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

        std::string key = normalizeKey(trim(line.substr(0, delimiterPos)));
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

void Config::merge(const Config& over) {
    for (auto& val : over.stringEntries) {
        erase(val.first.c_str());
        stringEntries[val.first] = val.second;
    }
    for (auto& val : over.intEntries) {
        erase(val.first.c_str());
        intEntries[val.first] = val.second;
    }
    for (auto& val : over.doubleEntries) {
        erase(val.first.c_str());
        doubleEntries[val.first] = val.second;
    }
}

void Config::erase(const char* key) {
    std::string normKey = normalizeKey(key);
    stringEntries.erase(normKey);
    intEntries.erase(normKey);
    doubleEntries.erase(normKey);
}

template<typename T>
static void removeEqual(std::unordered_map<std::string, T>& entries, const std::unordered_map<std::string, T>& base) {
    for (auto it = entries.begin(); it != entries.end();) {
        auto baseIt = base.find(it->first);
        if (baseIt != base.end() && baseIt->second == it->second)
            it = entries.erase(it);
        else
            ++it;
    }
}

void Config::removeEntriesEqualTo(const Config& base) {
    removeEqual(stringEntries, base.stringEntries);
    removeEqual(intEntries, base.intEntries);
    removeEqual(doubleEntries, base.doubleEntries);
}

bool Config::hasKey(const char* key) {
    std::string normKey = normalizeKey(key);
    return stringEntries.count(normKey) || intEntries.count(normKey) || doubleEntries.count(normKey);
}

bool Config::getValue(const char* key, std::string* val) {
    auto it = stringEntries.find(normalizeKey(key));

    if (it != stringEntries.end()) {
        if (val != NULL)
            *val = it->second;
        return true;
    }

    return false;
}

bool Config::getValue(const char* key, int* val) {
    auto it = intEntries.find(normalizeKey(key));

    if (it != intEntries.end()) {
        if (val != NULL)
            *val = it->second;
        return true;
    }

    return false;
}

bool Config::getValue(const char* key, double* val) {
    auto it = doubleEntries.find(normalizeKey(key));

    if (it != doubleEntries.end()) {
        if (val != NULL)
            *val = it->second;
        return true;
    }

    return false;
}

void Config::setValue(const char* key, std::string val) {
    stringEntries[normalizeKey(key)] = val;
}

void Config::setValue(const char* key, int val) {
    intEntries[normalizeKey(key)] = val;
}

void Config::setValue(const char* key, double val) {
    doubleEntries[normalizeKey(key)] = val;
}
