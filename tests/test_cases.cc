/**
 * tests/test_cases.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "test_cases.h"

#include <spdlog/spdlog.h>

std::pair<clanguml::config::config, cppast::libclang_compilation_database>
load_config(const std::string &test_name)
{
    auto config = clanguml::config::load(test_name + "/.clang-uml");

    cppast::libclang_compilation_database db(config.compilation_database_dir);

    return std::make_pair(std::move(config), std::move(db));
}

std::pair<clanguml::config::config, compilation_database> load_config2(
    const std::string &test_name)
{
    auto config = clanguml::config::load(test_name + "/.clang-uml");

    auto db = clanguml::cx::compilation_database::from_directory(
        config.compilation_database_dir);

    return std::make_pair(std::move(config), std::move(db));
}

clanguml::sequence_diagram::model::diagram generate_sequence_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    auto diagram_model =
        clanguml::generators::sequence_diagram::generate(db, diagram->name,
            dynamic_cast<clanguml::config::sequence_diagram &>(*diagram));

    return diagram_model;
}

clanguml::class_diagram::model::diagram generate_class_diagram(
    cppast::libclang_compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    auto diagram_model =
        clanguml::generators::class_diagram::generate(db, diagram->name,
            dynamic_cast<clanguml::config::class_diagram &>(*diagram));

    return diagram_model;
}

std::string generate_sequence_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::sequence_diagram::model::diagram &model)
{
    using namespace clanguml::generators::sequence_diagram::puml;

    std::stringstream ss;

    ss << generator(
        dynamic_cast<clanguml::config::sequence_diagram &>(*config), model);

    return ss.str();
}

std::string generate_class_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::class_diagram::model::diagram &model)
{
    using namespace clanguml::generators::class_diagram::puml;

    std::stringstream ss;

    ss << generator(
        dynamic_cast<clanguml::config::class_diagram &>(*config), model);

    return ss.str();
}

void save_puml(const std::string &path, const std::string &puml)
{
    std::filesystem::path p{path};
    std::filesystem::create_directory(p.parent_path());
    std::ofstream ofs;
    ofs.open(p, std::ofstream::out | std::ofstream::trunc);
    ofs << puml;
    ofs.close();
}

using namespace clanguml::test::matchers;

//
// Class diagram tests
//
#include "t00002/test_case.h"
#include "t00003/test_case.h"
#include "t00004/test_case.h"
#include "t00005/test_case.h"
#include "t00006/test_case.h"
#include "t00007/test_case.h"
#include "t00008/test_case.h"
#include "t00009/test_case.h"
#include "t00010/test_case.h"
#include "t00011/test_case.h"
#include "t00012/test_case.h"
#include "t00013/test_case.h"
#include "t00014/test_case.h"
#include "t00015/test_case.h"
#include "t00016/test_case.h"
#include "t00017/test_case.h"
#include "t00018/test_case.h"
#include "t00019/test_case.h"
#include "t00020/test_case.h"
#include "t00021/test_case.h"
#include "t00022/test_case.h"
#include "t00023/test_case.h"
#include "t00024/test_case.h"
#include "t00025/test_case.h"
#include "t00026/test_case.h"
#include "t00027/test_case.h"
#include "t00028/test_case.h"
#include "t00029/test_case.h"
#include "t00030/test_case.h"
#include "t00031/test_case.h"
#include "t00032/test_case.h"
#include "t00033/test_case.h"

//
// Sequence diagram tests
//
#include "t20001/test_case.h"

//
// Other tests (e.g. configuration file)
//
#include "t90000/test_case.h"

//
// Main test function
//
int main(int argc, char *argv[])
{
    Catch::Session session;
    using namespace Catch::clara;

    bool debug_log{false};
    auto cli = session.cli() |
        Opt(debug_log, "debug_log")["-u"]["--debug-log"]("Enable debug logs");

    session.cli(cli);

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
        return returnCode;

    if (debug_log) {
        spdlog::default_logger_raw()->set_level(spdlog::level::debug);
        spdlog::default_logger_raw()->set_pattern("[%l] %v");
    }

    return session.run();
}
