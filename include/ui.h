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

    void initalizeUI(int, int);
    void displayGetUserName();
    void displayGameMap(vector<string>);

    string username() const;

private:
    int canvas_width;
    int canvas_height;
    bool show_userinput;
    bool show_map;
    string user_name;
    vector<string> mapData;

    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
};