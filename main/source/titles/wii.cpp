#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "titles.h"

#define THREAD_STACK_SIZE 16384

std::vector<GameContainer> wiiGames;
static lwp_t wiiCoversThreadHandle;
static u8* wiiCoversThreadStack;
mutex_t wiiCoversMutex;

static const char _gamesRegions[] = {'E', 'P', 'J'}; //Implementing every region slows down loading times

static void* loadWiiCoversThread(void* arg) {
    char tempPath[PATH_MAX];
    char tempID[7];

    for (auto& game : wiiGames) {
        sprintf(tempID, "%s", game.gameIDString.c_str());
        sprintf(tempPath, "%s/%s.png", COVER_PATH, tempID);
        FILE* coverFP = fopen(tempPath, "rb");
        if (coverFP) {
            fclose(coverFP);
        } else {
            //Try looking for a cover of the same game from a different region
            bool coverFound = false;
            for (u32 i = 0; i < sizeof(_gamesRegions) && !coverFound; i++) {
                tempID[3] = _gamesRegions[i];
                sprintf(tempPath, "%s/%s.png", COVER_PATH, tempID);
                coverFP = fopen(tempPath, "rb");
                if (coverFP) {
                    coverFound = true;
                    fclose(coverFP);
                }
            }
            if (!coverFound)
                sprintf(tempPath, "%s/dummy.png", COVER_PATH);
            coverFP = fopen(tempPath, "rb");
            if (coverFP) {
                fclose(coverFP);
            } else {
                tempPath[0] = '\0';
            }
        }
        LWP_MutexLock(wiiCoversMutex);
        game.coverPath = tempPath;
        //This will force a reload of the cover.
        //DO NOT delete previously loaded cover because it is dummyCover (see main.cpp)
        game.image = NULL;
        LWP_MutexUnlock(wiiCoversMutex);
        usleep(100);
    }

    return NULL;
}

//Start a thread to load all the Wii games covers
static void loadWiiCovers() {
    LWP_MutexInit(&wiiCoversMutex, false);
    wiiCoversThreadStack = (u8*)memalign(32, THREAD_STACK_SIZE);
    LWP_CreateThread(&wiiCoversThreadHandle, loadWiiCoversThread, NULL, wiiCoversThreadStack, THREAD_STACK_SIZE, 30);
}

static void listWiiGames(const char* path) {
    char tempPath[PATH_MAX];
    struct dirent *dirp;

    DIR* dp = opendir(path);
    if (dp == NULL)
        return;

    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_name == NULL)
            continue;

        if (dirp->d_name[0] == '.')
            continue;

        if (dirp->d_type == DT_DIR) {
            sprintf(tempPath, "%s/%s", path, dirp->d_name);
            listWiiGames(tempPath);
        } else if (!strncmp(&dirp->d_name[strlen(dirp->d_name) - 5], ".wbfs", 5) && dirp->d_type == DT_REG) {
            u32 magic;
            char gameId[7];
            u32 gameIdU32;
            char gameName[0x41];
            char coverPath[PATH_MAX];
            char configPath[PATH_MAX];
            std::string cheatPath;

            sprintf(tempPath, "%s/%s", path, dirp->d_name);
            FILE* fp = fopen(tempPath, "rb");
            if (fp == NULL)
                continue;

            fread(&magic, 1, sizeof(u32), fp);
            if (magic != WBFS_MAGIC) {
                fclose(fp);
                continue;
            }

            cheatPath = std::string(tempPath, strlen(tempPath) - 5).append(".txt");

            fseek(fp, 0x200, SEEK_SET);
            fread(&gameIdU32, 1, sizeof(u32), fp);
            fseek(fp, 0x200, SEEK_SET);
            fread(gameId, 1, 6, fp);
            gameId[6] = '\0';
            sprintf(configPath, "%s/%s.cfg", CONFIG_PATH, gameId);
            coverPath[0] = '\0';

            //Try grabbing the game name from wiiTDB, otherwise read it from the disc image
            try {
                wiiGames.push_back(GameContainer(wiiTDB::getGameName(gameId), tempPath, coverPath, configPath, cheatPath, gameId, gameIdU32));
            } catch (std::out_of_range& e) {
                fseek(fp, 0x220, SEEK_SET);
                fread(gameName, 1, 0x40, fp);
                gameName[0x40] = '\0';
                wiiGames.push_back(GameContainer(gameName, tempPath, coverPath, configPath, cheatPath, gameId, gameIdU32));
            }
            fclose(fp);
        }
    }
    closedir(dp);
}

void addWiiGames() {
    listWiiGames(WBFS_BASE_FOLDER);
    std::sort(wiiGames.begin(), wiiGames.end(), GameContainer::compare);
    loadWiiCovers();
}
