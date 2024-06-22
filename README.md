# deadbeef-lyricbar 歌词插件


## 效果展示

![效果展示](https://github.com/zhanxh/deadbeef-lyricbar/blob/master/deadbeef-show.png)

## 源码
[代码](https://github.com/loskutov/deadbeef-lyricbar)

[改良版](https://github.com/AsVHEn/deadbeef-lyricbar)


## 工作环境

```
       _,met$$$$$gg.          xxxxxx@xxx-pc 
    ,g$$$$$$$$$$$$$$$P.       ------------- 
  ,g$$P"     """Y$$.".        OS: Debian GNU/Linux 12 (bookworm) x86_64 
 ,$$P'              `$$$.     Host: X99-QD4 
',$$P       ,ggs.     `$$b:   Kernel: 6.1.0-21-amd64 
`d$$'     ,$P"'   .    $$$    Uptime: 2 hours, 20 mins 
 $$P      d$'     ,    $$P    Packages: 2212 (dpkg), 8 (snap) 
 $$:      $$.   -    ,d$$'    Shell: zsh 5.9 
 $$;      Y$b._   _,d$P'      Resolution: 1920x1080 
 Y$$.    `.`"Y$$$$P"'         DE: GNOME 43.9 
 `$$b      "-.__              WM: Mutter 
  `Y$$                        WM Theme: Adwaita 
   `Y$$.                      Theme: Adwaita [GTK2/3] 
     `$$b.                    Icons: Adwaita [GTK2/3] 
       `Y$$b.                 Terminal: gnome-terminal 
          `"Y$b._             CPU: Intel Xeon E5-2680 v4 (28) @ 3.300GHz 
              `"""            GPU: AMD ATI Radeon RX 6650 XT / 6700S / 6800S 
                              Memory: 5526MiB / 64135MiB 
```

## 编译依赖

```shell

sudo apt-get install libgtkmm-3.0-dev
sudo sudo apt install libcurl4-openssl-dev
sudo apt install libtag1-dev
sudo apt-get install autoconf automake libtool autopoint 
```

## 下载拷贝deadbeef头文件

下载
```shell
git clone https://github.com/DeaDBeeF-Player/deadbeef.git
```

然后
```
apt-get install autoconf automake libtool autopoin

./autogen.sh

./Configre
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
git clone https://github.com/zhanxh/deadbeef-lyricbar.git
```

- linux
```shell
make gtk3
```
- windows mingw64
```shell
make gtk3 -f MakefileWin
```

### 基于**改良版**修改

- 调整了歌词默认路径同歌曲目录
- 支持KG自动下载歌词保存到歌曲目录
- 支持KG的歌词搜索
- glade中的西班牙语改为英文，代码中原英文翻译改为中文翻译

## 存在问题

1. windows下插件导出函数正常的情况下，仍然没有被加载，原因还在看。有哪位知道原因，发一下原因，谢谢！

[00:00.00] 这是一个开始
[00:04.50] 歌词的第一行
[00:08.00] 接下来的歌词
[00:08.004] 接下来的歌词