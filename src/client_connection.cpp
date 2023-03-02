#include "../include/client_connection.h"
#include "../include/serializer.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <thread>

ClientConnection::ClientConnection() : listening(false), hasUpdate(false), err_msg("")
{
}

ClientConnection::~ClientConnection()
{
    close(this->thread_data.socket_fd);
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
    int sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (sock == -1) {
        err_msg = std::string("socket: ").append(gai_strerror(sock));
        freeaddrinfo(info);

        return false;
    }

    // there are no threads at this point in the setup, so not locking this
    this->thread_data.socket_fd = sock;

    // set a timeout for reading on the socket for 50ms
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000;  // 50 ms
    status = setsockopt(this->thread_data.socket_fd,
                        SOL_SOCKET,
                        SO_RCVTIMEO,
                        (const void *)&tv,
                        sizeof(tv));

    // connect to the server
    status = connect(this->thread_data.socket_fd, info->ai_addr, info->ai_addrlen);
    if (status == -1) {
        err_msg = std::string("connect: ").append(strerror(status));
        freeaddrinfo(info);
        close(this->thread_data.socket_fd);
        return false;
    }

    freeaddrinfo(info);

    // start a new thread to listen for server responses
    this->listening = true;
    std::thread t(&ClientConnection::thread_listenToServer, this);
    t.detach();  // TODO: proper thread handling

    return true;
}

// send a request to the server for a new player id and get the map data.
// if this send/recv failed map data can't load, needs to be resent
bool ClientConnection::requestNewPlayer(std::string playername)
{
    int wrote = 0;
    int len = 0;
    int sent = 0;

    // serialize the send request data;
    SerializedData player = Serializer::SerializeNewPlayerRequest(playername);

    // send the newplayer request to the server
    do {
        {
            std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
            wrote = write(this->thread_data.socket_fd, (void *)player.data, player.len);
        }  // unlock critical section here

        if (wrote == -1) {
            err_msg = std::string("NewPlayer request: ").append(strerror(wrote));
            return false;
        }
        sent += wrote;
    } while (sent != player.len);

    return true;
}

bool ClientConnection::requestUpdatePlayer(int score,
                                           Position pos,
                                           std::vector<Position> visited)
{
    int wrote = 0;
    int len = 0;
    int sent = 0;

    SerializedData update = Serializer::SerializePlayerUpdateRequest(score, pos, visited);

    // send the newplayer request to the server
    do {
        {
            std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
            wrote = write(this->thread_data.socket_fd, (void *)update.data, update.len);
        }  // unlock critical section here

        if (wrote == -1) {
            err_msg = std::string("UpdatePlayer request: ").append(strerror(wrote));
            return false;
        }
        sent += wrote;
    } while (sent != update.len);

    return true;
}

void ClientConnection::thread_listenToServer()
{
    while (listening) {
        int r = 0;
        char response[ServerInfo::player_read_size];

        // get the response if there is any
        {
            std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
            r = read(this->thread_data.socket_fd, (void *)response, sizeof(response));
        }

        // no response yet
        if (r < 1) {
            std::this_thread::sleep_for(std::chrono::microseconds(GameInfo::ThreadSleep));
            continue;
        }

        DeSerializedData playerData;
        int type = *(int *)response;

        switch (type) {
        case RequestType::NewPlayer: {
            playerData = Serializer::DeSerializeNewPlayerResponse(response);

            std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
            this->thread_data.client_id = playerData.userID;
            this->thread_data.client_map = playerData.map;
            this->thread_data.opponent = playerData.has_opponent;

            break;
        }
        case RequestType::NewOpponent: {
            playerData = Serializer::DeSerializeNewOpponentResponse(response);
            std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
            this->thread_data.opponent = true;
            this->isPlayer2 = playerData.isPlayer2;
            break;
        }
        case RequestType::UpdatePlayer: {
            this->hasUpdate = true;
            playerData = Serializer::DeSerializeUpdatePlayerResponse(response);
            oppData.score = playerData.score;
            oppData.pos = playerData.opponent_pos;
            oppData.visited = playerData.visited;
            break;
        }
        case RequestType::DisconnectPlayer:
            break;
        }

        // sleep
        std::this_thread::sleep_for(std::chrono::microseconds(GameInfo::ThreadSleep));
    }
}

std::string ClientConnection::getErrorMsg()
{
    return this->err_msg;
}

std::vector<std::string> ClientConnection::getGameMap()
{
    std::vector<std::string> map;
    {
        std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
        map = this->thread_data.client_map;
    }

    return map;
}

bool ClientConnection::hasOpponent()
{
    bool op = false;
    {
        std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
        op = this->thread_data.opponent;
    }

    return op;
}

bool ClientConnection::recievedOpponentUpdate()
{
    return hasUpdate;
}

OpponentData ClientConnection::getOpponentData()
{
    hasUpdate = false;
    return oppData;
}

bool ClientConnection::getPlayer2()
{
    return isPlayer2;
}