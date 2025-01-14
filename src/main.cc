/**
 * \file
 * \brief Main function
 */
#include <cstdlib>  // for EXIT_SUCCESS
#include <exception>
#include <string>

#include "audio/player.h"                          // for Player
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "middleware/media_controller.h"           // for MediaController
#include "util/arg_parser.h"                       // for ArgumentParser
#include "util/logger.h"                           // For Logger
#include "view/base/terminal.h"                    // for Terminal

//! Command-line argument parsing
bool parse(int argc, char** argv, std::string& path) {
  using util::Argument;
  using util::ExpectedArguments;
  using util::ParsedArguments;
  using util::Parser;

  try {
    // Create arguments expectation
    auto expected_args = ExpectedArguments{
        Argument{
            .name = "log",
            .choices = {"-l", "--log"},
            .description = "Enable logging to specified path",
        },
        Argument{
            .name = "directory",
            .choices = {"-d", "--directory"},
            .description = "Initialize listing files from the given directory path",
        },
    };

    // Configure argument parser and run to get parsed arguments
    Parser arg_parser = util::ArgumentParser::Configure(expected_args);
    ParsedArguments parsed_args = arg_parser->Parse(argc, argv);

    // Check if contains filepath for logging
    if (auto logging_path = parsed_args["log"]; logging_path) {
      // Enable logging to specified path
      util::Logger::GetInstance().Configure(*logging_path);
    }

    // Check if contains dirpath for initial file listing
    if (auto initial_path = parsed_args["directory"]; initial_path) {
      path = *initial_path;
    }

  } catch (util::parsing_error&) {
    // Got some error while trying to parse, or even received help as argument
    // Just let ArgumentParser handle it
    return false;
  }

  return true;
}

/* ********************************************************************************************** */

int main(int argc, char** argv) {
  // In case of getting some unexpected argument or some other error:
  // Do not execute the program
  std::string initial_dir;
  if (!parse(argc, argv, initial_dir)) {
    return EXIT_SUCCESS;
  }

  // Create and initialize a new player
  auto player = audio::Player::Create();

  // Create and initialize a new terminal window
  auto terminal = interface::Terminal::Create(initial_dir);

  // Use terminal maximum width as input to decide how many bars should display on audio visualizer
  int number_bars = terminal->CalculateNumberBars();

  // Create and initialize a new middleware for terminal and player
  auto middleware = middleware::MediaController::Create(terminal, player, number_bars);

  // Register callbacks to Terminal and Player
  terminal->RegisterPlayerNotifier(middleware);
  player->RegisterInterfaceNotifier(middleware);

  // Create a full-size screen and register callbacks
  ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();

  terminal->RegisterEventSenderCallback([&screen](const ftxui::Event& e) { screen.PostEvent(e); });
  terminal->RegisterExitCallback([&screen]() { screen.ExitLoopClosure()(); });

  // Set hidden cursor, start GUI loop and clear screen after exit
  screen.SetCursor(ftxui::Screen::Cursor{.shape = ftxui::Screen::Cursor::Shape::Hidden});
  screen.Loop(terminal);
  screen.ResetPosition(true);

  return EXIT_SUCCESS;
}
