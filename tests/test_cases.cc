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
//#define CATCH_CONFIG_MAIN

#include "test_cases.h"

#include <spdlog/spdlog.h>

std::pair<clanguml::config::config, compilation_database> load_config(
    const std::string &test_name)
{
    auto config = clanguml::config::load(test_name + "/.clanguml");

    auto db = clanguml::cx::compilation_database::from_directory(
        config.compilation_database_dir);

    return std::make_pair(std::move(config), std::move(db));
}

clanguml::model::sequence_diagram::diagram generate_sequence_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    auto diagram_model =
        clanguml::generators::sequence_diagram::generate(db, diagram->name,
            dynamic_cast<clanguml::config::sequence_diagram &>(*diagram));

    return diagram_model;
}

clanguml::model::class_diagram::diagram generate_class_diagram(
    compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    auto diagram_model =
        clanguml::generators::class_diagram::generate(db, diagram->name,
            dynamic_cast<clanguml::config::class_diagram &>(*diagram));

    return diagram_model;
}

std::string generate_sequence_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::sequence_diagram::diagram &model)
{
    using namespace clanguml::generators::sequence_diagram::puml;

    std::stringstream ss;

    ss << generator(
        dynamic_cast<clanguml::config::sequence_diagram &>(*config), model);

    return ss.str();
}

std::string generate_class_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::model::class_diagram::diagram &model)
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

using clanguml::test::matchers::Abstract;
using clanguml::test::matchers::AliasMatcher;
using clanguml::test::matchers::Const;
using clanguml::test::matchers::Default;
using clanguml::test::matchers::HasCall;
using clanguml::test::matchers::HasCallWithResponse;
using clanguml::test::matchers::IsAbstractClass;
using clanguml::test::matchers::IsAggregation;
using clanguml::test::matchers::IsAssociation;
using clanguml::test::matchers::IsBaseClass;
using clanguml::test::matchers::IsClass;
using clanguml::test::matchers::IsClassTemplate;
using clanguml::test::matchers::IsComposition;
using clanguml::test::matchers::IsEnum;
using clanguml::test::matchers::IsField;
using clanguml::test::matchers::IsFriend;
using clanguml::test::matchers::IsInnerClass;
using clanguml::test::matchers::IsInstantiation;
using clanguml::test::matchers::Private;
using clanguml::test::matchers::Protected;
using clanguml::test::matchers::Public;
using clanguml::test::matchers::Static;

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

    if (debug_log)
        spdlog::default_logger_raw()->set_level(spdlog::level::debug);

    return session.run();
}
