#include <gccore.h>
#include <map>
#include <string>
#include <stdio.h>
#include <stdexcept>
#include "titles.h"
#include "wiitdb_txt.h"

namespace wiiTDB {
    std::map<std::string, std::string> database;

    void parse() {
        enum {
            SEEKING_GAMEID,
            READING_GAMEID,
            SEEKING_TITLE,
            READING_TITLE
        } state = SEEKING_GAMEID;
        char* tdb = (char*)wiitdb_txt;
        std::string gameId;
        std::string title;

        do {
            char c = *tdb++;

            switch (state) {
                case SEEKING_GAMEID:
                    if (!iscntrl(c)) {
                        gameId.push_back(c);
                        state = READING_GAMEID;
                    }
                break;

                case READING_GAMEID:
                    if (!isblank(c)) {
                        gameId.push_back(c);
                    } else {
                        state = SEEKING_TITLE;
                    }
                break;

                case SEEKING_TITLE:
                    if (!isblank(c) && c != '=') {
                        title.push_back(c);
                        state = READING_TITLE;
                    }
                break;

                case READING_TITLE:
                    if (!iscntrl(c)) {
                        title.push_back(c);
                    } else {
                        database.insert(std::pair<std::string, std::string> (gameId, title));
                        title.clear();
                        gameId.clear();
                        state = SEEKING_GAMEID;
                    }
                break;
            }
        } while (tdb != (char*)wiitdb_txt_end);
    }

    std::string getGameName(std::string gameId) {
        try {
            return database.at(gameId);
        } catch (std::out_of_range& e) {
            throw e;
        }
    }
}
