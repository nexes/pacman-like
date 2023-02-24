#include "../include/pacman.h"

int main(int argc, const char* argv[])
{
    Pacman pacman;

    // we need to establish a connection before anything else
    // if this fails an error message will be printed and we'll exit
    if (!pacman.setupNetwork())
        return 1;

    pacman.getUserName();
    pacman.run();

    return 0;
}
