#pragma once

#include <string>
#include <gccore.h>
#include <libgui.h>
#include <filesystem>
#include "gcsave.h"
#include "rvlmutex.h"
#include "rvltrigger.h"

#define WBFS_MAGIC          0x57424653 //"WBFS"
#define WBFS_BASE_FOLDER    "/wbfs"
#define COVER_PATH          "/rvloader/covers"
#define DUMMY_COVER_PATH    "/rvloader/covers/dummy.png"
#define CONFIG_PATH         "/rvloader/configs"

typedef struct {
    bool ahbAccess;
} hbMeta;

class GameContainer {
    public:
        time_t lastModified;
        std::string name;
        std::string path;
        std::string coverPath;
        std::string confPath;
        std::string cheatPath;
        std::string gameIDString;
        u32 gameID;
        GuiImage* image;
        GCSave save;

        GameContainer() {}
        GameContainer(time_t _lastModified, std::string _name, std::string _path, std::string _coverPath, std::string _confPath, std::string _gameIDString, u32 _gameID):
            lastModified(_lastModified), name(_name), path(_path), coverPath(_coverPath), confPath(_confPath), gameIDString(_gameIDString), gameID(_gameID), image(NULL) {}

        static bool compare(const GameContainer& gc1, const GameContainer& gc2) {
            const char* buffer1 = gc1.name.c_str();
            const char* buffer2 = gc2.name.c_str();

            while (*buffer1 && *buffer2) {
                u32 charLen1, charLen2;
                FT_ULong c1 = Font::getCharUTF8(buffer1, &charLen1);
                FT_ULong c2 = Font::getCharUTF8(buffer2, &charLen2);
                buffer1 += charLen1;
                buffer2 += charLen2;

                if (tolower(c1) != tolower(c2)) {
                    return tolower(c1) < tolower(c2);
                }
            }

            //The first string comes first if it reached the end before
            return (buffer1 == NULL) && (buffer2 != NULL);
        }
};

class HBContainer {
    public:
        std::string name;
        std::string path;
        std::string coverPath;
        hbMeta meta;
        std::string confPath;
        GuiImage* image;

        HBContainer(std::string _name, std::string _path, std::string _coverPath, hbMeta _meta, std::string _confPath) : name(_name), path(_path), coverPath(_coverPath), meta(_meta), confPath(_confPath), image(NULL) {}

        static bool compare(HBContainer hbc1, HBContainer hbc2) {
            const char* buffer1 = hbc1.name.c_str();
            const char* buffer2 = hbc2.name.c_str();

            while (*buffer1 && *buffer2) {
                u32 charLen1, charLen2;
                FT_ULong c1 = Font::getCharUTF8(buffer1, &charLen1);
                FT_ULong c2 = Font::getCharUTF8(buffer2, &charLen2);
                buffer1 += charLen1;
                buffer2 += charLen2;

                if (tolower(c1) != tolower(c2)) {
                    return tolower(c1) < tolower(c2);
                }
            }

            //The first string comes first if it reached the end before
            return (buffer1 == NULL) && (buffer2 != NULL);
        }
};

class GamesDatabase {
    protected:
        std::string dbName;
        std::string scanPath;
        std::string cachePath;
        std::string targetNameOrExtension;
        bool recursiveScan;
        bool scanningForDirectories;
        u8* scanAndUpdateThreadStack;
        lwp_t scanAndUpdateThreadHandle;
        u8* coversThreadStack;
        lwp_t coversThreadHandle;
        RVLMutex dbMutex;
        RVLTrigger hasFinishedScanning;
        void handleDirEntryForScan(const std::filesystem::directory_entry& dir_entry, std::unordered_map<std::string, GameContainer*>& lookup, std::vector<GameContainer>& newGames, std::vector<std::string>& verifiedPaths);
    public:
        std::vector<GameContainer> games;
        GamesDatabase() {
            dbName = "";
            scanPath = "";
            cachePath = "";
            targetNameOrExtension = "";
            recursiveScan = false;
            scanningForDirectories = false;
            
            scanAndUpdateThreadStack = NULL;
            coversThreadStack = NULL;
            games.reserve(10000);
        }
        GamesDatabase(std::string _scanPath, std::string _cachePath, std::string _targetNameOrExtension, bool _recursiveScan = false, bool _scanningForDirectories = false) {
            scanPath = _scanPath;
            cachePath = _cachePath;
            targetNameOrExtension = _targetNameOrExtension;
            recursiveScan = _recursiveScan;
            scanningForDirectories = _scanningForDirectories;

            scanAndUpdateThreadStack = NULL;
            coversThreadStack = NULL;
        }

