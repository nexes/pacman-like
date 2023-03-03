#pragma once

#include "../include/my_types.h"

#include <mutex>
#include <string>
#include <vector>

// data struct to hold opponents info on playerUpdate responses
struct OpponentData
{
    // opponent score
    int score;
    // opponent current position
    Position pos;
    // opponents visited positions
    std::vector<Position> visited;
};

class ClientConnection
{
public:
    ClientConnection();

    ~ClientConnection();

    // create a connection to the pacman server
    bool setupConnection();

    // send a request to the pacman server requesting a new player be setup.
    // this will return a unique player ID and the game map
    bool requestNewPlayer(std::string playername);

    // todo
    void requestDisconnectPlayer();

    // send a request to the pacman server to update the players info
    bool requestUpdatePlayer(int score, Position pos, std::vector<Position> visited);

    // checks if we've received an update on our opponent
    bool recievedOpponentUpdate();

    // get the updated data on our opponent
    OpponentData getOpponentData();

    // return the error message if one was set
    std::string getErrorMsg();

    // return the map retrieved from the server
    std::vector<std::string> getGameMap();

    // returns true if an opponent is found
    bool hasOpponent();

    // return true if this player is the second player
    bool isPlayer2();

private:
    struct CriticalSection
    {
        // mutex to lock when writing data
        std::mutex thread_mutex;

        // socket to the server I lock this because we write to it from the main thread
        // and read from it on a second thread
        int socket_fd;

        // data of our opponent. This gets updated constantly so I lock it when it's
        // changing
        OpponentData oppData;
    };

    // listen on a second thread for server responses
    void thread_listenToServer();

private:
    // unique client id from the server
    int client_id;
    // listening to the server
    bool listening;
    // if this player has an opponent
    bool opponent;
    // true if we've received an update from the opponent
    bool hasUpdate;
    // is this player the second player
    bool player2;
    // error message if there are any
    std::string err_msg;
    // client map
    std::vector<std::string> client_map;
    // shared data between threads
    CriticalSection thread_data;
};