#include "../include/serializer.h"

// serialize the new player request
// NewPlayer format {
//     Type: int
// }
SerializedData Serializer::SerializeNewPlayer()
{
    // there is nothing to tell the server except it's a new player
    SerializedData d;
    int *p = (int *)d.data;
    *p = RequestType::NewPlayer;

    d.len = sizeof(RequestType::NewPlayer);
    return d;
}

// serialize the new player response
// NewPlayer response format {
//      Type:  int
//      ID:    int (players unique id)
//      Len:   int (size of Data in bytes)
//      Data:  char[] (map data)
// }
SerializedData Serializer::SerializeNewPlayerResponse(int id, vector<string> &map)
{
    SerializedData d;
    int *int_ptr = (int *)d.data;

    // Type
    *int_ptr++ = RequestType::NewPlayer;

    // ID
    *int_ptr++ = id;

    // Len
    int map_len = 0;
    for (const string &line : map)
        map_len += line.length() + 1;
    *int_ptr++ = sizeof(char) * map_len;

    // Map Data
    char *char_ptr = (char *)int_ptr;

    for (const string &line : map) {
        for (int i = 0; i < line.size(); i++) {
            *char_ptr++ = line[i];
        }
        *char_ptr++ = '\n';
    }

    // Type:int + ID:int + Len:int + chars: char[]
    char *start = (char *)d.data;
    d.len = (sizeof(int) * 3) + char_ptr - start;

    return d;
}

// take a char array that was serialized by SerializeNewPlayerResponse() and deserialize
// it into a struct
DeSerializedData Serializer::DeSerializeNewPlayerResponse(const char data[])
{
    DeSerializedData d;
    int *ptr = (int *)data;

    d.responseType = *ptr++;
    d.userID = *ptr++;
    d.len = *ptr++;

    char *c_ptr = (char *)ptr;

    string line;
    for (int i = 0; i < d.len; i++) {
        if (*c_ptr == '\n') {
            d.map.push_back(line);
            line = "";
            c_ptr++;
        } else {
            line.push_back(*c_ptr++);
        }
    }

    return d;
}