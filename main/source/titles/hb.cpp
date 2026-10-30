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
#include <mxml.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <sys/stat.h>
#include "rvlmutex.h"
#include "utils.h"
#include "main.h"

#define THREAD_STACK_SIZE 16384

std::vector<HBContainer> wiiHomebrews;
RVLMutex wiiHomebrewsMutex;
RVLTrigger wiiHomebrewsFinishedScanning;


static std::string getElemCDATA(mxml_node_t* node, const char* element) {
    if (!node)
        return "";

    mxml_node_t* n = mxmlGetFirstChild(mxmlFindElement(node, node, element,
                                        NULL, NULL, MXML_DESCEND_FIRST));

    while (n) {
        if ((mxmlGetType(n) == MXML_OPAQUE) && mxmlGetOpaque(n))
            return std::string(mxmlGetOpaque(n));

        n = mxmlWalkNext(n, node, MXML_NO_DESCEND);
    }

    return "";
}

static bool parseMetaXML(const char* filepath, hbMeta* meta, std::string* name) {
    mxml_node_t* node;

    *name = "";
    meta->ahbAccess = false;

    FILE* fp = fopen(filepath, "r");
    if (fp == NULL)
        return false;

    mxml_node_t* xmlTree = mxmlLoadFile(NULL, fp, MXML_OPAQUE_CALLBACK);

    node = mxmlFindElement(xmlTree, xmlTree, "app", NULL, NULL, MXML_DESCEND_FIRST);

    if (!node) { //No app node
        mxmlDelete(xmlTree);
        fclose(fp);
        return false;
    }

    *name = getElemCDATA(node, "name");

    if (mxmlFindElement(node, node, "ahb_access", NULL, NULL, MXML_DESCEND_FIRST))
        meta->ahbAccess = true;
    if (mxmlFindElement(node, node, "no_ios_reload", NULL, NULL, MXML_DESCEND_FIRST))
        meta->ahbAccess = true;

    mxmlDelete(xmlTree);

    fclose(fp);

    return true;
}

#define HB_CACHE_PATH "/rvloader/hb_cache.tsv"

static u8* hbScanThreadStack = NULL;
static lwp_t hbScanThreadHandle = LWP_THREAD_NULL;

static time_t getLastModified(const char* path) {
    struct stat result;
    return (stat(path, &result) == 0) ? result.st_mtime : 0;
}

static std::string getHBCoverPath(const std::string& appDir) {
    std::string coverPath = appDir + "/icon.png";
    if (fileExists(coverPath))
        return coverPath;
    coverPath = std::string(COVER_PATH) + "/dummyHB.png";
    return fileExists(coverPath) ? coverPath : "";
}

static bool readHBCache() {
    std::ifstream file(HB_CACHE_PATH);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::stringstream ss(line);
        std::string name, path, coverPath, ahbAccessStr, lastModifiedStr;
        if (std::getline(ss, name, '\t') &&
            std::getline(ss, path, '\t') &&
            std::getline(ss, coverPath, '\t') &&
            std::getline(ss, ahbAccessStr, '\t') &&
            std::getline(ss, lastModifiedStr, '\t')) {
            std::string appDir = path.substr(0, path.find_last_of('/'));
            hbMeta meta;
            meta.ahbAccess = ahbAccessStr == "1";
            if (!fileExists(coverPath))
                coverPath = getHBCoverPath(appDir);
            wiiHomebrews.emplace_back(name, path, coverPath, meta, appDir + "/config.cfg");
            wiiHomebrews.back().lastModified = std::stoll(lastModifiedStr);
        }
    }

    return true;
}

static bool writeHBCache() {
    std::ofstream file(HB_CACHE_PATH);
    if (!file.is_open()) return false;

    for (HBContainer& hb : wiiHomebrews) {
        file << hb.name << "\t" << hb.path << "\t" << hb.coverPath << "\t" << hb.meta.ahbAccess << "\t" << hb.lastModified << "\n";
    }

    return true;
}

static void scanHomebrews() {
    std::unordered_map<std::string, HBContainer> lookup;
    std::vector<HBContainer> newHomebrews;
    struct dirent *dirp;

    DIR* dp = opendir("/apps");
    if (dp == NULL)
        return;

    //Copy the cached entries so the scan can run without holding the lock
    {
        std::lock_guard<RVLMutex> lock(wiiHomebrewsMutex);
        for (auto& hb : wiiHomebrews) {
            lookup.emplace(hb.path, hb);
        }
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (titlesThreadsStopping)
            break;
        if (dirp->d_name[0] == '.' || dirp->d_type != DT_DIR)
            continue;

        std::string appDir = std::string("/apps/") + dirp->d_name;

        //Look for boot.dol. If missing, look for boot.elf
        std::string dolPath = appDir + "/boot.dol";
        time_t dolLastModified = getLastModified(dolPath.c_str());
        if (dolLastModified == 0) {
            dolPath = appDir + "/boot.elf";
            dolLastModified = getLastModified(dolPath.c_str());
            if (dolLastModified == 0)
                continue;
        }

        std::string metaPath = appDir + "/meta.xml";
        time_t lastModified = std::max(dolLastModified, getLastModified(metaPath.c_str()));
        std::string coverPath = getHBCoverPath(appDir);

        //Skip parsing meta.xml if neither it nor the dol changed
        auto it = lookup.find(dolPath);
        if (it != lookup.end() && it->second.lastModified == lastModified) {
            HBContainer& hb = it->second;
            newHomebrews.emplace_back(hb.name, dolPath, coverPath, hb.meta, hb.confPath);
            newHomebrews.back().lastModified = lastModified;
            continue;
        }

        hbMeta meta;
        std::string hbName;
        meta.ahbAccess = false;
        parseMetaXML(metaPath.c_str(), &meta, &hbName);

        if (hbName == "")
            hbName = dirp->d_name;

        newHomebrews.emplace_back(hbName, dolPath, coverPath, meta, appDir + "/config.cfg");
        newHomebrews.back().lastModified = lastModified;
    }
    closedir(dp);
    if (titlesThreadsStopping)
        return;

    std::lock_guard<RVLMutex> lock(wiiHomebrewsMutex);

    //Keep already loaded icons whose path did not change, free the others
    std::unordered_map<std::string, HBContainer*> newLookup;
    for (auto& hb : newHomebrews) {
        newLookup[hb.path] = &hb;
    }
    for (auto& hb : wiiHomebrews) {
        if (hb.image == NULL)
            continue;
        auto it = newLookup.find(hb.path);
        if (it != newLookup.end() && it->second->coverPath == hb.coverPath) {
            it->second->image = hb.image;
        } else if (hb.image != dummyHBIcon) { //Shared global, must never be freed
            delete hb.image;
        }
    }

    wiiHomebrews.swap(newHomebrews);
    writeHBCache();
}

static void* hbScanThread(void* arg) {
    scanHomebrews();
    wiiHomebrewsFinishedScanning.send();
    return NULL;
}

void addWiiHomebrews() {
    readHBCache();
    hbScanThreadStack = (u8*)memalign(32, THREAD_STACK_SIZE);
    LWP_CreateThread(&hbScanThreadHandle, hbScanThread, NULL, hbScanThreadStack, THREAD_STACK_SIZE, 30);
}

void joinHomebrewsThread() {
    if (hbScanThreadHandle != LWP_THREAD_NULL)
        LWP_JoinThread(hbScanThreadHandle, NULL);
}
