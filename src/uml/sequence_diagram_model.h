#pragma once

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace clanguml {
namespace model {
namespace sequence_diagram {

enum class message_t { kCall, kReturn };

struct message {
    message_t type;
    std::string from;
    std::string from_usr;
    std::string to;
    std::string message;
    unsigned int line;
};

struct diagram {
    std::string name;
    std::vector<message> sequence;

    void sort()
    {
        std::sort(sequence.begin(), sequence.end(),
            [](const auto &a, const auto &b) -> bool {
                if (a.from_usr == b.from_usr)
                    return a.line > b.line;

                return a.from_usr > b.from_usr;
            });
    }
};
}
}
}
