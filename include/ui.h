#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>

using std::string;
using std::vector;

class UI
{
public:
    UI();

    // display ther username UI and return the the name the user inputs
    string displayGetUserName();
    // display the game map the server sent
    void displayGameMap(vector<string>);

private:
    bool show_userinput;
    bool show_map;
    string user_name;
    vector<string> mapData;

    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
};