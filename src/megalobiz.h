// #include <deadbeef/deadbeef.h>
// #include <gtk/gtk.h>

// #include "lrcspotify.h"
#include "utils.h"
#ifdef __cplusplus

using namespace std;

extern bool syncedlyrics;

vector<string> megalobiz_get_songs(string song, string artist);

struct parsed_lyrics megalobiz(string artist, string song);

struct parsed_lyrics megalobiz_lyrics_downloader(string trackid);

#endif
