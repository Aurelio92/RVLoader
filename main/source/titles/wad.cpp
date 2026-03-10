#include <gccore.h>
#include <fstream>
#include "titles.h"
#include "systitles.h"
#include "utils.h"

void GamesDatabaseWAD::createGameContainer(std::vector<GameContainer>& newGames, time_t lastModified, const std::string& path) {
    std::string gameIDString;
    u32 gameID;
    std::string gameName(0x41, '\0');
    std::string coverPath;
    std::string configPath;
    
    std::ifstream ifs(path, std::ios::binary);

    if (!ifs.is_open()) {
        printf("Failed to open wad file %s\n", path.c_str());
        return;
    }

    //Read gameID from WAD
    WAD wad;
    ifs.read(reinterpret_cast<char*>(&wad.header), sizeof(WAD_HEADER));
    DCFlushRange(&wad.header, sizeof(WAD_HEADER));

    //Check if valid WAD file
    if  (wad.header.headerSize != 0x20 ||
        (wad.header.type != 0x49730000 && wad.header.type != 0x69620000 && wad.header.type != 0x426b0000)) {
            printf("WAD type does not match. Expected 0x49730000, 0x69620000 or 0x426b0000, got 0x%08X\n", wad.header.type);
        ifs.close();
        return;
    }

    //Jump to titleIDOffset offset
    size_t titleIDOffset = ((wad.header.headerSize + 0x3F) & ~0x3F) + ((wad.header.certSize + 0x3F) & ~0x3F) + ((wad.header.crlSize + 0x3F) & ~0x3F) + ((wad.header.tikSize + 0x3F) & ~0x3F) + 0x190;
    ifs.seekg(titleIDOffset, std::ios::beg);
    ifs.read(reinterpret_cast<char*>(&gameID), sizeof(u32));
    ifs.close();

    gameIDString += static_cast<char>((gameID >> 24) & 0xFF);
    gameIDString += static_cast<char>((gameID >> 16) & 0xFF);
    gameIDString += static_cast<char>((gameID >> 8) & 0xFF);
    gameIDString += static_cast<char>(gameID & 0xFF);

    try {
        gameName = wiiTDB::getGameName(gameIDString);
    } catch (std::out_of_range& e) {
        size_t lastSlash = path.find_last_of("/\\");
        size_t lastDot = path.find_last_of(".");
        if (lastSlash == std::string::npos) lastSlash = -1;
        if (lastDot == std::string::npos || lastDot < lastSlash) lastDot = path.length();
        
        gameName = path.substr(lastSlash + 1, lastDot - lastSlash - 1);
    }

    configPath = std::string(CONFIG_PATH) + "/" + gameIDString + ".cfg";
    coverPath = std::string(COVER_PATH) + "/" + gameIDString + ".png";

    if (!fileExists(coverPath)) { //Check if gameID cover exists
        coverPath = DUMMY_COVER_PATH;
        if (!fileExists(coverPath)) { //Check if dummy exists
            coverPath = "";
        }
    }

    newGames.emplace_back(lastModified, gameName, path, coverPath, configPath, gameIDString, gameID);
    addMetadataToGameContainer(newGames.back());
}

void GamesDatabaseWAD::addMetadataToGameContainer(GameContainer& gc) {
    gc.cheatPath = std::string(gc.path, gc.path.length() - targetNameOrExtension.length()).append(".txt");
}
