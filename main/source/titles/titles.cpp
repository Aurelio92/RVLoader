#include <gccore.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <malloc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_set>
#include <filesystem>
#include "rvlmutex.h"
#include "utils.h"
#include "titles.h"
#include "threadsprofiler.h"

#define THREAD_STACK_SIZE 16384

void GamesDatabase::handleDirEntryForScan(const std::filesystem::directory_entry& dir_entry, std::unordered_map<std::string, GameContainer*>& lookup, std::vector<GameContainer>& newGames, std::vector<std::string>& verifiedPaths) {
    if ((scanningForDirectories && !dir_entry.is_directory())
        || (!scanningForDirectories && !dir_entry.is_regular_file())
        || dir_entry.path().filename().string()[0] == '.') {
        return;
    }

    if (!scanningForDirectories && dir_entry.path().extension().string() != targetNameOrExtension) {
        return;
    }

    std::string current_path = dir_entry.path().string();
    if (scanningForDirectories) {
        current_path += "/" + targetNameOrExtension;
    }
    auto it = lookup.find(current_path);

    struct stat result;
    time_t fileLastModified = 0;
    if (stat(current_path.c_str(), &result) == 0) {
        fileLastModified = result.st_mtime;
    }

    //Check if the file is already in the database and if it has not been modified.
    if (it != lookup.end() && it->second->lastModified == fileLastModified) {
        //No need to read all the informations again.
        verifiedPaths.push_back(current_path);
    } else {
        // New file found! Read ID/Title from disk
        createGameContainer(newGames, fileLastModified, current_path); 
    }
}

/*
    Reads a TSV cache file and populates the games vector
*/
bool GamesDatabase::readCache() {
    std::ifstream file(cachePath);
    if (!file.is_open()) return false;
    printf("%s: Reading cache\n", dbName.c_str());
    auto fileSize = std::filesystem::file_size(cachePath);
    std::string content(fileSize, '\0');
    file.read(&content[0], fileSize);
    file.close();
    printf("%s: Read cache\n", dbName.c_str());

    std::istringstream stream(content);
    printf("%s: Stream created\n", dbName.c_str());

    bool dummyExists = fileExists(DUMMY_COVER_PATH);
    printf("%s: Dummy exists: %s\n", dbName.c_str(), dummyExists ? "true" : "false");
    
    std::string line;

    while (std::getline(stream, line)) {
        //Remove \r if found
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        //Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
            continue; 
        }

        //Parse
        std::stringstream ss(line);
        std::string gameIDString, name, path, coverPath, lastModifiedStr;

        //Check if we can successfully read all 3 columns
        if (std::getline(ss, gameIDString, '\t') &&
            std::getline(ss, name, '\t') &&
            std::getline(ss, path, '\t') &&
            std::getline(ss, coverPath, '\t') &&
            std::getline(ss, lastModifiedStr, '\t')) {
            u32 gameID = *(u32*)gameIDString.c_str();
            std::string configPath = std::string(CONFIG_PATH) + "/" + gameIDString + ".cfg";
            if (!fileExists(coverPath)) {
                if (dummyExists) {
                    coverPath = DUMMY_COVER_PATH;
                } else {
                    coverPath = "";
                }
            }
            games.emplace_back(std::stoll(lastModifiedStr), name, path, coverPath, configPath, gameIDString, gameID);
            addMetadataToGameContainer(games.back());
        }
    }
    printf("%s: Read cache done\n", dbName.c_str());

    return true;
}

/*
    Writes the games vector to a TSV cache file
*/
bool GamesDatabase::writeCache() {
    std::ofstream file(cachePath);
    if (!file.is_open()) return false;

    for (GameContainer& game : games) {
        file << game.gameIDString << "\t" << game.name << "\t" << game.path << "\t" << game.coverPath << "\t" << game.lastModified << "\n";
    }
    
    return true;
}

