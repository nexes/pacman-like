#pragma once

#include <string>
#include <vector>

#include "../include/my_types.h"

using std::string;
using std::vector;

// the returned data struct for each serialization function
struct SerializedData
{
    // the number of bytes being sent. How much data was read into data[]
    int len;
    // the byte array being sent
    char data[SerializeInfo::MAX_SIZE];
};

// the returned data struct for each de-serialization function
// TODO: this is a data structure that is used for all response types, even if some of the
// data members aren't being used by that response.
//
// TODO: Refactor this
struct DeSerializedData
{
    // the response type
    int responseType;
    // the unique user ID provided by the server
    int userID;
    // the player score
    int score;
    // the opponents score
    int op_score;
    // the opponents position
    Position opponent_pos;
    // the playername
    string playername;
    // if this player has an opponent to player
    bool has_opponent;
    // if this player is the second player
    bool isPlayer2;
    // the len of the data
    int len;
    // for the new player resonse this will be the game map
    vector<string> map;
    // list of opponents visited positions
    vector<Position> visited;
};

namespace Serialize {
    // serialize a new player request to send to the server
    SerializedData NewPlayerRequest(string playername);

    // serialize a player update
    SerializedData PlayerUpdateRequest(int score, Position pos, vector<Position> visited);

    // serialize a new opponent request to send to the client
    SerializedData NewOpponentResponse(int socket, int isPlayer2, string name);

    // serialize the new player response to send to the client
    SerializedData NewPlayerResponse(int id, int has_opponent, vector<string> &map);

    // serialize the disconnect request
    SerializedData PlayerDisconnectRequest(int id, int p1_score, int p2_score);

}  // namespace Serialize

namespace DeSerialize {
    // de-serialize the new player response.
    DeSerializedData NewPlayerResponse(const char data[]);

    // de-serialize the new player request
    DeSerializedData NewPlayerRequest(const char data[]);

    // de-serialize the update player response
    DeSerializedData PlayerUpdateResponse(const char data[]);

    // de-serialize the new opponent response
    DeSerializedData NewOpponentResponse(const char data[]);

    // de-serialize the player disconnect response
    DeSerializedData PlayerDisconnectResponse(const char data[]);

}  // namespace DeSerialize
