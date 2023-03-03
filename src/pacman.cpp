#include "../include/pacman.h"
#include "../include/my_types.h"

#include <ftxui/component/loop.hpp>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

Pacman::Pacman() : username("Player"), opponent_name(""), playing(true), player_score(0)
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
    bool show_map = false;
    std::vector<Position> visited;

    std::cout << "Waiting for player 2...\n";

    // wait until player 2 connects to show the map
    while (!this->connection.hasOpponent())
        std::this_thread::sleep_for(std::chrono::milliseconds(GameInfo::ThreadSleep));

    // get the map from the server
    std::vector<std::string> map = this->connection.getGameMap();

    // give the map to the UI and set if this player is the second player
    this->ui.setGameMap(map, this->connection.isPlayer2());

    // display the game map and return the game loop object (FTXUI component)
    ftxui::Loop loop = this->ui.getGameLoop();

    // the main game loop
    while (playing) {
        // get our opponents data
        if (this->connection.recievedOpponentUpdate()) {
            OpponentData op = this->connection.getOpponentData();

            this->opponent_score = op.score;
            this->ui.setOpponentScore(op.score);
            this->ui.setOpponentPosition(op.pos.first, op.pos.second);
            this->ui.updateMap(op.visited);
        }

        // send our players updated data
        int score = this->ui.getScore();
        Position current = this->ui.getPosition();
        visited = this->ui.getMovements();

        // TODO: remove the visited cells we've already updated. send less data
        if (!this->connection.requestUpdatePlayer(score, current, visited))
            std::cerr << "error sending update\n";

        loop.RunOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(GameInfo::ThreadSleep));
    }

    std::cout << "Thank you for playing!\n";
}