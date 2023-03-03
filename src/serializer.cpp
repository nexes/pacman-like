#include "../include/serializer.h"

#include <iostream>

// serialize the new player request
// NewPlayer format {
//     Type: int
//     Name: string (player name)
// }
SerializedData Serializer::SerializeNewPlayerRequest(string playername)
{
    SerializedData d;
    int *ptr = (int *)d.data;

    // type
    *ptr++ = RequestType::NewPlayer;

    // player name
    char *c_ptr = (char *)ptr;
    for (const char c : playername)
        *c_ptr++ = c;

    // delimiter
    *c_ptr++ = '%';

    char *start = (char *)d.data;
    d.len = sizeof(int) + c_ptr - start;
    return d;
}

// serialize the player update request
// UpdatePlayer format {
//      Type:    int      (type)
//      Score:   int      (payer score)
//      Pos row: int      (player row position)
//      Pos col  int      (player col position)
//      Len:     int      (number of Visited)
//      Visited: int[x,y] (player visited positons)
// }
SerializedData Serializer::SerializePlayerUpdateRequest(int score,
                                                        Position pos,
                                                        vector<Position> visited)
{
    SerializedData d;
    int *ptr = (int *)d.data;

    // type
    *ptr++ = RequestType::UpdatePlayer;

    // score
    *ptr++ = score;

    // position row
    *ptr++ = pos.first;

    // position col
    *ptr++ = pos.second;

    // visited nodes
    int len = visited.size();
    *ptr++ = len;

    for (const Position &p : visited) {
        *ptr++ = p.first;   // visited x
        *ptr++ = p.second;  // visited y
    }

    d.len = sizeof(int) * 5 + (len * 2 * sizeof(int));

    return d;
}

// serialzie the new opponet response
// NewOpponent response format {
//      Type:    int
//      ID:      int (players unique id)
//      Player2: int (if this is player 2)
//      len:     int (length of the player name)
//      Name:    string (the other players name)
// }
SerializedData Serializer::SerializeNewOpponentResponse(int socket,
                                                        int isPlayer2,
                                                        string name)
{
    SerializedData d;
    int *ptr = (int *)d.data;

    *ptr++ = RequestType::NewOpponent;
    *ptr++ = socket;
    *ptr++ = isPlayer2;

    int len = name.length();
    *ptr++ = len;

    char *c_ptr = (char *)ptr;

    for (int i = 0; i < len; i++)
        *c_ptr++ = name[i];

    d.len = sizeof(int) * 4 + (sizeof(char) * len);
    return d;
}

// serialize the new player response
// NewPlayer response format {
//      Type:  int
//      ID:    int (players unique id)
//      OP:    int (has an opponent)
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
    // OP
    *int_ptr++ = 0;

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
    d.len = (sizeof(int) * 4) + char_ptr - start;

    return d;
}

DeSerializedData Serializer::DeSerializeNewPlayerResponse(const char data[])
{
    DeSerializedData d;
    int *ptr = (int *)data;

    d.responseType = *ptr++;
    d.userID = *ptr++;
    d.has_opponent = *ptr++;
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

DeSerializedData Serializer::DeSerializeNewPlayerRequest(const char data[])
{
    DeSerializedData d;

    int *ptr = (int *)data;
    d.responseType = *ptr++;

    char *c_ptr = (char *)ptr;
    while (*c_ptr != '%')
        d.playername.push_back(*c_ptr++);

    return d;
}

DeSerializedData Serializer::DeSerializeUpdatePlayerResponse(const char data[])
{
    DeSerializedData d;
    int *ptr = (int *)data;

    d.responseType = *ptr++;
    d.score = *ptr++;
    d.opponent_pos.first = *ptr++;
    d.opponent_pos.second = *ptr++;
    d.len = *ptr++;

    int x;
    int y;

    for (int i = 0; i < d.len; i++) {
        x = *ptr++;
        y = *ptr++;

        d.visited.push_back({x, y});
    }

    return d;
}

DeSerializedData Serializer::DeSerializeNewOpponentResponse(const char data[])
{
    DeSerializedData d;
    int *ptr = (int *)data;

    d.responseType = *ptr++;
    d.userID = *ptr++;
    d.isPlayer2 = *ptr++;

    int len = *ptr++;

    char *c_ptr = (char *)ptr;
    for (int i = 0; i < len; i++)
        d.playername.push_back(*c_ptr++);

    return d;
}
