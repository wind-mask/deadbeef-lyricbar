
default:
    @just --list

install:make
    sudo rm /usr/lib/deadbeef/ddb_lyricbar_gtk3.so
    sudo mv ddb_lyricbar_gtk3.so /usr/lib/deadbeef/
    sudo chmod 644 /usr/lib/deadbeef/ddb_lyricbar_gtk3.so
install-debug:make-debug
    sudo rm /usr/lib/deadbeef/ddb_lyricbar_gtk3.so
    sudo mv ddb_lyricbar_gtk3.so /usr/lib/deadbeef/
    sudo chmod 644 /usr/lib/deadbeef/ddb_lyricbar_gtk3.so

make:
    make gtk3
make-debug:
    make debug