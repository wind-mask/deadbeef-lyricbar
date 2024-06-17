#include "kugou.h"
#include "json.hpp"
#include "base64.h"
#include <chrono>
#include <thread>

using json = nlohmann::json;

std::string urlencode(const std::string& url) {
    std::string encoded;
    for (unsigned char c : url) {
        // Perform URL encoding based on RFC 3986
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded.push_back(c); // Keep alphanumeric characters, '-', '_', '.', '~'
        }
        else {
            // Encode other characters as %xx
            encoded.append("%");
            char buffer[8] = {}; // 存储格式化的字符串，包括结尾的'\0'
            sprintf(buffer, "%02X", c); // 填充两位十六进制数，不足的前面补0
            encoded.append(buffer);
        }
    }
    return encoded;
}


std::vector<std::string> GetFileHashs(std::string artist, std::string song, struct curl_slist* headers)
{
    std::vector<std::string> songs;

    std::string song_artist = song + "+" + artist;
    std::string encodeStr1 = urlencode(song_artist);
    std::string getFileHash = "https://songsearch.kugou.com/song_search_v2?filter=2&platform=WebFilter&keyword=" + encodeStr1 + "&pagesize=10&page=1";

    std::string fileHashContent = text_downloader(headers, getFileHash);
    auto j1 = json::parse(fileHashContent);
    auto j11 = j1.at("data");
    auto j12 = j11.at("lists");
    int size = j12.size();
    if(size>10) {
        size = 10;
    }
    int i = 0;
    for (json::iterator it = j12.begin(); it != j12.end(); ++it, ++i)
    {
        json j13 = *it;
        std::string fileHash;
        j13.at("FileHash").get_to(fileHash);
        std::string songName;
        j13.at("SongName").get_to(songName);
        std::string albumName;
        j13.at("AlbumName").get_to(albumName);
        std::string singerName;
        j13.at("SingerName").get_to(singerName);
        // cout << fileHash << endl;

        songs.push_back(singerName);
        songs.push_back(songName);
        songs.push_back(albumName);
        songs.push_back(fileHash);
        if(i >= size) {
            break;
        }
           
    }
    
    return songs;
}

void GetAccesskey(std::string fileHash, struct curl_slist* headers, std::string& id, std::string& accesskey)
{
    std::string getAccesskey = "http://lyrics.kugou.com/search?ver=1&man=yes&client=mobi&hash=" + fileHash;

    std::string accesskeyContent = text_downloader(headers, getAccesskey);
    auto j2 = json::parse(accesskeyContent);
    auto j21 = j2["candidates"][0];

    j21.at("id").get_to(id);
    j21.at("accesskey").get_to(accesskey);
    // std::cout << id << ", " << accesskey << std::endl;
}

std::string GetLyrics(std::string id, std::string accesskey, struct curl_slist* headers)
{
    std::string getLyrics = "http://lyrics.kugou.com/download?ver=1&client=iphone&fmt=lrc&charset=utf8&id=" + id + "&accesskey=" + accesskey;

    auto result = text_downloader(headers, getLyrics);
    auto js = json::parse(result);
    std::string content;
    js.at("content").get_to(content);
    std::string data = base64_decode(content);

    return data;
}

std::string down_lyrics(std::string artist, std::string song)
{
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0)");
    headers = curl_slist_append(headers, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
    headers = curl_slist_append(headers, "Accept:application/json, text/javascript, */*; q=0.01");
    headers = curl_slist_append(headers, "Accept-Language:zh-CN,zh;q=0.8");
    std::string album = song;
    std::string fileHash = GetFileHashs(artist, song, headers)[0];
    std::this_thread::sleep_for(std::chrono::microseconds(300));
    std::string id;
    std::string accesskey;
    GetAccesskey(fileHash, headers, id, accesskey);
    std::this_thread::sleep_for(std::chrono::microseconds(300));
    std::string lyrics = GetLyrics(id, accesskey, headers);

    return lyrics;
}


std::vector<std::string> kugou_get_songs(std::string song, std::string artist)
{
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0)");
    headers = curl_slist_append(headers, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
    headers = curl_slist_append(headers, "Accept:application/json, text/javascript, */*; q=0.01");
    headers = curl_slist_append(headers, "Accept-Language:zh-CN,zh;q=0.8");

    return GetFileHashs(artist, song, headers);
}

struct parsed_lyrics kugou(std::string artist, std::string song)
{
    struct parsed_lyrics string_lyrics = {"", false};

    std::string lyrics = down_lyrics(artist, song);
    string_lyrics.lyrics = lyrics;

    return string_lyrics;
}

struct parsed_lyrics kugou_lyrics_downloader(std::string  fileHash)
{
    struct parsed_lyrics string_lyrics = {"", false};

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:76.0)");
    headers = curl_slist_append(headers, "Content-Type:application/x-www-form-urlencoded; charset=UTF-8");
    headers = curl_slist_append(headers, "Accept:application/json, text/javascript, */*; q=0.01");
    headers = curl_slist_append(headers, "Accept-Language:zh-CN,zh;q=0.8");
    std::string id;
    std::string accesskey;
    GetAccesskey(fileHash, headers, id, accesskey);
    std::this_thread::sleep_for(std::chrono::microseconds(300));
    std::string lyrics = GetLyrics(id, accesskey, headers);
    string_lyrics.lyrics = lyrics;

    return string_lyrics;
}