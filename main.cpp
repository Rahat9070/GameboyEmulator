#include "src/Cartridge/cartridge.h"
#include "src/gameboy.h"

bool isValidROMFile(const std::string& filename) {
    return filename.size() > 3 && filename.substr(filename.size() - 3) == ".gb";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_rom_file>" << std::endl;
        return -1;
    }
    std::string directory = argv[1];
    if (!isValidROMFile(directory)) {
        std::cerr << "Error: The file must have a .gb extension" << std::endl;
        return -1;
    }
    Cartridge cartridge = Cartridge(directory);
    Gameboy gameboy(&cartridge);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        gameboy.step();
    }
    return 0;
}