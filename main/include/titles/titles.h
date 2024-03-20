#pragma once

#include <string>
#include <gccore.h>
#include <libgui.h>

#define WBFS_MAGIC          0x57424653 //"WBFS"
#define WBFS_BASE_FOLDER    "/wbfs"
#define COVER_PATH          "/rvloader/covers"
#define CONFIG_PATH         "/rvloader/configs"

typedef struct {
    bool ahbAccess;
} hbMeta;

class GameContainer {
    public:
        std::string name;
        std::string path;
        std::string coverPath;
        std::string confPath;
        std::string cheatPath;
        std::string gameIDString;
        u32 gameID;
        GuiImage* image;

        GameContainer(std::string _name, std::string _path, std::string _coverPath, std::string _confPath, std::string _cheatPath, std::string _gameIDString, u32 _gameID) : name(_name), path(_path), coverPath(_coverPath), confPath(_confPath), cheatPath(_cheatPath), gameIDString(_gameIDString), gameID(_gameID), image(NULL) {}

        static bool compare(GameContainer gc1, GameContainer gc2) {
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

extern std::vector<GameContainer> wiiGames;
extern std::vector<GameContainer> gcGames;
extern std::vector<GameContainer> vcGames;
extern std::vector<GameContainer> wiiChannels;
extern std::vector<HBContainer> wiiHomebrews;

extern mutex_t wiiCoversMutex;
extern mutex_t gcCoversMutex;
extern mutex_t vcCoversMutex;
extern mutex_t wiiChanCoversMutex;

namespace wiiTDB {
    void parse();
    std::string getGameName(std::string gameId);
}

void addWiiGames();
void addGCGames();
void addVCGames();
void addWiiChannels();
void addWiiHomebrews();
