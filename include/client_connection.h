#pragma once

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
    bool requestNewPlayer();

    // todo
    void requestDisconnectPlayer();

    // todo
    void requestUpdatePlayer();

    // return the error message if one was set
    std::string getErrorMsg();

private:
    // socket to the server
    int socket_fd;
    // unique client id from the server
    int client_id;
    // client map
    std::vector<std::string> client_map;
    // error message if there are any
    std::string err_msg;
};