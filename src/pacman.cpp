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
}

void Pacman::setupNetwork()
{
}

void Pacman::run()
{
    std::cout << "Hello Pacman\n";
}