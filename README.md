# deadbeef-lyricbar 优化


## 效果展示

![效果展示](https://github.com/zhanxh/deadbeef-lyricbar/blob/master/deadbeef-show.png)

## 源码
[代码](https://github.com/loskutov/deadbeef-lyricbar)

[改良版](https://github.com/AsVHEn/deadbeef-lyricbar)


## 工作环境

> Ubuntu 22.04.3 LTS

## 编译依赖

```shell

sudo apt-get install libgtkmm-3.0-dev
sudo apt-get install libcurl5-openssl-dev

```

## 下载拷贝deadbeef头文件

下载
```shell
git clone https://github.com/DeaDBeeF-Player/deadbeef.git
```


```shell
# 拷贝头文件
cd deadbeef/include
sudo cp -r deadbeef /usr/local/include
# 拷贝GTKui头文件
cd deadbeef/plugins/gtkui
sudo cp *.h /usr/local/include/deadbeef 
```

## 编译deadbeef-lyricbar

```shell
git clone https://github.com/loskutov/deadbeef-lyricbar.git
```

```shell
make gtk3
```
### 基于**改良版**修改

```C++
#include <sstream>
```


```c++

// Function to parse synced lyrics:
struct sync lyric2vector( string lyrics){
	//cout << "lyric2vector" "\n";
	vector<string> synclyrics;
	vector<double> position;
	string line;
	int squarebracket;
	int repeats = 0; 
	//If last character is a ] add an space to have same number of lyrics and positions.
	if (lyrics.at(lyrics.length() - 1) == ']'){
		lyrics.push_back(' ');
	}
	if (lyrics.length() <= 3){
		position.push_back(0);
		struct sync  emptylyrics = bubbleSort(position, synclyrics, 1);
		return emptylyrics;
	}

	for (unsigned i=0; i < lyrics.length() - 3; ++i){
		if ((lyrics.at(i) == '[') && (lyrics.at(i+1) == '0') && (lyrics.at(i+3) == ':') ) {
  			++repeats;
			position.push_back ((lyrics.at(i + 1) - 48)*600 + (lyrics.at(i + 2) - 48)*60 + (lyrics.at(i + 4) - 48)*10 + (lyrics.at(i + 5) - 48) + (lyrics.at(i + 7) - 48)*0.1 + (lyrics.at(i + 8) - 48)*0.01);
			if ((lyrics.length() > i + 10) && (lyrics.at(i+10) != '[')) {
				line = lyrics.substr(i + 10, lyrics.length() - i - 10);
				squarebracket = line.find_first_of('[');
				if (((lyrics.at(i+8 + squarebracket)) != '\n') && (lyrics.at(i+8 + squarebracket)) != '\r'){
					if(lyrics.at(i+10) == ']') // do兼容毫秒3位
						line = lyrics.substr(i + 11, squarebracket-1);
					else
						line = lyrics.substr(i + 10, squarebracket-1);
				}
				else {
					if(lyrics.at(i+10) == ']') // do兼容毫秒3位
						line = lyrics.substr(i + 11, squarebracket-1);
					else
						line = lyrics.substr(i + 10, squarebracket-2);
				}
				++repeats;
				while (--repeats ) {
					synclyrics.push_back (line);
				}
					
			}
		}
	}

	int n = position.size();
	struct sync  goodlyrics = bubbleSort(position, synclyrics, n);

// For debug:
//	for (auto k = goodlyrics.position.begin(); k != goodlyrics.position.end(); ++k)
//    	cout << *k << " ";
//
//	for (auto i = goodlyrics.synclyrics.begin(); i != goodlyrics.synclyrics.end(); ++i)
//    	cout << *i << '\n';
	
	return goodlyrics;
}

```

```C++

struct parsed_lyrics get_lyrics_next_to_file(DB_playItem_t *track) {
	bool sync = false;
	ifstream infile;
	string lyrics;
	std::stringstream buffer;

	deadbeef->pl_lock();
	const char *track_location = deadbeef->pl_find_meta(track, ":URI");
	deadbeef->pl_unlock();
	string trackstring = track_location;
	size_t lastindex = trackstring.find_last_of(".");
	trackstring = trackstring.substr(0, lastindex);
	
	std::cout << "track_location: " << track_location << std::endl;
	std::cout << "trackstring: " << trackstring << std::endl;

	infile.open(trackstring + ".lrc", ios_base::app); //ios_base::trunc
	if(infile.is_open()){
		std::stringstream buffer;
		buffer << infile.rdbuf();
		lyrics = buffer.str();
		sync = true;	
		std::cout << "file exist : " << trackstring << ".lrc" << std::endl;
		std::cout << "lyrics1: " << lyrics << std::endl;
	}
	else
	{
		infile.open(trackstring + ".txt", ios_base::app); //ios_base::trunc
		if(infile.is_open()){
			std::stringstream buffer;
			buffer << infile.rdbuf();
			lyrics = buffer.str();
			std::cout << "lyrics2: "<< lyrics << std::endl;
		}
		else{
			std::cout << "file not exist : " << trackstring << ".txt" << std::endl;
		}
	}
	return{lyrics, sync};	
}
```

