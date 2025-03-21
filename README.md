# Game Boy Emulator
Gameboy Emulator written in c++

## Setup
To run this project, the libraries required are provided to install within `requirements.txt`. To install them on unix/WSL
```
$ xargs sudo apt install -y < ./requirements.txt
```
or on MacOS
```
xargs brew install < ./requirements.txt
```
If these commands return errors, you can install each library within `requirements.txt` seperately.

## Build
To build and create the application, run the commands below within the directory
```
cmake .
make
```

## Usage
To run the application
```
./GameboyEmulator <path_to_rom_file>
```