#pragma once

#include "client_connection.h"
#include "ui.h"

class Pacman
{
public:
    Pacman();
    ~Pacman();

    void setupUI(int, int);

    // setup a connection to the pacman server
    // returns true if the connection was successful
    bool setupNetwork();

    void run();

private:
    int width;
    int height;

    UI ui;
    ClientConnection connection;
};