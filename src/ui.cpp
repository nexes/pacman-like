#include "../include/my_types.h"
#include "../include/ui.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>

#include <fstream>
#include <iostream>

UI::UI()
    : show_userinput(true),
      show_map(false),
      user_name(""),
      player_x(-1),
      player_y(-1),
      player_score(0),
      opponent_x(-1),
      opponent_y(-1),
      opponent_score(0)
{
}

string UI::displayGetUserName()
{
    // setup UI componenets to show user input and an OK button
    ftxui::Component username_input = ftxui::Input(&this->user_name, "Player Name");
    ftxui::Component ok_button = ftxui::Button("Connect", this->screen.ExitLoopClosure());

    // size the user input box
    username_input |= ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 15) |
                      ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 1) | ftxui::vcenter;
    // size the okay button
    ok_button |= ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 3) |
                 ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 10);

    // place both UI components into one container
    ftxui::Component userinput_container = ftxui::Container::Horizontal({
        username_input,
        ok_button,
    });

    // render the userinput container in a hbox
    userinput_container |= ftxui::Renderer([&](ftxui::Element inner) {
        return ftxui::hbox({
                   ftxui::text(" Player Name: ") | ftxui::bold | ftxui::vcenter,
                   inner,
               }) |
               ftxui::center;
    });

    this->screen.Loop(userinput_container);
    return this->user_name;
}

void UI::displayGameMap()
{
    // create a canvas component that will display the pac-man map
    this->canvas = ftxui::Renderer([&] {
        ftxui::Canvas c = ftxui::Canvas(GameInfo::CanvasWidth, GameInfo::CanvasHeight);
        int x_space = 2;
        int y_space = 4;
        int x = -x_space;
        int y = -y_space;

        // draw the map
        int rows = this->mapData.size();
        for (int i = 0; i < rows; i++) {
            string line = this->mapData[i];
            y += y_space;

            for (int j = 0; j < line.length(); j++) {
                x += x_space;

                switch (line[j]) {
                case ' ':
                    c.DrawBlock(x, y, false);
                    break;
                case '-':
                    c.DrawPointLine(x, y, x + x_space, y, ftxui::Color::Blue);
                    break;
                case '|':
                    c.DrawPointLine(x, y, x, y + y_space, ftxui::Color::Blue);
                    break;
                case '.': {
                    c.DrawPoint(x, y, true, ftxui::Color::Yellow);
                    break;
                }
                case 'X':
                    // get the initial position of player 1
                    if (player_x == -1)
                        player_x = x;
                    if (player_y == -1)
                        player_y = y;
                    break;
                case 'Y':
                    // get the initial position of player 2
                    if (opponent_x == -1)
                        opponent_x = x;
                    if (opponent_y == -1)
                        opponent_y = y;

                    break;
                }
            }
            x = 0;
        }

        // draw our two players after the map is drawn
        c.DrawBlockEllipseFilled(player_x + x_space, player_y, 1, 1, ftxui::Color::Red);
        c.DrawBlockEllipseFilled(opponent_x + x_space,
                                 opponent_y,
                                 1,
                                 1,
                                 ftxui::Color::Green);

        // returns our canvas as an 'Element'
        return ftxui::canvas(std::move(c));
    });

    // capture keyboard events on the canvas component
    this->canvas |= ftxui::CatchEvent([&](ftxui::Event event) {
        if (!event.is_mouse()) {
            int rowSize = this->mapData.size();
            int colIdx = player_x / 2;
            int rowIdx = player_y / 4;

            // handle player moving down
            if (event == ftxui::Event::ArrowDown) {
                if (rowIdx + 1 < rowSize) {
                    char point = this->mapData[rowIdx + 1][colIdx];

                    if (point != '|' && point != '-')
                        player_y += 4;

                    if (point == '.') {
                        player_score += 5;
                        this->mapData[rowIdx][colIdx] = ' ';
                        movement.push_back({rowIdx, colIdx});
                    }
                }
                return true;

                // handle player moving left
            } else if (event == ftxui::Event::ArrowLeft) {
                if (colIdx - 1 >= 0) {
                    char point = this->mapData[rowIdx][colIdx - 1];

                    if (point != '|' && point != '-')
                        player_x -= 2;

                    if (point == '.') {
                        player_score += 5;
                        this->mapData[rowIdx][colIdx] = ' ';
                        movement.push_back({rowIdx, colIdx});
                    }
                }
                return true;

                // handle player moving right
            } else if (event == ftxui::Event::ArrowRight) {
                // each line (row) can have a different size
                int colSize = this->mapData[rowIdx].size();
                if (colIdx + 1 < colSize) {
                    char point = this->mapData[rowIdx][colIdx + 1];

                    if (point != '|' && point != '-')
                        player_x += 2;

                    if (point == '.') {
                        player_score += 5;
                        this->mapData[rowIdx][colIdx] = ' ';
                        movement.push_back({rowIdx, colIdx});
                    }
                }
                return true;

                // handle player moving up
            } else if (event == ftxui::Event::ArrowUp) {
                if (rowIdx - 1 >= 0) {
                    char point = this->mapData[rowIdx - 1][colIdx];

                    if (point != '|' && point != '-')
                        player_y -= 4;

                    if (point == '.') {
                        player_score += 5;
                        this->mapData[rowIdx][colIdx] = ' ';
                        movement.push_back({rowIdx, colIdx});
                    }
                }
                return true;
            }
        }

        // return false so the event keeps bubbling up
        return false;
    });

    // render the canvas in a window
    this->canvas |= ftxui::Renderer([&](ftxui::Element inner) {
        return ftxui::window(ftxui::text("Pac-Man-[kinda]") | ftxui::hcenter,
                             ftxui::vbox({inner}));
    });

    this->screen.Loop(this->canvas);
}

void UI::setGameMap(vector<string> map)
{
    this->mapData = map;
}

int UI::getScore()
{
    return player_score;
}

vector<std::pair<int, int>> UI::getMovements()
{
    // clear the list sense they've been handled
    auto moves = this->movement;
    this->movement.clear();

    return moves;
}

void UI::setOpponentPosition(int x, int y)
{
    opponent_x = x;
    opponent_y = y;
}

void UI::setOpponentScore(int score)
{
    opponent_score = score;
}

std::pair<int, int> UI::getPosition()
{
    return std::make_pair(player_x, player_y);
}
