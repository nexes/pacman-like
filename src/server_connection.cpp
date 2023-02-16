#include "../include/server_connection.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>

ServerConnection::ServerConnection()
    : socket_fd(-1), back_log(25), port("8080"), err_msg(""), thread_data()
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
    status = getaddrinfo(NULL, this->port.c_str(), &hints, &info);
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
    status = listen(this->socket_fd, this->back_log);
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

        // accept a new client connection
        int client_fd =
            accept(this->socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        // if an error occured print out (or save to a log) the error, but don't stop
        // the server
        if (client_fd == -1) {
            std::cout << "accept(): " << gai_strerror(client_fd) << "\n";
            continue;
        }

        // lock our critical section with std::lock_guard.
        // lock_guard will unlock the mutex when it falls out of it's scope
        {
            std::lock_guard<std::mutex> lock(thread_data.thread_mutex);
            thread_data.accepted_fd = client_fd;
        }

        // start a new thread
        std::thread t(&ServerConnection::thread_handleConnection, this);
    }
}

// This functions runs in the spawned thread
// If we receive a new player, we check if there is another player who has not
// been paired up. If there is, we pair that player with this new player and the
// thread continues to run.
// If there is no player waiting to be paired, this new player will be inserted
// into the client map and the thread exits.
void ServerConnection::thread_handleConnection()
{
    int new_client;
    std::unordered_map<int, int>::iterator players;

    // lock our critical sections with a lock_guard that unlocks the mutex
    // when it falls out of scope
    {
        std::lock_guard<std::mutex> lock(thread_data.thread_mutex);
        new_client = thread_data.accepted_fd;
        players = thread_data.client_pairs.find(new_client);
    }

    // read from the new client
    // TODO:
}

std::string ServerConnection::getErrorMsg()
{
    return this->err_msg;
}

std::string ServerConnection::getPort()
{
    return this->port;
}