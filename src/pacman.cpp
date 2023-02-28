#include "../include/pacman.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "../include/my_types.h"
#include "../include/serializer.h"

Pacman::Pacman() : username("Player"), playing(true)
{
}

Pacman::~Pacman()
{
}

void Pacman::getUserName()
{
    this->username = ui.displayGetUserName();
}

bool Pacman::setupNetwork()
{
    // setup connection to the server
    if (!connection.setupConnection()) {
        std::cerr << "Failed to connect to the Pac-Man server: "
                  << this->connection.getErrorMsg() << "\n";
        return false;
    }

    // request a new player from the server
    if (!connection.requestNewPlayer(this->username)) {
        std::cerr << "Failed new player request: " << this->connection.getErrorMsg()
                  << "\n";
        return false;
    }

    return true;
}

void Pacman::run()
{
    std::cout << "Waiting for player 2...\n";
    bool show_map = false;

    // TODO: delete this, just for testing
    std::vector<std::string> t;
    bool tt = false;
    while (!show_map) {
        tt = this->connection.hasOpponent();
        if (tt) {
            show_map = true;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    t = this->connection.getGameMap();
    this->ui.displayGameMap(t);

    std::cout << "Thank you for playing!\n";
}