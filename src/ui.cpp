#include "../include/ui.h"

#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>

UI::UI()
    : canvas_width(100),
      canvas_height(100),
      show_userinput(true),
      show_map(false),
      user_name("")
{
}

// return the username the player inputed
string UI::username() const
{
    return this->user_name;
}

void UI::displayGetUserName()
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
                   ftxui::text(" Username: ") | ftxui::bold | ftxui::vcenter,
                   inner,
               }) |
               ftxui::center;
    });

    this->screen.Loop(userinput_container);
}

void UI::displayGameMap(vector<string> mapLines)
{
    int width = this->canvas_width;
    int height = this->canvas_height;
    string key_press = "";

    // create a canvas component that will display the pac-man map
    ftxui::Component canvas_component = ftxui::Renderer([&] {
        ftxui::Canvas c = ftxui::Canvas(width, height);
        int x = 0;
        int y = 0;
        int x_space = 2;
        int y_space = 4;

        for (std::string line : mapLines) {
            for (int i = 0; i < line.length(); i++) {
                switch (line[i]) {
                case '-':
                    c.DrawBlockLine(x, y, x + x_space, y, ftxui::Color::Blue);
                    break;
                case '|':
                    c.DrawBlockLine(x, y, x, y + y_space, ftxui::Color::Blue);
                    break;
                case '.':
                    c.DrawPoint(x, y, true, ftxui::Color::Yellow);
                    break;
                case 'X':
                    c.DrawBlockCircleFilled(x, y, 1, ftxui::Color::Red);
                    break;
                case 'Y':
                    c.DrawBlockCircleFilled(x, y, 1, ftxui::Color::Green);
                    break;
                }

                x += x_space;
            }

            y += y_space;
            x = 0;
        }

        // returns our canvas as an 'Element'
        return ftxui::canvas(std::move(c));
    });

    // capture keyboard events on the canvas component
    canvas_component |= ftxui::CatchEvent([&](ftxui::Event event) {
        if (!event.is_mouse()) {
            if (event == ftxui::Event::ArrowDown) {
                key_press = "DOWN";
                return true;
            } else if (event == ftxui::Event::ArrowLeft) {
                key_press = "LEFT";
                return true;
            } else if (event == ftxui::Event::ArrowRight) {
                key_press = "RIGHT";
                return true;
            } else if (event == ftxui::Event::ArrowUp) {
                key_press = "UP";
                return true;
            }
        }

        // return false so the event keeps bubbling up
        return false;
    });

    // render the canvas in a window
    canvas_component |= ftxui::Renderer([&](ftxui::Element inner) {
        return ftxui::window(ftxui::text("Pac-Man-[kinda]") | ftxui::hcenter,
                             ftxui::vbox({
                                 inner,
                                 ftxui::text(key_press),
                             }));
    });

    this->screen.Loop(canvas_component);
}

void UI::initalizeUI(int width, int height)
{
    canvas_width = width;
    canvas_height = height;
}
