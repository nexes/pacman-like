#pragma once

// this file will hold all the 'magic' numbers of the program. This way if something needs
// to change I can change it in on place
#include <string>

namespace RequestType {
    // New player request
    const int NewPlayer = 0;

    // Disconnect player request
    const int DisconnectPlayer = 1;

    // Payer update request
    const int UpdatePlayer = 2;
}  // namespace RequestType

namespace SerializeInfo {
    // 8k: make size of data to serialize
    const int MAX_SIZE = 8192;
}  // namespace SerializeInfo

namespace ServerInfo {
    // the number of connections allowed, passed to listen()
    const int back_log = 25;

    // the server port number, default 8080
    static std::string server_port = "8080";

    // the server hostname or IP
    // const char *server_name = "csslab12.uwb.edu";
    static std::string server_name = "localhost";

    // 2k max data size to read from players buffer
    const int player_read_size = 2048;

}  // namespace ServerInfo