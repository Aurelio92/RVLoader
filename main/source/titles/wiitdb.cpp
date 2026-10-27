#include <gccore.h>
#include <bzlib.h>
#include <map>
#include <string>
#include <stdio.h>
#include <stdexcept>
#include "titles.h"
#include "wiitdb_txt_bz2.h"

namespace wiiTDB {
    std::map<std::string, std::string> database;

    void parse() {
        enum {
            SEEKING_GAMEID,
            READING_GAMEID,
            SEEKING_TITLE,
            READING_TITLE
        } state = SEEKING_GAMEID;

        bz_stream strm = {0};
        int bzret;
        unsigned int current_output_capacity = wiitdb_txt_bz2_size; // Initial guess for output buffer size
        char* tdb_orig = (char*)malloc(current_output_capacity);
        if (!tdb_orig) {
            throw std::runtime_error("Failed to allocate memory for decompression output.");
        }

        strm.bzalloc = NULL;
        strm.bzfree = NULL;
        strm.opaque = NULL;

        bzret = BZ2_bzDecompressInit(&strm, 0, 0); // verbosity = 0, small = 0
        if (bzret != BZ_OK) {
            free(tdb_orig);
            throw std::runtime_error("BZ2_bzDecompressInit failed.");
        }

        strm.next_in = (char*)wiitdb_txt_bz2;
        strm.avail_in = wiitdb_txt_bz2_size;
        strm.next_out = tdb_orig;
        strm.avail_out = current_output_capacity;

        do {
            bzret = BZ2_bzDecompress(&strm);

            if (bzret == BZ_OK && strm.avail_out == 0) {
                // Output buffer is full, reallocate
                unsigned int new_capacity = current_output_capacity * 2;
                char* new_tdb_orig = (char*)realloc(tdb_orig, new_capacity);
                if (!new_tdb_orig) {
                    BZ2_bzDecompressEnd(&strm);
                    free(tdb_orig);
                    throw std::runtime_error("Failed to reallocate memory for decompression output.");
                }
                tdb_orig = new_tdb_orig;
                strm.next_out = tdb_orig + current_output_capacity; // Move next_out to the end of old data
                strm.avail_out = new_capacity - current_output_capacity;
                current_output_capacity = new_capacity;
            } else if (bzret != BZ_OK && bzret != BZ_STREAM_END) {
                BZ2_bzDecompressEnd(&strm);
                free(tdb_orig);
                throw std::runtime_error("BZ2_bzDecompress failed with error: " + std::to_string(bzret));
            }
        } while (bzret != BZ_STREAM_END);

        BZ2_bzDecompressEnd(&strm);

        size_t tdb_size = strm.total_out_lo32; // Actual decompressed size
        char* tdb = tdb_orig;
        char* tdb_end = tdb + tdb_size;

        std::string gameID;
        std::string title;

        do {
            char c = *tdb++;

            switch (state) {
                case SEEKING_GAMEID:
                    if (!iscntrl(c)) {
                        gameID.push_back(c);
                        state = READING_GAMEID;
                    }
                break;

                case READING_GAMEID:
                    if (!isblank(c)) {
                        gameID.push_back(c);
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
                        database.insert(std::pair<std::string, std::string> (gameID, title));
                        title.clear();
                        gameID.clear();
                        state = SEEKING_GAMEID;
                    }
                break;
            }
        } while (tdb != (char*)tdb_end);

        free(tdb_orig);
    }

    std::string getGameName(std::string gameID) {
        try {
            return database.at(gameID);
        } catch (std::out_of_range& e) {
            throw e;
        }
    }
}
