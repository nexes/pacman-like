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
struct DeSerializedData
{
    // the unique user ID provided by the server
    int userID;
    // the response type
    int responseType;
    // the len of the data
    int len;

    // the game map data
    vector<string> map;
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
    static SerializedData SerializeNewPlayer();

    // serialize the new player response to send to the client
    static SerializedData SerializeNewPlayerResponse(int, vector<string> &);

    // de-serialize the new player response. Pass in the char array from the response
    static DeSerializedData DeSerializeNewPlayerResponse(const char[]);
};