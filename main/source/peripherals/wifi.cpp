#include <gccore.h>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"
#include "wifi.h"

extern "C" {
#include <tinysd.h>
}

//Wi-Fi SD host controller, uncached
#define SDHC_BASE 0xCD080000

//Delay function for tinySD
extern "C" void tsd_delay_us(uint32_t microseconds) {
    usleep(microseconds);
}

//Probe the Wi-Fi card once and cache its CIS ids
static bool identify(struct tsd_sdio_cis* cis) {
    static bool probed = false;
    static bool present = false;
    static struct tsd_sdio_cis found;

    //Probe the card
    if (!probed) {
        static struct tsd_sdhci sdhci;
        struct tsd_sdhci_config config = {.base = SDHC_BASE};

        int ret = tsd_sdhci_init(&sdhci, config);
        if (ret == 0)
            ret = tsd_sdio_identify(TSD_HOST(&sdhci), &found);
        present = (ret == 0);
        probed = true;
    }

    *cis = found;
    return present;
}

//Read the Wi-Fi card's manufacturer id, or 0 without a card
u16 Wifi::getManufacturer() {
    struct tsd_sdio_cis cis;

    if (!identify(&cis))
        return 0;
    return cis.manufacturer_id;
}

//Decide whether a Wi-Fi setting turns Wi-Fi on
bool Wifi::isEnabled(int setting) {
    char wdPath[64];

    //Turn Auto on for any card with Wi-Fi modules
    if (setting == WIFI_AUTO)
        return getManufacturer() == WIFI_MANUFACTURER_BROADCOM || getModulePath("wd.bin", wdPath, sizeof(wdPath));
    return setting == WIFI_ON;
}

//Find a Wi-Fi module for the fitted card
bool Wifi::getModulePath(const char* name, char* path, size_t len) {
    struct tsd_sdio_cis cis;

    if (!identify(&cis))
        return false;

    //Look in /rvloader/Hiidra/wifi/<manufacturer id>-<card id> first
    snprintf(path, len, "/rvloader/Hiidra/wifi/%04x-%04x/%s", cis.manufacturer_id, cis.card_id, name);
    if (fileExists(path))
        return true;

    //Fall back to /rvloader/Hiidra/wifi/<manufacturer id>
    snprintf(path, len, "/rvloader/Hiidra/wifi/%04x/%s", cis.manufacturer_id, name);
    return fileExists(path);
}
