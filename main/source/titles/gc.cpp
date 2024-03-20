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

#define GC_MAGIC    0xC2339F3D
#define CISO_MAGIC  0x4349534F

std::vector<GameContainer> gcGames;
static lwp_t gcCoversThreadHandle;
static u8* gcCoversThreadStack;
mutex_t gcCoversMutex;

static const char _gamesRegions[] = {'E', 'P', 'J'}; //Implementing every region slows down loading times

static void* loadGCCoversThread(void* arg) {
    char tempPath[PATH_MAX];
    char tempID[7];

    for (auto& game : gcGames) {
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
        LWP_MutexLock(gcCoversMutex);
        game.coverPath = tempPath;
        //This will force a reload of the cover.
        //DO NOT delete previously loaded cover because it is dummyCover (see main.cpp)
        game.image = NULL;
        LWP_MutexUnlock(gcCoversMutex);
        usleep(100);
    }

    return NULL;
}

//Start a thread to load all the GC games covers
static void loadGCCovers() {
    LWP_MutexInit(&gcCoversMutex, false);
    gcCoversThreadStack = (u8*)memalign(32, THREAD_STACK_SIZE);
    LWP_CreateThread(&gcCoversThreadHandle, loadGCCoversThread, NULL, gcCoversThreadStack, THREAD_STACK_SIZE, 30);
}

void addGCGames() {
    char tempPath[PATH_MAX];
    struct dirent *dirp;

    DIR* dp = opendir("/games");
    if (dp == NULL)
        return;

    while ((dirp = readdir(dp)) != NULL) {
        u32 magic;
        char gameId[7];
        u32 gameIdU32;
        char gameName[0x41]; //Should be 0x3E0, but we crop it
        char coverPath[PATH_MAX];
        char configPath[PATH_MAX];

        if (dirp->d_name == NULL)
            continue;

        if (dirp->d_name[0] == '.')
            continue;

        if (dirp->d_type != DT_DIR)
            continue;

        sprintf(tempPath, "/games/%s/game.iso", dirp->d_name);
        FILE* fp = fopen(tempPath, "rb");
        if (fp == NULL)
            continue;

        fseek(fp, 0x1C, SEEK_SET);
        fread(&magic, 1, sizeof(u32), fp);
        if (magic == GC_MAGIC) {
            fseek(fp, 0x0, SEEK_SET);
            fread(&gameIdU32, 1, sizeof(u32), fp);
            fseek(fp, 0x0, SEEK_SET);
            fread(gameId, 1, 6, fp);
            gameId[6] = '\0';
        } else if (magic == CISO_MAGIC) {
            fseek(fp, 0x8000, SEEK_SET);
            fread(&gameIdU32, 1, sizeof(u32), fp);
            fseek(fp, 0x8000, SEEK_SET);
            fread(gameId, 1, 6, fp);
            gameId[6] = '\0';
        } else {
            fclose(fp);
            continue;
        }
        sprintf(configPath, "%s/%s.cfg", CONFIG_PATH, gameId);
        sprintf(coverPath, "%s/%s.png", COVER_PATH, gameId);

        //Try grabbing the game name from wiiTDB, otherwise read it from the disc image
        try {
            gcGames.push_back(GameContainer(wiiTDB::getGameName(gameId), tempPath, coverPath, configPath, "", gameId, gameIdU32));
        } catch (std::out_of_range& e) {
            if (magic == GC_MAGIC) {
                fseek(fp, 0x20, SEEK_SET);
                fread(gameName, 1, 0x40, fp);
                gameName[0x40] = '\0';
            } else if (magic == CISO_MAGIC) {
                fseek(fp, 0x8020, SEEK_SET);
                fread(gameName, 1, 0x40, fp);
                gameName[0x40] = '\0';
            }
            gcGames.push_back(GameContainer(gameName, tempPath, coverPath, configPath, "", gameId, gameIdU32));
        }
        fclose(fp);
    }
    closedir(dp);

    std::sort(gcGames.begin(), gcGames.end(), GameContainer::compare);
    loadGCCovers();
}
