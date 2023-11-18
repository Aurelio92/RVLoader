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

#define THREAD_STACK_SIZE 16384

std::vector<HBContainer> wiiHomebrews;


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

void addWiiHomebrews() {
    char dolPath[PATH_MAX];
    char tempPath[PATH_MAX];
    struct dirent *dirp;
    hbMeta meta;
    std::string hbName;

    DIR* dp = opendir("/apps");
    if (dp == NULL)
        return;

    while ((dirp = readdir(dp)) != NULL) {
        FILE* fp;
        char coverPath[PATH_MAX];
        char configPath[PATH_MAX];

        if (dirp->d_name == NULL)
            continue;

        if (dirp->d_name[0] == '.')
            continue;

        if (dirp->d_type != DT_DIR)
            continue;

        //Try opening boot.dol. If fails, try boot.elf
        sprintf(dolPath, "/apps/%s/boot.dol", dirp->d_name);
        fp = fopen(dolPath, "rb");
        if (fp == NULL) {
            sprintf(dolPath, "/apps/%s/boot.elf", dirp->d_name);
            fp = fopen(dolPath, "rb");
            if (fp == NULL)
                continue;
        }
        fclose(fp);

        hbName = "";
        meta.ahbAccess = false;
        sprintf(tempPath, "/apps/%s/meta.xml", dirp->d_name);
        fp = fopen(tempPath, "rb");
        if (fp != NULL) {
            fclose(fp);
            parseMetaXML(tempPath, &meta, &hbName);
        }

        if (hbName == "")
            hbName = dirp->d_name;

        sprintf(coverPath, "/apps/%s/icon.png", dirp->d_name);
        FILE* coverFP = fopen(coverPath, "rb");
        if (coverFP)
            fclose(coverFP);
        else {
            sprintf(coverPath, "%s/dummyHB.png", COVER_PATH);
            coverFP = fopen(coverPath, "rb");
            if (coverFP) {
                fclose(coverFP);
            } else {
                coverPath[0] = '\0';
            }
        }

        sprintf(configPath, "/apps/%s/config.cfg", dirp->d_name);

        wiiHomebrews.push_back(HBContainer(hbName, dolPath, coverPath, meta, configPath));
    }
    closedir(dp);
}
