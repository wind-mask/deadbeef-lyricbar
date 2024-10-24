#pragma once
// #include <deadbeef/deadbeef.h>
#include "utils.h"
#ifdef __cplusplus
extern bool syncedlyrics;

struct parsed_lyrics music163(std::string artist, std::string song);

std::vector<std::string> music163_get_songs(std::string song,
                                            std::string artist);

struct parsed_lyrics music163_lyrics_downloader(std::string id);

#endif