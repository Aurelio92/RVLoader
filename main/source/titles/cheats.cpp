#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdint.h>
#include "cheats.h"

void Cheat::parseFile(const std::string filename) {
    CheatCode curCheat;
    std::string curCheatName;
    std::string gameCode;
    std::string gameName;
    std::string codeLine;
    std::ifstream infile(filename);

    cheatCodes.clear();

    //Start parsing text code
    std::getline(infile, gameCode);
    std::getline(infile, gameName);

    while (std::getline(infile, codeLine)) {
        codeLine.erase(std::remove(codeLine.begin(), codeLine.end(), '\r'), codeLine.end()); //Remove tailing CR, if any
        std::istringstream lineStream(codeLine);
        uint32_t gecko1, gecko2;

        //Check for end of cheatcode chunk
        if (codeLine == "" && curCheatName != "" && !curCheat.codesList.empty()) {
            cheatCodes[curCheatName] = curCheat;
            curCheatName = "";
            curCheat.notes = "";
            curCheat.codesList.clear();
            continue;
        }

        if (lineStream >> std::hex >> gecko1 >> std::hex >> gecko2) {
            curCheat.codesList.push_back(std::make_pair(gecko1, gecko2));
        } else if (curCheatName == "") { //New code
            curCheat.active = false;
            curCheatName = codeLine;
        } else if (curCheat.notes == "") {
            curCheat.notes = codeLine;
        } else {
            curCheat.notes += '\n' + codeLine;
        }
    }

    if (curCheatName != "" && !curCheat.codesList.empty()) {
        cheatCodes[curCheatName] = curCheat;
    }
}

std::string Cheat::getCheatNameHash(std::string cheatName) {
    return std::to_string(std::hash<std::string>{}(cheatName));
}

void Cheat::setCheatActive(const std::string cheatName, bool active) {
    try {
        cheatCodes.at(cheatName).active = active;
    } catch (std::out_of_range& e) {
        throw e;
    }
}

bool Cheat::getCheatActive(const std::string cheatName) {
    try {
        return cheatCodes.at(cheatName).active;
    } catch (std::out_of_range& e) {
        throw e;
    }
}

std::vector<uint32_t> Cheat::generateGCT() {
    std::vector<uint32_t> gct;
    uint32_t gctSize = 4;

    for (auto& cheat : cheatCodes) {
        gctSize += cheat.second.active ? 2 * cheat.second.codesList.size() : 0;
    }
    gct.reserve(gctSize);
    gct.push_back(0x00D0C0DE);
    gct.push_back(0x00D0C0DE);
    for (auto& cheat : cheatCodes) {
        if (cheat.second.active) {
            for (auto& cheatGeckoCodes : cheat.second.codesList) {
                gct.push_back(cheatGeckoCodes.first);
                gct.push_back(cheatGeckoCodes.second);
            }
        }
    }
    gct.push_back(0xF0000000);
    gct.push_back(0x00000000);

    std::ofstream fout("data.gct", std::ios::out | std::ios::binary);
    fout.write((char*)gct.data(), gct.size() * sizeof(uint32_t));
    fout.close();

    return gct;
}
