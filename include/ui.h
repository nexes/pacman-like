#pragma once

#include "../include/my_types.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <string>
#include <vector>

using std::string;
using std::vector;

// This class handles drawing the UI and listening and processing the users keyboard
// events.
class UI
{
public:
    UI();

    // display ther username UI and return the the name the user inputs
    string displayGetUserName();

    // give the map to the UI. This needs to be called before displayGameMap()
    void setGameMap(vector<string> map, bool player2);

    // return the FTXUI game loop object to 'step' the game loop
    ftxui::Loop getGameLoop();

    // update the map to reflect player movement removing 'coins'
    void updateMap(vector<Position> visited);

    // get the current player score
    int getScore();

    // get the players position
    Position getPosition();

    // update where the opponent is at on the map
    void setOpponentPosition(int x, int y);

    // update the opponents score
    void setOpponentScore(int score);

    // get the current player movement history.
    vector<Position> getMovements();

private:
    void setupCanvas();
    void setupUserInput();

private:
    bool show_userinput;
    bool show_map;
    bool isPlayer2;
    string user_name;

    // player 1's position
    int player_x;
    int player_y;
    int player_score;

    // player 2's position
    int opponent_x;
    int opponent_y;
    int opponent_score;

    vector<string> mapData;
    vector<Position> movement;

    ftxui::Component canvas;
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
};