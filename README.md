# Game Boy Emulator
This is a simpleGame Boy Emulator written in C++. This emulator supports the original Game Boy and was created to learn more about the creation of emulators. The emulator does not contain any extra unique features compared to other Game Boy emulators.

## Setup
To run this project, the libraries required to install are provided within `requirements.txt`. To install them on unix/WSL:
```
$ xargs sudo apt install -y < ./requirements.txt
```
or on MacOS:
```
xargs brew install < ./requirements.txt
```
If these commands return errors, you can install each library within `requirements.txt` seperately.

## Build
To build and create the application, run the commands below within the directory:
```
cmake .
make
```

## Usage
After making the build, to run the application run:
```
./GameboyEmulator <path_to_rom_file>
```