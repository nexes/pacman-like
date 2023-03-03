#pragma once

#include "client_connection.h"
#include "ui.h"

#include <string>

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
    // player 2's username
    std::string opponent_name;

    // this controls our main loop
    bool playing;
    // our players score
    int player_score;
    // opponents score
    int opponent_score;

    // the UI class
    UI ui;
    // handle all connections to/from the server
    ClientConnection connection;
};