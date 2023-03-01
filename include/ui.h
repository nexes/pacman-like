#pragma once

#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

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
    void setGameMap(vector<string>);

    // display the game map the server sent
    void displayGameMap();

    // get the current player score
    int getScore();

    // get the players position
    std::pair<int, int> getPosition();

    // update where the opponent is at on the map
    void setOpponentPosition(int, int);

    // update the opponents score
    void setOpponentScore(int);

    // get the current player movement history. When called this will return a list of (x,
    // y) cords the player has moved to. This will clear the movement list each time its
    // called
    vector<std::pair<int, int>> getMovements();

private:
    bool show_userinput;
    bool show_map;
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
    vector<std::pair<int, int>> movement;

    ftxui::Component canvas;
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
};