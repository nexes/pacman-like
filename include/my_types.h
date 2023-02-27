#pragma once

// this file will hold all the variables of the program. This way if something needs
// to change it can be changed here
#include <string>

namespace RequestType {
    // request types from client to server
    const int NewPlayer = 0;
    const int DisconnectPlayer = 1;
    const int UpdatePlayer = 2;
    const int None = 3;

}  // namespace RequestType

namespace SerializeInfo {
    // 8k: max size of data to serialize
    const int MAX_SIZE = 8192;
}  // namespace SerializeInfo

namespace ServerInfo {
    // the number of connections allowed, passed to listen()
    const int back_log = 25;

    // the server port number, default 8080
    static std::string server_port = "8080";

    // the server hostname or IP
    // static std::string server_name = "csslab12.uwb.edu";
    static std::string server_name = "localhost";

    // 2k max data size to read from players buffer
    const int player_read_size = 2048;

}  // namespace ServerInfo

namespace GameInfo {
    // width of the game window
    const int WindowWidth = 150;
    // height of the game window
    const int WindowHeight = 80;
    // width of the canvas (where to map is drawn)
    const int CanvasWidth = 180;
    // height of the canvas (where to map is drawn)
    const int CanvasHeight = 100;
    // sleep the thread for 50 ms (~20fps)
    const int ThreadSleep = 50;
}  // namespace GameInfo
