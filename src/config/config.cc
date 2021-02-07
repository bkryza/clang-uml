#include "config.h"

namespace clanguml {
namespace config {

config load(const std::string &config_file)
{
    YAML::Node doc = YAML::LoadFile(config_file);

    return doc.as<config>();
}
}
}