void GamesDatabase::scanGames() {
    std::unordered_map<std::string, GameContainer*> lookup;
    std::vector<GameContainer> newGames;
    std::vector<std::string> verifiedPaths;

    //Return if the scan path does not exist or is not a directory
    if (!std::filesystem::exists(scanPath) || !std::filesystem::is_directory(scanPath)) {
        return;
    }

    //Build lookup table.
    //The block scope ensures the lock is released after the lookup table is built.
    printf("%s: Building lookup table\n", dbName.c_str());
    {
        std::lock_guard<RVLMutex> lock(dbMutex);
        for (auto& game : games) {
            lookup[game.path] = &game;
        }
    }

    printf("%s: Scanning games\n", dbName.c_str());
    if (recursiveScan) {
        for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(scanPath)) {
            handleDirEntryForScan(dir_entry, lookup, newGames, verifiedPaths);
        }
    } else {
        for (const auto& dir_entry : std::filesystem::directory_iterator(scanPath)) {
            handleDirEntryForScan(dir_entry, lookup, newGames, verifiedPaths);
        }
    }
    printf("%s: Scanned games\n", dbName.c_str());
    
    //Locks the database to update it
    std::lock_guard<RVLMutex> lock(dbMutex);
    
    // 1. Remove deleted games (in entries but not in verifiedPaths)
    // Convert verifiedPaths to a set for fast lookup during deletion checks
    printf("%s: Removing deleted games\n", dbName.c_str());
    printf("%s: Verified paths count: %u\n", dbName.c_str(), verifiedPaths.size());
    std::unordered_set<std::string> verifiedSet(verifiedPaths.begin(), verifiedPaths.end());
    
    printf("%s: remove_if\n", dbName.c_str());
    // Remove_if moves "to keep" items to front, returns iterator to "garbage"
    auto new_end = std::remove_if(games.begin(), games.end(),
        [&](const GameContainer& e) {
            // If path is NOT in verified set, delete it
            return verifiedSet.find(e.path) == verifiedSet.end();
        });
    printf("%s: remove_if finished\n", dbName.c_str());
    printf("%s: Erasing %u games\n", dbName.c_str(), games.size() - (new_end - games.begin()));
    printf("%s: Games size: %u\n", dbName.c_str(), games.size());
    games.erase(new_end, games.end());
    printf("%s: Games size: %u\n", dbName.c_str(), games.size());
    printf("%s: erase finished\n", dbName.c_str());

    // 2. Add new games
    games.insert(games.end(), newGames.begin(), newGames.end());

    printf("%s: Beginning sorting\n", dbName.c_str());
    u64 startTime = gettime();
    u32 startMem = SYS_GetArena2Size();
    std::sort(games.begin(), games.end(), GameContainer::compare);
    printf("%s: Sorting finished in %u ms. Lost %.3f MB\n", dbName.c_str(), diff_msec(startTime, gettime()), (float)(startMem - SYS_GetArena2Size()) / 1048576.0f);

    writeCache();
}

void GamesDatabase::startScanAndUpdateThread() {
    readCache();
    scanAndUpdateThreadStack = (u8*)memalign(32, THREAD_STACK_SIZE);
    ThreadsProfiler::createThreadAndProfile(dbName + " ScanAndUpdateThread", &scanAndUpdateThreadHandle, scanAndUpdateThread, (void*)this, scanAndUpdateThreadStack, THREAD_STACK_SIZE, 30);
}

void* GamesDatabase::scanAndUpdateThread(void* arg) {
    GamesDatabase* db = (GamesDatabase*)arg;
    db->scanGames();
    db->coversThreadStack = (u8*)memalign(32, THREAD_STACK_SIZE);
    ThreadsProfiler::createThreadAndProfile(db->dbName + " CoversThread", &db->coversThreadHandle, loadCoversThread, (void*)db, db->coversThreadStack, THREAD_STACK_SIZE, 30);
    db->hasFinishedScanning.send();
    return NULL;
}

void* GamesDatabase::loadCoversThread(void* arg) {
    static const char gamesRegions[] = {'E', 'P', 'J'};
    GamesDatabase* db = (GamesDatabase*)arg;
    std::unordered_map<GameContainer*, std::string> coverMap;
    std::string tempID;
    std::string tempPath;

    bool dummyExists = fileExists(DUMMY_COVER_PATH);

    //Make a copy of the cover paths for each game
    {
        std::lock_guard<RVLMutex> lock(db->dbMutex);
        for (auto& game : db->games) {
            coverMap[&game] = game.gameIDString;
        }
    }

    //Check for covers
    for (auto& cover : coverMap) {
        tempID = cover.second;
        tempPath = std::string(COVER_PATH) + "/" + tempID + ".png";
        if (!fileExists(tempPath)) {
            //Try looking for a cover of the same game from a different region
            bool coverFound = false;
            for (u32 i = 0; i < sizeof(gamesRegions) && !coverFound; i++) {
                if (cover.second[3] == gamesRegions[i]) {
                    continue;
                }
                tempID[3] = gamesRegions[i];
                tempPath = std::string(COVER_PATH) + "/" + tempID + ".png";
                if (fileExists(tempPath)) {
                    coverFound = true;
                }
            }
            if (coverFound) {
                cover.second = tempID;
            } else {
                cover.second = "";
            }
        }
    }

    //Update the cover paths
    {
        std::lock_guard<RVLMutex> lock(db->dbMutex);
        for (auto& cover : coverMap) {
            GameContainer* game = cover.first;
            if (cover.second == "" && dummyExists) {
                game->coverPath  = DUMMY_COVER_PATH;
            } else if (cover.second != "") {
                game->coverPath  = std::string(COVER_PATH) + "/" + cover.second + ".png";
            } else {
                game->coverPath  = "";
            }
            if (game->image != NULL) {
                delete game->image;
                game->image = NULL; //This will force a reload of the cover.
            }
        }
        db->writeCache();
    }

    return NULL;
}

void lockTitlesDBs() {
    gcGamesDatabase.lock();
    wiiGamesDatabase.lock();
    vcGamesDatabase.lock();
    wiiChannelsDatabase.lock();
}

void unlockTitlesDBs() {
    gcGamesDatabase.unlock();
    wiiGamesDatabase.unlock();
    vcGamesDatabase.unlock();
    wiiChannelsDatabase.unlock();
}
