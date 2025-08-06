# deadbeef-lyricbar 歌词插件

## 源码
[代码](https://github.com/loskutov/deadbeef-lyricbar)
[改良版](https://github.com/AsVHEn/deadbeef-lyricbar)
[upstream](https://github.com/zhanxh/deadbeef-lyricbar)

## 工作环境

```
                                      
                   -`                 
                  .o+`                
                 `ooo/                    ┌─────────── Hardware Information ───────────┐
                `+oooo:                  
               `+oooooo:                   
               -+oooooo+:                      Arch Linux x86_64   
             `/:-:++oooo+:                     Linux 6.11.5-arch1-1
            `/++++/+++++++:                    Hyprland (Wayland)  
           `/++++++++++++++:                   nushell 0.99.1      
          `/+++ooooooooooooo/`              
         ./ooosssso++osssssso+`             
        .oossssso-````/ossssss+`            
       -osssssso.      :ssssssso.           
      :osssssss/        osssso+++.          
     /ossssssss/        +ssssooo/-          
   `/ossssso+/:-        -:/+osssso+-      
  `+sso+:-`                 `.-/+oso:       
 `++:.                           `-/+/      
 .`                                 `/    └────────────────────────────────────────────┘
                                            ● ● ● ● ● ● ● ●
```

## 编译依赖
gtkmm3
libcurl
libtag
deadbeef

## 编译deadbeef-lyricbar

- linux
```shell
make gtk3
```

### 基于**upstream**修改
  - 减少了一些崩溃的可能
  - 添加了网易云歌词来源
  - 删除了保存到歌词文件和元数据的功能
  - 个人用于linux，不考虑windows是否可用
  - 按个人意见改变了歌词滚动方式