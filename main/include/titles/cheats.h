#pragma once

#include <string>
#include <stdint.h>
#include <utility>
#include <map>
#include <list>
#include <vector>
#include <algorithm>

class Cheat {
public:
    typedef struct {
        std::list<std::pair<uint32_t, uint32_t>> codesList;
        std::string notes;
        bool active;
    } CheatCode;

private:
    std::map<std::string, CheatCode> cheatCodes;

public:
    std::map<std::string, CheatCode>::iterator begin() {return cheatCodes.begin();}
    std::map<std::string, CheatCode>::iterator end() {return cheatCodes.end();}
    void parseFile(const std::string filename);
    static std::string getCheatNameHash(std::string cheatName);
    void setCheatActive(const std::string cheatName, bool active);
    bool getCheatActive(const std::string cheatName);
    std::vector<uint32_t> generateGCT();

};