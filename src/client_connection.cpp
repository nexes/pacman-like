#include "../include/client_connection.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "../include/serializer.h"

ClientConnection::ClientConnection()
    : socket_fd(-1), err_msg(""), client_id(-1), client_map()
{
}

ClientConnection::~ClientConnection()
{
    close(socket_fd);
}

bool ClientConnection::setupConnection()
{
    int status;
    struct addrinfo *info;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // setup client info
    status = getaddrinfo(ServerInfo::server_name.c_str(),
                         ServerInfo::server_port.c_str(),
                         &hints,
                         &info);
    if (status != 0) {
        err_msg = std::string("getaddrinfo: ").append(gai_strerror(status));
        return false;
    }

    // setup the client socket fd
    socket_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (socket_fd == -1) {
        err_msg = std::string("socket: ").append(gai_strerror(socket_fd));
        freeaddrinfo(info);

        return false;
    }

    // connect to the server
    status = connect(socket_fd, info->ai_addr, info->ai_addrlen);
    if (status == -1) {
        err_msg = std::string("connect: ").append(strerror(status));
        freeaddrinfo(info);
        close(socket_fd);
        return false;
    }

    freeaddrinfo(info);
    return true;
}

// send a request to the server for a new player id and get the map data.
// if this send/recv failed map data can't load, needs to be resent
bool ClientConnection::requestNewPlayer()
{
    int len = 0;
    int sent = 0;
    bool success = true;

    // serialize the send request data;
    SerializedData player_data = Serializer::SerializeNewPlayer();

    // send the newplayer request to the server
    do {
        int s = write(this->socket_fd, (void *)player_data.data, player_data.len);
        if (s == -1) {
            err_msg = std::string("NewPlayer request: ").append(strerror(s));
            success = false;
            break;
        }
        sent += s;
    } while (sent != player_data.len);

    // get the response
    char response[ServerInfo::player_read_size];
    int r = read(this->socket_fd, (void *)response, sizeof(response));
    if (r == -1) {
        err_msg = std::string("NewPlayer request: ").append(strerror(r));
        success = false;
    }

    // deserialize the byte stream to a usable data struct
    DeSerializedData data = Serializer::DeSerializeNewPlayerResponse(response);
    this->client_id = data.userID;
    this->client_map = data.map;

    // TODO: handle bad response data

    return success;
}

std::string ClientConnection::getErrorMsg()
{
    return this->err_msg;
}

std::vector<std::string> ClientConnection::getGameMap() const
{
    return this->client_map;
}