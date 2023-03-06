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
// data members aren't being used by that response. Refactor this???
struct DeSerializedData
{
    // the response type
    int responseType;
    // the unique user ID provided by the server
    int userID;
    // the player score
    int score;
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

// this is a simple and not too clever serializer. This is not a general
// purpose serializer, it will just work for this project. It doens't need
// to know anything about the project, so it will just be a static, functional
// serializer with no side effects.
class Serializer
{
public:
    Serializer() = delete;
    Serializer(const Serializer &) = delete;

    // serialize a new player request to send to the server
    static SerializedData SerializeNewPlayerRequest(string playername);

    // serialize a player update
    static SerializedData SerializePlayerUpdateRequest(int score,
                                                       Position pos,
                                                       vector<Position> visited);

    // serialize a new opponent request to send to the client
    static SerializedData SerializeNewOpponentResponse(int socket,
                                                       int isPlayer2,
                                                       string name);

    // serialize the new player response to send to the client
    static SerializedData SerializeNewPlayerResponse(int id,
                                                     int has_opponent,
                                                     vector<string> &map);

    // de-serialize the new player response.
    static DeSerializedData DeSerializeNewPlayerResponse(const char data[]);

    // de-serialize the new player request
    static DeSerializedData DeSerializeNewPlayerRequest(const char data[]);

    // de-serialize the update player response
    static DeSerializedData DeSerializeUpdatePlayerResponse(const char data[]);

    // de-serialize the new opponent response
    static DeSerializedData DeSerializeNewOpponentResponse(const char data[]);
};