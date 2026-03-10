#include <gccore.h>
#include <fstream>
#include "titles.h"
#include "utils.h"

GamesDatabaseWii wiiGamesDatabase;

void GamesDatabaseWii::createGameContainer(std::vector<GameContainer>& newGames, time_t lastModified, const std::string& path) {
    u32 magic;
    std::string gameIDString(7, '\0');
    u32 gameID;
    std::string gameName(0x41, '\0');
    std::string coverPath;
    std::string configPath;
    
    std::ifstream ifs(path, std::ios::binary);

    if (!ifs.is_open())
        return;

    ifs.read(reinterpret_cast<char*>(&magic), sizeof(u32));
    if (magic != WBFS_MAGIC) {
        ifs.close();
        return;
    }
    
    ifs.seekg(0x200, std::ios::beg);
    ifs.read(reinterpret_cast<char*>(&gameID), sizeof(u32));
    ifs.seekg(0x200, std::ios::beg);
    ifs.read(gameIDString.data(), 6);
    gameIDString.resize(ifs.gcount());

    try {
        gameName = wiiTDB::getGameName(gameIDString);
    } catch (std::out_of_range& e) {
        ifs.seekg(0x220, std::ios::beg);
        ifs.read(gameName.data(), 0x40);
        gameName.resize(ifs.gcount());
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

void GamesDatabaseWii::addMetadataToGameContainer(GameContainer& gc) {
    gc.cheatPath = std::string(gc.path, gc.path.length() - targetNameOrExtension.length()).append(".txt");
}

void addWiiGames() {
    wiiGamesDatabase.startScanAndUpdateThread();
}
