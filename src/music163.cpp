#include "music163.h"
#include "json.hpp"
#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <string>
using json = nlohmann::json;
auto search_api = "https://music.163.com/api/search/get?s=";
auto search_config = "&type=1&offset=0&limit=1";
std::vector<std::string> music163_get_songs(std::string song,
                                            std::string artist) {
  auto search_url = search_api + url_encode(song + artist) + search_config;
  auto result = text_downloader(nullptr, search_url);
  auto j = json::parse(result);
  if (j.is_null()) {
    return {};
  }
  try {
    auto j1 = j.at("result");
    if (j1.is_null()) {
      return {};
    }
    auto j2 = j1.at("songs");
    if (j2.is_null() || !j2.is_array() || j2.size() == 0) {
      return {};
    }
    std::vector<std::string> songs;
    for (auto &song : j2) {
      auto name = song.at("name");
      auto artist = song.at("artists")[0].at("name");
      auto album = song.at("album").at("name");
      songs.push_back(artist.get<std::string>());
      songs.push_back(name.get<std::string>());
      songs.push_back(album.get<std::string>());
      int64_t id = song.at("id").get<int64_t>();
      songs.push_back(std::to_string(id));
    }
    return songs;
  } catch (std::exception &e) {
    return {};
  }
}
struct parsed_lyrics music163(std::string artist, std::string song) {
  auto songs = music163_get_songs(song, artist);
  if (songs.size() == 0) {
    return {"", false};
  }

  auto id = songs[3];
  auto lyrics = music163_lyrics_downloader(id);
  return lyrics;
}
struct parsed_lyrics music163_lyrics_downloader(std::string id) {
  auto lyrics_url = "https://music.163.com/api/song/media?id=" + id;
  auto lyrics_result = text_downloader(nullptr, lyrics_url);
  try {
    auto lyrics_json = json::parse(lyrics_result);
    if (lyrics_json.is_null() || lyrics_json.at("code").get<int>() != 200) {
      return {"", false};
    }
    auto lyrics = lyrics_json.at("lyric");
    if (lyrics.is_null() || !lyrics.is_string()) {
      return {"", false};
    }
    return {lyrics.get<std::string>(), true};
  } catch (std::exception &e) {
    return {"", false};
  }
}