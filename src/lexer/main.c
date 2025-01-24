#include "main.h"

int main(int argc, char **argv) {
    DEBUG_ME;

    if (argc < 2) {
        return handle_missing_arguments();
    }

    language_map_t selected_language = language_maps[0];

    int result = process_language_flag(argc, argv, &selected_language);
    if (result != EXIT_SUCCESS) {
        return result;
    }

    return process_command(argc, argv, selected_language);
}
