
#include "common.h"
#include <stdlib.h>


void process_ui_urgent_file(string ui_urgent_file_address, time_point last_ui_urgent_timestamp) {
    auto new_entries = get_new_entries(ui_urgent_file_address, last_ui_urgent_timestamp);

    for (auto & entry : new_entries) {
        auto type = entry["type"];
        auto timestamp = entry["timestamp"];
        if (type == "register") {
            // call register rpc

        } else {
            std::cerr << "unknown type: " << type << std::endl;
            exit(1);
        }
    }
}