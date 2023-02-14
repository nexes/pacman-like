#include "../include/pacman.h"

int main(int argc, const char* argv[])
{
    Pacman pacman;

    pacman.setupUI(150, 80);
    pacman.setupNetwork();

    pacman.run();

    return 0;
}
