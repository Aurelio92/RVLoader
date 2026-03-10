#include <gccore.h>
#include <fstream>
#include "titles.h"
#include "utils.h"
#include "gcsave.h"

#define GC_MAGIC    0xC2339F3D
#define CISO_MAGIC  0x4349534F

GamesDatabaseGC gcGamesDatabase;

void GamesDatabaseGC::createGameContainer(std::vector<GameContainer>& newGames, time_t lastModified, const std::string& path) {
    u32 magic;
    std::string gameIDString(7, '\0');
    u32 gameID;
    std::string gameName(0x41, '\0');
    std::string coverPath;
    std::string configPath;
    
    std::ifstream ifs(path, std::ios::binary);

    if (!ifs.is_open())
        return;

    ifs.seekg(0x1C, std::ios::beg);
    ifs.read(reinterpret_cast<char*>(&magic), sizeof(u32));
    if (magic == GC_MAGIC) {
        ifs.seekg(0x0, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(&gameID), sizeof(u32));
        ifs.seekg(0x0, std::ios::beg);
        ifs.read(gameIDString.data(), 6);
        gameIDString.resize(ifs.gcount());
    } else if (magic == CISO_MAGIC) {
        ifs.seekg(0x8000, std::ios::beg);
        ifs.read(reinterpret_cast<char*>(&gameID), sizeof(u32));
        ifs.seekg(0x8000, std::ios::beg);
        ifs.read(gameIDString.data(), 6);
        gameIDString.resize(ifs.gcount());
    } else {
        ifs.close();
        return;
    }

    try {
        gameName = wiiTDB::getGameName(gameIDString);
    } catch (std::out_of_range& e) {
        if (magic == GC_MAGIC) {
            ifs.seekg(0x20, std::ios::beg);
            ifs.read(gameName.data(), 0x40);
            gameName.resize(ifs.gcount());
        } else if (magic == CISO_MAGIC) {
            ifs.seekg(0x8020, std::ios::beg);
            ifs.read(gameName.data(), 0x40);
            gameName.resize(ifs.gcount());
        }
    }

    ifs.close();

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

void GamesDatabaseGC::addMetadataToGameContainer(GameContainer& gc) {
    std::string savePath;
    savePath = "/saves/" + gc.gameIDString.substr(0, 4) + ".raw";
    gc.save.loadSave(savePath);
}

void addGCGames() {
    gcGamesDatabase.startScanAndUpdateThread();
}
