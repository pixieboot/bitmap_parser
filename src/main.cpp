#include "BMP.hpp"

int main([[maybe_unused]] int argc, char *argv[]) {
    try {
        config cfg{};
        BMP::parse_command_params(argv, cfg);
        if (cfg.arg1 == "help" || cfg.arg1 == "-h") {
            BMP::showManual();
            return 0;
        }

        Buf b{};
        BMP::loadFileToBuf(b, cfg.arg1);

        BMP bmp{};
        bmp.parseBmpData(b, cfg.arg2);
    } catch (std::exception const &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return 0;
}
