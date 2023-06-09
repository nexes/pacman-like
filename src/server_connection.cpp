#include "../include/serializer.h"
#include "../include/server_connection.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <thread>

ServerConnection::ServerConnection() : socket_fd(-1), err_msg(""), thread_data()
{
}

ServerConnection::~ServerConnection()
{
    close(this->socket_fd);
}

// this gets called from the Server binary. A map is read from a txt file and passed into
// here. This will be servered on newPlayerRequests
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

// this will wait for incoming connections. pair the player and spawn a thread
void ServerConnection::waitForPlayer()
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
    std::string player_name;
    char p1_data[ServerInfo::player_read_size];

    while (playing) {
        // read incoming data
        int p1_read = read(player_socket, (void *)p1_data, sizeof(p1_data));
        if (p1_read == -1) {
            std::cerr << "Error reading player " << player_socket << strerror(p1_read)
                      << "\n";
        }

        // if nothing was read, sleep and continue
        if (p1_read < 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(GameInfo::ThreadSleep));
            continue;
        }

        // parse and handle player requests
        if (p1_read > 0) {
            int opponent_fd;
            int *p = (int *)p1_data;
            int type = *p;  // first 4 bytes is always the request type

            switch (type) {
            case RequestType::NewPlayer: {
                DeSerializedData p = DeSerialize::NewPlayerRequest(p1_data);

                {  // lock and check if we have an opponent
                    std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);

                    this->thread_data.player_names[player_socket] = p.playername;
                    opponent_fd = this->thread_data.client_pairs[player_socket];
                }

                // send the new player response
                handle_newPlayerRequest(player_socket, opponent_fd);

                // if this player has an opponent (meaning this is player 2) send the
                // newOpponentRequest with the other players name
                if (opponent_fd != -1) {
                    std::string player1_name;
                    std::string player2_name;

                    {
                        std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
                        player1_name = this->thread_data.player_names[opponent_fd];
                        player2_name = this->thread_data.player_names[player_socket];
                    }
                    handle_newOpponentRequest(player_socket, true, player1_name);
                    handle_newOpponentRequest(opponent_fd, false, player2_name);
                }
                break;
            }
            case RequestType::DisconnectPlayer: {
                DeSerializedData p = DeSerialize::PlayerDisconnectRequest(p1_data);
                std::string player1_name;
                std::string player2_name;

                // lock the critical section to remove players socket from client_pairs
                {
                    std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
                    opponent_fd = this->thread_data.client_pairs[p.userID];

                    if (opponent_fd != -1) {
                        this->thread_data.client_pairs[opponent_fd] = -1;
                        player1_name = this->thread_data.player_names[p.userID];
                        player2_name = this->thread_data.player_names[opponent_fd];
                    }

                    // remove this player and their opponent from the map
                    this->thread_data.client_pairs.erase(p.userID);
                }

                // if this is the disconnecting player, update the leader board and tell
                // the opponent of the disconnect
                if (opponent_fd != -1) {
                    {
                        std::lock_guard<std::mutex> lock(this->leader_board.thread_mutex);
                        this->leader_board.board.push_back(
                            {player1_name, p.score, player2_name, p.op_score});
                    }

                    handle_disconnectPlayerRequest(opponent_fd, p.op_score, p.score);
                    handle_disconnectPlayerRequest(p.userID, p.score, p.op_score);
                }

                playing = false;
                break;
            }
            case RequestType::UpdatePlayer:
                DeSerializedData p = DeSerialize::PlayerUpdateResponse(p1_data);

                {  // lock and get our opponents socket fd
                    std::lock_guard<std::mutex> lock(this->thread_data.thread_mutex);
                    opponent_fd = this->thread_data.client_pairs[player_socket];
                }

                if (opponent_fd != -1) {
                    handle_updatePlayerRequest(opponent_fd,
                                               p.score,
                                               p.opponent_pos,
                                               p.visited);
                }
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
void ServerConnection::handle_newPlayerRequest(int socket, int has_opponent)
{
    int sent = 0;
    int op = has_opponent == -1 ? 0 : 1;
    SerializedData d = Serialize::NewPlayerResponse(socket, op, this->map);

    do {
        int s = write(socket, (void *)d.data, d.len);
        if (s == -1) {
            // TODO: error handle
            std::cerr << "Error New Player Request sending " << strerror(s) << "\n";
            break;
        }

        sent += s;
    } while (sent != d.len);
}

// tell the other player that an opponent has been found and give them that players name
void ServerConnection::handle_newOpponentRequest(int player_socket,
                                                 bool isPlayer2,
                                                 std::string name)
{
    int sent = 0;
    int p2 = isPlayer2 ? 1 : 0;
    SerializedData d = Serialize::NewOpponentResponse(player_socket, p2, name);

    do {
        int s = write(player_socket, (void *)d.data, d.len);
        if (s == -1) {
            // TODO: error handle
            std::cerr << "Error New Opponent Request sending " << strerror(s) << "\n";
            break;
        }

        sent += s;
    } while (sent != d.len);
}

void ServerConnection::handle_updatePlayerRequest(int socket,
                                                  int score,
                                                  Position pos,
                                                  std::vector<Position> visited)
{
    int sent = 0;
    SerializedData d = Serialize::PlayerUpdateRequest(score, pos, visited);

    do {
        int s = write(socket, (void *)d.data, d.len);
        if (s == -1) {
            // TODO: error handle
            std::cerr << "Error Sending Update Player Request " << strerror(s) << "\n";
            break;
        }

        sent += s;
    } while (sent != d.len);
}

void ServerConnection::handle_disconnectPlayerRequest(int socket, int score, int op_score)
{
    int sent = 0;
    SerializedData d;
    // SerializedData d = Serialize::PlayerDisconnectRequest(socket, score, op_score);
    {
        std::lock_guard<std::mutex> lock(this->leader_board.thread_mutex);
        d = Serialize::PlayerDisconnectResponse(socket, this->leader_board.board);
    }

    do {
        int s = write(socket, (void *)d.data, d.len);
        if (s == -1) {
            // TODO: error handle
            std::cerr << "Error Sending Disconnect Player Request " << strerror(s)
                      << "\n";
            break;
        }

        sent += s;
    } while (sent != d.len);
}

std::string ServerConnection::getErrorMsg()
{
    return this->err_msg;
}
