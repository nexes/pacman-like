#pragma once

#include <string>

#include "client_connection.h"
#include "ui.h"

class Pacman
{
public:
    Pacman();
    ~Pacman();

    // display the username UI and return the username
    void getUserName();

    // setup a connection to the pacman server
    // returns true if the connection was successful
    bool setupNetwork();

    // starts the game
    void run();

private:
    // this players username
    std::string username;
    // this controls our main loop
    bool playing;
    // the UI class FTXUI
    UI ui;
    // handle all connections to/from the server
    ClientConnection connection;
};