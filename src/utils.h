#pragma once
#ifndef LYRICBAR_UTILS_H
#define LYRICBAR_UTILS_H

#include <deadbeef/deadbeef.h>
#include <curl/curl.h>


#include "main.h"
#include "ui.h"

#ifndef __cplusplus
#include <stdbool.h>
#else

#include <string>
#include <vector>

using namespace std;

struct id3v2_tag {
	DB_id3v2_tag_t tag{};
	~id3v2_tag() { deadbeef->junk_id3v2_free(&tag); }
};

struct parsed_lyrics{
	std::string lyrics;
	bool sync;
};

struct sync{
	vector<std::string> synclyrics;
	vector<double> position;
};

struct chopped{
	std::string past;
	std::string present;
	std::string future;
};


std::string specialforplus(const char* text);

bool is_playing(DB_playItem_t *track);

void update_lyrics(void *tr);

struct parsed_lyrics get_lyrics_from_metadata(DB_playItem_t *track);

void save_meta_data(DB_playItem_t *playing_song, struct parsed_lyrics lyrics);
void save_next_to_file(struct parsed_lyrics lyrics, DB_playItem_t *track);

int mkpath(const std::string &name, mode_t mode);

std::string replace_string(std::string subject, const std::string& search, const std::string& replace);

vector<std::string> split(std::string s, std::string delimiter);

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

std::string text_downloader(curl_slist *slist, std::string url);

std::string url_encode(const std::string& url);

inline std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

inline std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

inline std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    return ltrim(rtrim(str, chars), chars);
}

extern "C" {
#endif // __cplusplus
int remove_from_cache_action(DB_plugin_action_t *, ddb_action_context_t ctx);
bool is_cached(const char *artist, const char *title);
void ensure_lyrics_path_exists();

#ifdef __cplusplus
}
#endif

#endif // LYRICBAR_UTILS_H
