#pragma once
#include <deadbeef/deadbeef.h>
#include "utils.h"

#include <gtk/gtk.h>

#ifdef __cplusplus
#include <vector>
#include <string>


// using namespace std;

extern bool syncedlyrics;

std::vector<std::string> kugou_get_songs(std::string song, std::string artist);

struct parsed_lyrics kugou(std::string artist, std::string song);

struct parsed_lyrics kugou_lyrics_downloader(std::string fileHash);

#endif
