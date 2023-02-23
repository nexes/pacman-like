#include "../include/pacman.h"

#include <iostream>

Pacman::Pacman() : width(100), height(100)
{
}

Pacman::~Pacman()
{
}

void Pacman::setupUI(int width, int height)
{
    this->width = width;
    this->height = height;

    ui.initalizeUI(width, height);
    ui.displayGetUserName();
    // ui.displayGameMap(strings);
}

bool Pacman::setupNetwork()
{
    if (!connection.setupConnection()) {
        std::cout << "Failed to connect to the Pac-Man server: "
                  << this->connection.getErrorMsg() << "\n";
        return false;
    }

    // request a new player from the server
    if (!connection.requestNewPlayer()) {
        std::cout << "Failed new player request: " << this->connection.getErrorMsg()
                  << "\n";
        return false;
    }

    return true;
}

void Pacman::run()
{
    std::cout << "Hello Pacman\n";
}