        void lock() { dbMutex.lock(); }
        void unlock() { dbMutex.unlock(); }
        
        bool readCache();
        bool writeCache();
        void scanGames();
        void startScanAndUpdateThread();
        static void* scanAndUpdateThread(void* arg);
        static void* loadCoversThread(void* arg);
        bool hasFinishedScanningGames() { return hasFinishedScanning.check(); }
        virtual void createGameContainer(std::vector<GameContainer>& newGames, time_t lastModified, const std::string& path) { }
        virtual void addMetadataToGameContainer(GameContainer& gc) { }
};

class GamesDatabaseGC : public GamesDatabase {
    public:
        GamesDatabaseGC() {
            dbName = "GC";
            scanPath = "/games";
            cachePath = "/rvloader/gc_game_cache.tsv";
            targetNameOrExtension = "game.iso";
            recursiveScan = false;
            scanningForDirectories = true;
        }
        void createGameContainer(std::vector<GameContainer>& newGames, time_t lastModified, const std::string& path) override;
        void addMetadataToGameContainer(GameContainer& gc) override;
};

class GamesDatabaseWii : public GamesDatabase {
    public:
        GamesDatabaseWii() {
            dbName = "Wii";
            scanPath = "/wbfs";
            cachePath = "/rvloader/wii_game_cache.tsv";
            targetNameOrExtension = ".wbfs";
            recursiveScan = true;
            scanningForDirectories = false;
        }
        void createGameContainer(std::vector<GameContainer>& newGames, time_t lastModified, const std::string& path) override;
        void addMetadataToGameContainer(GameContainer& gc) override;
};

class GamesDatabaseWAD : public GamesDatabase {
    public:
        GamesDatabaseWAD() {
            dbName = "WAD";
            scanPath = "";
            cachePath = "";
            targetNameOrExtension = ".wad";
            recursiveScan = false;
            scanningForDirectories = false;
        }
        void createGameContainer(std::vector<GameContainer>& newGames, time_t lastModified, const std::string& path) override;
        void addMetadataToGameContainer(GameContainer& gc) override;
};

class GamesDatabaseVC : public GamesDatabaseWAD {
    public:
        GamesDatabaseVC() {
            dbName = "VC";
            scanPath = "/vc";
            cachePath = "/rvloader/vc_game_cache.tsv";
            targetNameOrExtension = ".wad";
            recursiveScan = false;
            scanningForDirectories = false;
        }
};

class GamesDatabaseChannels : public GamesDatabaseWAD {
    public:
        GamesDatabaseChannels() {
            dbName = "Channels";
            scanPath = "/channels";
            cachePath = "/rvloader/channels_game_cache.tsv";
            targetNameOrExtension = ".wad";
            recursiveScan = false;
            scanningForDirectories = false;
        }
};

extern GamesDatabaseGC gcGamesDatabase;
extern GamesDatabaseWii wiiGamesDatabase;
extern GamesDatabaseVC vcGamesDatabase;
extern GamesDatabaseChannels wiiChannelsDatabase;

extern std::vector<HBContainer> wiiHomebrews;

namespace wiiTDB {
    void parse();
    std::string getGameName(std::string gameID);
}

void lockTitlesDBs();
void unlockTitlesDBs();
void addWiiGames();
void addGCGames();
void addVCGames();
void addWiiChannels();
void addWiiHomebrews();
