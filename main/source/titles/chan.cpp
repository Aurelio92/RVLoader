#include "titles.h"

GamesDatabaseChannels wiiChannelsDatabase;

void addWiiChannels() {
    wiiChannelsDatabase.startScanAndUpdateThread();
}
