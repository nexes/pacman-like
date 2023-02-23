#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// TODO
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
    void waitForClient();

    // sets the game map that will be sent to the clients
    void setGameMap(std::vector<std::string>);

    // return the error message. If a function returns false, the error
    // message will be set and can be retrieved here
    std::string getErrorMsg();

    // returns the port number the server is listening on
    std::string getPort();

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
    };

    // handle a new connection ( a new player )
    // void handleConnection(CriticalSection &);
    void thread_handleConnection();

private:
    // the server file descriptor
    int socket_fd;
    // the number of connections allowed, passed to listen()
    int back_log;
    // the server port number, default 8080
    std::string port;
    // the value of the error message if any
    std::string err_msg;
    // the map the players will play on
    std::vector<std::string> map;

    // data structure that each thread will have access to
    CriticalSection thread_data;
};