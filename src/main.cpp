#include "../include/pacman.h"

int main(int argc, const char* argv[])
{
    Pacman pacman;

    pacman.setupNetwork();
    pacman.setupUI(150, 80);

    pacman.run();

    return 0;
}
