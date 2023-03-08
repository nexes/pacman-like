#pragma once

#include "../include/my_types.h"

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class ServerConnection
{
public:
    // default constructor
    ServerConnection();

    // default destructor
    ~ServerConnection();

    // create a socket ready to listen on port 8080. The socket will
    // not start accepting from the function. Returns true if the
    // server socket was created successfully
    bool setupConnection();

    // listen on port 8080 to handle incoming clients
    void waitForPlayer();

    // sets the game map that will be sent to the clients
    void setGameMap(std::vector<std::string>);

    // return the error message. If a function returns false, the error
    // message will be set and can be retrieved here
    std::string getErrorMsg();

private:
    struct CriticalSection
    {
        // mutex to lock when writing data
        std::mutex thread_mutex;

        // this is the accepted fd from accept().
        // this value if new, will be inserted into client_pairs.
        // this value gets overwritten on new accept() calls
        int accepted_fd;

        // an unordered map of players paired together
        // key: the players socket descriptor
        // value: the players opponents socket descriptor.
        std::unordered_map<int, int> client_pairs;

        // map the player name to their socket fd. This is so we can send the name to the
        // correct opponent
        std::unordered_map<int, std::string> player_names;
    };

    // handle a new connection
    void thread_handleConnection(int socket);

    // handle the new player request
    void handle_newPlayerRequest(int socket, int has_opponent);

    // handle and new opponent found for player one
    void handle_newOpponentRequest(int socket, bool isPlayer2, std::string name);

    // handle the player disconnect request
    void handle_disconnectPlayerRequest(int socket, int score, int op_score);

    // handle a player update request
    void handle_updatePlayerRequest(int socket,
                                    int score,
                                    Position pos,
                                    std::vector<Position> visited);

private:
    // the server file descriptor
    int socket_fd;
    // the value of the error message if any
    std::string err_msg;
    // the map the players will play on
    std::vector<std::string> map;
    // data structure that each thread will have access to
    CriticalSection thread_data;
};
