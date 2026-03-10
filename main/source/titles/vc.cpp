#include "titles.h"

GamesDatabaseVC vcGamesDatabase;

void addVCGames() {
    vcGamesDatabase.startScanAndUpdateThread();
}
