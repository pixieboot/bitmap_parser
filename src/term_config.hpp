#ifndef term_config_H_
#define term_config_H_

#include <cassert>
#include <fstream>
#include <stdexcept>
#include <string>

struct TermArgsConfig {
  std::string arg1{}; // arg1: -r, -w or -e for read, write or edit
  std::string arg2{}; // arg2: path to/of the image
  std::string arg3{}; // arg3: width of the image
  std::string arg4{}; // arg4: height of the image
  std::string arg5{}; // arg5: blue
  std::string arg6{}; // arg6: green
  std::string arg7{}; // arg7: red
  std::string arg8{}; // arg8: alpha

  /**
   * Parses given args from the term
   *
   * @oaram: int argc; the amount of args
   * @param: char **argv; array of args from the terminal
   * @return: struct config; data struct for args in std::string format
   * (cfg.arg1, cfg.arg2...)
   */
  void parseTermArgs(int argc, char **argv, TermArgsConfig &cfg) {
    std::string_view help = "help";
    std::string_view h = "-h";
    std::string_view read = "-r";
    std::string_view write = "-w";
    std::string_view edit = "-e";

    if (argc <= 1) {
      throw std::runtime_error(
          "> Missing arguments\n> Type 'help' or '-h' for manual");
    }

    if (!help.compare(argv[1]) || !h.compare(argv[1])) {
      cfg.arg1 = argv[1];
      return;
    }

    if (!read.compare(argv[1]) || !write.compare(argv[1]) ||
        !edit.compare(argv[1])) {
      cfg.arg1 = argv[1];
    } else {
      throw std::runtime_error("> Invalid first argument\n"
                               "> Command read (-r), write (-w), edit "
                               "(-e) or help (-h) are required as "
                               "the first argument\n"
                               "> Type 'help' or '-h' for manual");
    }

    if (!argv[2]) {
      throw std::runtime_error("> Path to the image is required as the second "
                               "argument\n> Type 'help' or '-h' for manual");
    }

    if (!read.compare(argv[1]) || !edit.compare(argv[1])) {
      std::ifstream valid_path(argv[2]);
      if (!valid_path) {
        std::string path = argv[2];
        throw std::runtime_error("> '" + path +
                                 "' is invalid path to the file "
                                 "\n> Type 'help' or '-h' for manual");
      }
    }
    cfg.arg2 = argv[2];

    if (argv[3]) {
      cfg.arg3 = argv[3];
    }
    if (argv[4]) {
      cfg.arg4 = argv[4];
    }
    if (argv[5]) {
      cfg.arg5 = argv[5];
    }
    if (argv[6]) {
      cfg.arg6 = argv[6];
    }
    if (argv[7]) {
      cfg.arg7 = argv[7];
    }
    if (argv[8]) {
      cfg.arg8 = argv[8];
    }
  }
};

#endif // term_config_H_
