#pragma once

#include <mutex>
#include <string>
#include <vector>

class ClientConnection
{
public:
    ClientConnection();

    ~ClientConnection();

    // create a connection to the pacman server
    bool setupConnection();

    // send a request to the pacman server requesting a new player be setup.
    // this will return a unique player ID and the game map
    bool requestNewPlayer(std::string);

    // todo
    void requestDisconnectPlayer();

    // todo
    void requestUpdatePlayer();

    // return the error message if one was set
    std::string getErrorMsg();

    // return the map retrieved from the server
    std::vector<std::string> getGameMap();

    bool hasOpponent();

private:
    struct CriticalSection
    {
        // mutex to lock when writing data
        std::mutex thread_mutex;
        // socket to the server we listen on a second thread
        int socket_fd;
        // if this player has an opponent
        bool opponent;
        // unique client id from the server
        int client_id;
        // client map
        std::vector<std::string> client_map;
    };

    // listen on a second thread for server responses
    void thread_listenToServer();

private:
    // listening to the server
    bool listening;
    // error message if there are any
    std::string err_msg;
    // shared data between threads
    CriticalSection thread_data;
};