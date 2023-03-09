#include "../include/ui.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/dom/elements.hpp>

#include <fstream>
#include <iostream>

UI::UI()
    : show_userinput(true),
      show_map(false),
      player_name(""),
      opponent_name(""),
      player_x(-1),
      player_y(-1),
      player_score(0),
      opponent_x(-1),
      opponent_y(-1),
      opponent_score(0),
      quit(false)
{
}

string UI::displayGetUserName()
{
    // setup UI componenets to show user input and an OK button
    ftxui::Component username_input = ftxui::Input(&this->player_name, "Player Name");
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
        return ftxui::vbox({
                   ftxui::hbox(
                       {ftxui::text(" Player Name: ") | ftxui::bold | ftxui::vcenter,
                        inner}),
                   ftxui::separator(),
                   ftxui::text("Make sure your terminal is large enough to see the map."),
               }) |
               ftxui::center;
    });

    this->screen.Loop(userinput_container);
    return this->player_name;
}

void UI::displayDisconnect(string name)
{
    ftxui::Component okay = ftxui::Button("Okay", this->screen.ExitLoopClosure());
    ftxui::Component container = ftxui::Container::Horizontal({okay});

    container |= ftxui::Renderer([&](ftxui::Element inner) {
        return ftxui::hbox({
                   ftxui::text(name + " has left. "),
                   ftxui::separator(),
                   inner,
               }) |
               ftxui::center;
    });

    this->screen.Loop(container);
}

void UI::displayEndGameScore(LeaderBoard board)
{
    std::vector<std::vector<std::string>> table_rows;
    table_rows.push_back({
        "Game",
        "Player 1",
        "Score",
        "Player 2",
        "Score",
        "Winner",
    });

    for (int i = 0; i < board.size(); i++) {
        auto row = board[i];
        std::vector<std::string> r;

        r.push_back(std::to_string(i + 1));
        r.push_back(std::get<0>(row));
        r.push_back(std::to_string(std::get<1>(row)));
        r.push_back(std::get<2>(row));
        r.push_back(std::to_string(std::get<3>(row)));

        if (std::get<1>(row) > std::get<3>(row))
            r.push_back(std::get<0>(row));
        else if (std::get<1>(row) < std::get<3>(row))
            r.push_back(std::get<2>(row));
        else
            r.push_back("Tie");

        table_rows.push_back(r);
    }

    // create a table
    auto table = ftxui::Table(table_rows);

    table.SelectAll().Border(ftxui::LIGHT);

    // Add border around the winner column.
    table.SelectColumn(5).Border(ftxui::LIGHT);

    // Make first row bold with a double border.
    table.SelectRow(0).Decorate(ftxui::bold);
    table.SelectRow(0).SeparatorVertical(ftxui::LIGHT);
    table.SelectRow(0).Border(ftxui::DOUBLE);

    // right align the scores and winner column
    table.SelectColumn(2).DecorateCells(ftxui::align_right);
    table.SelectColumn(4).DecorateCells(ftxui::align_right);
    table.SelectColumn(5).DecorateCells(ftxui::align_right);

    // Select row from the second to the last and alternate between colors
    auto content = table.SelectRows(1, -1);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Blue), 3, 0);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Cyan), 3, 1);
    content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::White), 3, 2);

    auto document = table.Render();
    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(document));
    Render(screen, document);
    screen.Print();
    std::cout << "\n";
}

// render the final canvas and return the game loop
ftxui::Loop UI::getGameLoop()
{
    ftxui::Component quit_button = ftxui::Button("Quit", [&] {
        this->quit = true;
        this->screen.ExitLoopClosure();
    });

    // render the canvas in a window
    this->canvas |= ftxui::Renderer([&](ftxui::Element inner) {
        return ftxui::window(
            ftxui::text("Pac-Man-[kinda]") | ftxui::hcenter,
            ftxui::vbox({
                ftxui::text(this->player_name + " - you are the red player"),
                inner | ftxui::center,
                ftxui::separator(),
                ftxui::hbox({
                    ftxui::text(this->player_name + " " +
                                std::to_string(this->player_score) + " ") |
                        ftxui::bold,
                    ftxui::separator(),
                    ftxui::text(" " + this->opponent_name + " " +
                                std::to_string(this->opponent_score)) |
                        ftxui::bold,
                }) | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 5),
            }) | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 10));
    });

    ftxui::Component canvas_container = ftxui::Container::Vertical({
        this->canvas,
        quit_button | ftxui::align_right,
    });

    ftxui::Loop loop(&this->screen, canvas_container);
    return loop;
}

// setup the canvas and userinput. this needs to be called before getGameLoop is called
void UI::setGameMap(vector<string> map, bool player2, string player2Name)
{
    this->mapData = map;
    this->isPlayer2 = player2;
    this->opponent_name = player2Name;

    this->setupCanvas();
    this->setupUserInput();
}

// this captures keyboard input on the canvas component
void UI::setupUserInput()
{
    // capture keyboard events on the canvas component
    this->canvas |= ftxui::CatchEvent([&](ftxui::Event event) {
        if (event == ftxui::Event::Custom) {
            return false;
        }

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
                        std::lock_guard<std::mutex> lock(this->mutex);

                        this->player_score += 5;
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
                        std::lock_guard<std::mutex> lock(this->mutex);

                        this->player_score += 5;
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
                        std::lock_guard<std::mutex> lock(this->mutex);

                        this->player_score += 5;
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
                        std::lock_guard<std::mutex> lock(this->mutex);

                        this->player_score += 5;
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
}

// this functions sets up a ftxui canvas. this is the component the map and players are
// drawn to
void UI::setupCanvas()
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
                    if (!isPlayer2) {
                        if (player_x == -1)
                            player_x = x;
                        if (player_y == -1)
                            player_y = y;
                    } else {
                        if (opponent_x == -1)
                            opponent_x = x;
                        if (opponent_y == -1)
                            opponent_y = y;
                    }

                    break;
                case 'Y':
                    // get the initial position of player 2
                    if (isPlayer2) {
                        if (player_x == -1)
                            player_x = x;
                        if (player_y == -1)
                            player_y = y;
                    } else {
                        if (opponent_x == -1)
                            opponent_x = x;
                        if (opponent_y == -1)
                            opponent_y = y;
                    }

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
}

void UI::updateMap(vector<Position> visited)
{
    for (const Position &pos : visited)
        mapData[pos.first][pos.second] = ' ';
}

int UI::getScore()
{
    return this->player_score;
}

vector<std::pair<int, int>> UI::getMovements()
{
    std::lock_guard<std::mutex> lock(this->mutex);
    auto moves = this->movement;

    // this->movement.clear();
    return moves;
}

void UI::setOpponentPosition(int x, int y)
{
    this->opponent_x = x;
    this->opponent_y = y;
}

void UI::setOpponentScore(int score)
{
    this->opponent_score = score;
}

std::pair<int, int> UI::getPosition()
{
    return std::make_pair(player_x, player_y);
}

void UI::tick()
{
    this->screen.PostEvent(ftxui::Event::Custom);
}

bool UI::hasQuit()
{
    return quit;
}
