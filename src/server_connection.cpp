#include "../include/server_connection.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "../include/my_types.h"
#include "../include/serializer.h"

ServerConnection::ServerConnection() : socket_fd(-1), err_msg(""), thread_data()
{
}

ServerConnection::~ServerConnection()
{
    // todo:
    // this->client_pairs.clear();

    close(this->socket_fd);
}

void ServerConnection::setGameMap(std::vector<std::string> map)
{
    // TODO:
    this->map = map;
}

// want to return a struct with a union for the error instead?
bool ServerConnection::setupConnection()
{
    int status;
    struct addrinfo hints;
    struct addrinfo *info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // setup server info
    status = getaddrinfo(NULL, ServerInfo::server_port.c_str(), &hints, &info);
    if (status != 0) {
        this->err_msg = std::string("getaddrinfo(): ").append(gai_strerror(status));
        return false;
    }

    // create a new socket
    this->socket_fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    if (this->socket_fd == -1) {
        this->err_msg = std::string("socket(): ").append(strerror(this->socket_fd));
        freeaddrinfo(info);
        return false;
    }

    // allow socket descriptor to be reusable
    int yes = 1;
    setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    // bind our socket to our port
    status = bind(this->socket_fd, info->ai_addr, info->ai_addrlen);
    if (status == -1) {
        this->err_msg = std::string("bind() ").append(gai_strerror(status));

        close(this->socket_fd);
        freeaddrinfo(info);
        return false;
    }

    // listen on our socket
    status = listen(this->socket_fd, ServerInfo::back_log);
    if (status == -1) {
        this->err_msg = std::string("listen(): ").append(strerror(status));

        close(this->socket_fd);
        freeaddrinfo(info);
        return false;
    }

    freeaddrinfo(info);
    return true;
}

void ServerConnection::waitForClient()
{
    bool accepting = true;

    while (accepting) {
        struct sockaddr_storage client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        bool paired = false;

        // accept a new client connection
        int client_fd =
            accept(this->socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        // if an error occured print out (or save to a log) the error, but don't stop
        // the server
        if (client_fd == -1) {
            std::cerr << "accept(): " << gai_strerror(client_fd) << "\n";
            continue;
        }

        // lock our critical section with std::lock_guard. unlock once dropped from scope
        {
            std::lock_guard<std::mutex> lock(thread_data.thread_mutex);

            // if this is the first player on the server, than just insert them with no
            // opponent (-1)
            if (thread_data.client_pairs.size() == 0) {
                thread_data.client_pairs.insert({client_fd, -1});
            } else {
                // check if we can pair this new player with someone who is waiting.
                // we will pair like this: A->B and B->A.
                // so each thread can quickly get it's opponent
                for (const auto &[key, value] : thread_data.client_pairs) {
                    if (value == -1) {
                        thread_data.client_pairs[key] = client_fd;
                        thread_data.client_pairs[client_fd] = key;
                        paired = true;
                        break;
                    }
                }

                // no waiting player found, insert as a new player waiting
                if (!paired)
                    thread_data.client_pairs.insert({client_fd, -1});
            }
        }  // critical section unlocks here

        // start a new thread to handle this player
        std::thread t(&ServerConnection::thread_handleConnection, this, client_fd);
        t.detach();  // TODO: better thread managment
    }
}

// This function runs in the spawned thread
// This function is called when the server's accept()'d a new client
void ServerConnection::thread_handleConnection(int player_socket)
{
    bool playing = true;
    int opponent_fd = -1;
    char p1_data[ServerInfo::player_read_size];

    while (playing) {
        {
            std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
            opponent_fd = this->thread_data.client_pairs[player_socket];
        }  // critical section unlocks here

        int p1_read = read(player_socket, (void *)p1_data, sizeof(p1_data));
        if (p1_read == -1) {
            std::cerr << "Error reading player 1 " << strerror(p1_read) << "\n";
        }

        // if nothing was read, sleep and continue
        if (p1_read < 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(GameInfo::ThreadSleep));
            continue;
        }

        // parse and handle player 1 request
        if (p1_read > 0) {
            int *p = (int *)p1_data;
            int type = *p;  // first 4 bytes is always the type

            switch (type) {
            case RequestType::NewPlayer:
                thread_newPlayerRequest(player_socket);

                if (opponent_fd != -1) {
                    std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
                    thread_newOpponentRequest(player_socket);
                    thread_newOpponentRequest(opponent_fd);
                }
                break;
            case RequestType::DisconnectPlayer:
                break;
            case RequestType::UpdatePlayer:
                break;
            };
        }

        // sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(GameInfo::ThreadSleep));
    }
}

// handle the new player request. Send a unique ID for the user and the game map.
// if this is player one, hasOpponent will be false, player two this will be true
// this function is called from the spawned thread
void ServerConnection::thread_newPlayerRequest(int socket)
{
    SerializedData d = Serializer::SerializeNewPlayerResponse(socket, this->map);

    int len = d.len;
    int sent = 0;

    do {
        int s = write(socket, (void *)d.data, len);
        if (s == -1) {
            // TODO: error handle
            std::cerr << "Error sending " << strerror(s) << "\n";
            break;
        }

        sent += s;
    } while (sent != len);
}

// tell the other player that an opponent has been found
void ServerConnection::thread_newOpponentRequest(int player_socket)
{
    SerializedData d = Serializer::SerializeNewOpponentResponse(player_socket);

    int len = d.len;
    int sent = 0;

    do {
        int s = write(player_socket, (void *)d.data, len);
        if (s == -1) {
            // TODO: error handle
            std::cerr << "Error sending " << strerror(s) << "\n";
            break;
        }

        sent += s;
    } while (sent != len);
}

std::string ServerConnection::getErrorMsg()
{
    return this->err_msg;
}
