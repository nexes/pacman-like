#include "../include/pacman.h"
#include "../include/my_types.h"

#include <ftxui/component/loop.hpp>

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

Pacman::Pacman() : username("Player"), opponent_name(""), playing(false), player_score(0)
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

void Pacman::setupGame()
{
    std::cout << "Waiting for an opponent...\n";

    // wait until player 2 connects to show the map
    while (!this->connection.hasOpponent())
        std::this_thread::sleep_for(std::chrono::milliseconds(GameInfo::ThreadSleep));

    // get the map and player2 name from the server and give it to the UI
    bool isPlayer2 = this->connection.isPlayer2();
    opponent_name = this->connection.getOpponentName();
    std::vector<std::string> map = this->connection.getGameMap();

    this->ui.setGameMap(map, isPlayer2, opponent_name);
    playing = true;
}

void Pacman::run()
{
    int score = 0;
    std::vector<Position> visited;
    Position last_pos = std::make_pair(0, 0);

    // display the game map and return the game loop object (FTXUI component)
    ftxui::Loop loop = this->ui.getGameLoop();

    while (playing) {
        // if the player clicked the 'quit' button or their opponent has quit
        if (this->ui.hasQuit() || this->connection.sentDisconnect()) {
            playing = false;
            break;
        }

        // get our opponents data
        if (this->connection.recievedOpponentUpdate()) {
            OpponentData op = this->connection.getOpponentData();

            this->opponent_score = op.score;
            this->ui.setOpponentScore(op.score);
            this->ui.setOpponentPosition(op.pos.first, op.pos.second);
            this->ui.updateMap(op.visited);
        }

        // send our players updated data
        score = this->ui.getScore();
        visited = this->ui.getMovements();
        Position current = this->ui.getPosition();

        // only need to send an update if the player has made a move
        if (last_pos.first != current.first || last_pos.second != current.second) {
            last_pos = current;

            if (!this->connection.requestUpdatePlayer(score, current, visited))
                std::cerr << "error sending update " << this->connection.getErrorMsg()
                          << "\n";
        }

        this->ui.tick();
        loop.RunOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(GameInfo::ThreadSleep));
    }

    if (!this->connection.sentDisconnect()) {
        this->connection.requestDisconnectPlayer(score, opponent_score);
    } else {
        // display to the other player that this player quit
        this->ui.displayDisconnect(opponent_name);
    }
}

void Pacman::showLeaderBoard()
{
    this->ui.displayEndGameScore(this->connection.getLeaderBoard());
}