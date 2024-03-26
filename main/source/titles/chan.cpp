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
#include "systitles.h"

#define THREAD_STACK_SIZE 16384

std::vector<GameContainer> wiiChannels;
static lwp_t wiiChanCoversThreadHandle;
static u8* wiiChanCoversThreadStack;
mutex_t wiiChanCoversMutex;

static const char _gamesRegions[] = {'E', 'P', 'J'}; //Implementing every region slows down loading times

static void* loadWiiChannelsCoversThread(void* arg) {
    char tempPath[PATH_MAX];
    char tempID[7];

    for (auto& game : wiiChannels) {
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
        LWP_MutexLock(wiiChanCoversMutex);
        game.coverPath = tempPath;
        //This will force a reload of the cover.
        //DO NOT delete previously loaded cover because it is dummyCover (see main.cpp)
        game.image = NULL;
        LWP_MutexUnlock(wiiChanCoversMutex);
        usleep(100);
    }

    return NULL;
}

//Start a thread to load all the GC games covers
static void loadWiiChannelsCovers() {
    LWP_MutexInit(&wiiChanCoversMutex, false);
    wiiChanCoversThreadStack = (u8*)memalign(32, THREAD_STACK_SIZE);
    LWP_CreateThread(&wiiChanCoversThreadHandle, loadWiiChannelsCoversThread, NULL, wiiChanCoversThreadStack, THREAD_STACK_SIZE, 30);
}

void addWiiChannels() {
    char tempPath[PATH_MAX];
    struct dirent *dirp;

    DIR* dp = opendir("/channels");
    if (dp == NULL)
        return;

    while ((dirp = readdir(dp)) != NULL) {
        char gameId[5];
        u32 gameIdU32;
        char gameName[0x41];
        char coverPath[PATH_MAX];
        char configPath[PATH_MAX];
        std::string cheatPath;

        if (dirp->d_name == NULL)
            continue;

        if (dirp->d_name[0] == '.')
            continue;

        if (dirp->d_type != DT_REG)
            continue;

        if (strncmp(&dirp->d_name[strlen(dirp->d_name) - 4], ".wad", 4))
            continue;

        sprintf(tempPath, "/channels/%s", dirp->d_name);
        FILE* fp = fopen(tempPath, "rb");
        if (fp == NULL)
            continue;

        //Read gameID from WAD
        WAD wad;
        fread(&wad.header, 1, sizeof(WAD_HEADER), fp);
        DCFlushRange(&wad.header, sizeof(WAD_HEADER));

        //Check if valid WAD file
        if  (wad.header.headerSize != 0x20 ||
            (wad.header.type != 0x49730000 && wad.header.type != 0x69620000 && wad.header.type != 0x426b0000)) {
            fclose(fp);
            continue;
        }

        cheatPath = std::string(tempPath, strlen(tempPath) - 4).append(".txt");

        //Jump to titleIDOffset offset
        size_t titleIDOffset = ((wad.header.headerSize + 0x3F) & ~0x3F) + ((wad.header.certSize + 0x3F) & ~0x3F) + ((wad.header.crlSize + 0x3F) & ~0x3F) + ((wad.header.tikSize + 0x3F) & ~0x3F) + 0x190;
        fseek(fp, titleIDOffset, SEEK_SET);
        fread(&gameIdU32, 1, sizeof(u32), fp);
        fclose(fp);

        gameId[0] = (gameIdU32 >> 24) & 0xFF;
        gameId[1] = (gameIdU32 >> 16) & 0xFF;
        gameId[2] = (gameIdU32 >> 8) & 0xFF;
        gameId[3] = gameIdU32 & 0xFF;
        gameId[4] = '\0';

        sprintf(configPath, "%s/%s.cfg", CONFIG_PATH, gameId);
        sprintf(coverPath, "%s/%s.png", COVER_PATH, gameId);

        //Try grabbing the game name from wiiTDB, otherwise read it from the disc image
        try {
            wiiChannels.push_back(GameContainer(wiiTDB::getGameName(gameId), tempPath, coverPath, configPath, cheatPath, gameId, gameIdU32));
        } catch (std::out_of_range& e) {
            snprintf(gameName, 0x40, "%s", dirp->d_name);
            gameName[0x40] = '\0';
            wiiChannels.push_back(GameContainer(gameName, tempPath, coverPath, configPath, cheatPath, gameId, gameIdU32));
        }

        fclose(fp);
    }
    closedir(dp);

    std::sort(wiiChannels.begin(), wiiChannels.end(), GameContainer::compare);
    loadWiiChannelsCovers();
}
