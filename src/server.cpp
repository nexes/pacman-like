#include <fstream>
#include <iostream>
#include <vector>

#include "../include/my_types.h"
#include "../include/server_connection.h"

// read the game map from the text file and return it as a list
// of strings
std::vector<std::string> readMapData(std::string map_file)
{
    std::vector<std::string> map;

    std::ifstream file(map_file, std::ios::in);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            map.push_back(line);
        }
        file.close();
    } else {
        std::cerr << "Couldn't open map file: " << map_file << "\n";
    }

    return map;
}

int main(int argc, char *argv[])
{
    ServerConnection connection;
    connection.setGameMap(readMapData("./Map1.txt"));

    if (!connection.setupConnection()) {
        std::cerr << connection.getErrorMsg();
        return -1;
    }

    std::cerr << "Server listening on port " << ServerInfo::server_port << "\n";
    connection.waitForClient();

    return 0;
}