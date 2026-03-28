#include "term_config.hpp"
#include "BMP.hpp"

int main(int argc, char *argv[]) {
  try {
    TermArgsConfig term{};
    term.parseTermArgs(argc, argv, term);

    BMP bmp{};
    bmp.parseInput(term);
  } catch (std::exception const &e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return 0;
}
