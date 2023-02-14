#pragma once

#include "../include/ui.h"

class Pacman
{
public:
    Pacman();
    ~Pacman();

    void setupUI(int, int);
    void setupNetwork();

    void run();

private:
    int width;
    int height;
    UI ui;
};