#include "kugou.h"
#include "base64.h"
#include "json.hpp"
#include <chrono>
#include <iostream>
#include <regex>
#include <thread>

using json = nlohmann::json;

std::vector<std::string> str_split(std::string s, std::string delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }
  res.push_back(s.substr(pos_start));
  return res;
}

std::string LycrisDetial(std::string lycris) {
  std::string result_lycris = "";
  auto datas = str_split(lycris, "\n");
  for (auto &data : datas) {
    std::smatch match; // 搜索结果
    std::regex pattern("(\\[\\d\\d:\\d\\d.\\d\\d\\])");
    if (std::regex_search(data, match, pattern)) {
      result_lycris.append(data);
      result_lycris.append("\n");
    }
  }
  return result_lycris;
}

std::string urlencode(const std::string &url) {
  std::string encoded;
  for (unsigned char c : url) {
    // Perform URL encoding based on RFC 3986
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded.push_back(c); // Keep alphanumeric characters, '-', '_', '.', '~'
    } else {
      // Encode other characters as %xx
      encoded.append("%");
      char buffer[8] = {};        // 存储格式化的字符串，包括结尾的'\0'
      sprintf(buffer, "%02X", c); // 填充两位十六进制数，不足的前面补0
      encoded.append(buffer);
    }
  }
  return encoded;
}

std::vector<std::string> GetFileHashs(std::string artist, std::string song,
                                      struct curl_slist *headers) {
  std::vector<std::string> songs;

  std::string song_artist = song + "+" + artist;
  std::string encodeStr1 = urlencode(song_artist);
  std::string getFileHash =
      "https://songsearch.kugou.com/"
      "song_search_v2?filter=2&platform=WebFilter&keyword=" +
      encodeStr1 + "&pagesize=10&page=1";

  std::string fileHashContent = text_downloader(headers, getFileHash);
  auto j1 = json::parse(fileHashContent);
  auto j11 = j1.at("data");
  auto j12 = j11.at("lists");
  int size = j12.size();
  if (size > 10) {
    size = 10;
  }
  int i = 0;
  for (json::iterator it = j12.begin(); it != j12.end(); ++it, ++i) {
    json j13 = *it;
    std::string fileHash;
    j13.at("FileHash").get_to(fileHash);
    std::string songName;
    j13.at("SongName").get_to(songName);
    std::string albumName;
    j13.at("AlbumName").get_to(albumName);
    std::string singerName;
    j13.at("SingerName").get_to(singerName);
    // std::cout << fileHash << ", " << songName << ", " << albumName << ", " <<
    // singerName << std::endl;

    songs.push_back(singerName);
    songs.push_back(songName);
    songs.push_back(albumName);
    songs.push_back(fileHash);
    if (i >= size) {
      break;
    }
  }

  return songs;
}

std::string GetFileHash(std::string song, std::string artist,
                        struct curl_slist *headers) {
  std::string song_artist = song + "+" + artist;
  std::string encodeStr1 = urlencode(song_artist);
  std::string getFileHash =
      "https://songsearch.kugou.com/"
      "song_search_v2?filter=2&platform=WebFilter&keyword=" +
      encodeStr1 + "&pagesize=10&page=1";

  std::string fileHashContent = text_downloader(headers, getFileHash);
  auto j1 = json::parse(fileHashContent);
  auto j11 = j1.at("data");
  auto j12 = j11.at("lists");
  auto j13 = j12[0];
  std::string fileHash;
  j13.at("FileHash").get_to(fileHash);
  std::cout << fileHash << std::endl;
  return fileHash;
}

std::string GetAccesskey(std::string fileHash, struct curl_slist *headers,
                         std::string &id, std::string &accesskey) {
  string getAccesskey =
      "http://lyrics.kugou.com/search?ver=1&man=yes&client=mobi&hash=" +
      fileHash;

  std::string accesskeyContent = text_downloader(headers, getAccesskey);
  if (accesskeyContent.empty())
    return "";
  auto j2 = json::parse(accesskeyContent);
  auto j21 = j2["candidates"][0];
  if (j21.find("id") != j21.end()) {
    j21.at("id").get_to(id);
  }
  if (j21.find("accesskey") != j21.end()) {
    j21.at("accesskey").get_to(accesskey);
  }
  // std::cout << id << ", " << accesskey << std::endl;

  return "";
}

std::string GetLyrics(std::string id, std::string accesskey,
                      struct curl_slist *headers) {
  string getLyrics = "http://lyrics.kugou.com/"
                     "download?ver=1&client=iphone&fmt=lrc&charset=utf8&id=" +
                     id + "&accesskey=" + accesskey;

  auto result = text_downloader(headers, getLyrics);
  if (result.empty())
    return "";
  auto js = json::parse(result);
  std::string content;
  if (js.find("content") != js.end()) {
    js.at("content").get_to(content);
  }
  std::string data = base64_decode(content);
  std::string lyrics = LycrisDetial(data);
  return lyrics;
}

std::string down_lyrics(std::string artist, std::string song) {
  struct curl_slist *headers = nullptr;
  headers = curl_slist_append(
      headers,
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0)");
  headers = curl_slist_append(
      headers, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
  headers = curl_slist_append(
      headers, "Accept:application/json, text/javascript, */*; q=0.01");
  headers = curl_slist_append(headers, "Accept-Language:zh-CN,zh;q=0.8");
  std::string album = song;
  std::string fileHash = GetFileHashs(artist, song, headers)[3];
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::string id;
  std::string accesskey;
  GetAccesskey(fileHash, headers, id, accesskey);
  std::this_thread::sleep_for(std::chrono::milliseconds(600));
  std::string lyrics = GetLyrics(id, accesskey, headers);

  return lyrics;
}

std::vector<std::string> kugou_get_songs(std::string song, std::string artist) {
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(
      headers,
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0)");
  headers = curl_slist_append(
      headers, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
  headers = curl_slist_append(
      headers, "Accept:application/json, text/javascript, */*; q=0.01");
  headers = curl_slist_append(headers, "Accept-Language:zh-CN,zh;q=0.8");

  return GetFileHashs(artist, song, headers);
}

struct parsed_lyrics kugou(std::string artist, std::string song) {
  struct parsed_lyrics string_lyrics = {"", true};

  std::string lyrics = down_lyrics(artist, song);
  string_lyrics.lyrics = lyrics;

  return string_lyrics;
}

struct parsed_lyrics kugou_lyrics_downloader(std::string fileHash) {
  struct parsed_lyrics string_lyrics = {"", false};

  struct curl_slist *headers = nullptr;
  headers = curl_slist_append(
      headers,
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0)");
  headers = curl_slist_append(
      headers, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
  headers = curl_slist_append(
      headers, "Accept:application/json, text/javascript, */*; q=0.01");
  headers = curl_slist_append(headers, "Accept-Language:zh-CN,zh;q=0.8");
  std::string id;
  std::string accesskey;
  GetAccesskey(fileHash, headers, id, accesskey);
  std::this_thread::sleep_for(std::chrono::milliseconds(600));
  std::string lyrics = GetLyrics(id, accesskey, headers);
  string_lyrics.lyrics = lyrics;

  return string_lyrics;
}