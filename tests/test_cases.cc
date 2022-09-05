/**
 * tests/test_cases.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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
#include "common/generators/plantuml/generator.h"

#include <spdlog/spdlog.h>

void inject_diagram_options(std::shared_ptr<clanguml::config::diagram> diagram)
{
    // Inject links config to all test cases
    clanguml::config::generate_links_config links_config{
        R"(https://github.com/bkryza/clang-uml/blob/{{ git.commit }}/{{ element.source.path }}#L{{ element.source.line }})",
        R"({% if existsIn(element, "comment") and existsIn(element.comment, "brief")  %}{{ abbrv(trim(replace(element.comment.brief.0, "\n+", " ")), 256) }}{% else %}{{ element.name }}{% endif %})"};

    diagram->generate_links.set(links_config);
}

std::pair<clanguml::config::config,
    std::unique_ptr<clang::tooling::CompilationDatabase>>
load_config(const std::string &test_name)
{
    auto config = clanguml::config::load(test_name + "/.clang-uml");

    std::string err{};
    auto compilation_database =
        clang::tooling::CompilationDatabase::autoDetectFromDirectory(
            config.compilation_database_dir(), err);

    if (!err.empty())
        throw std::runtime_error{err};

    return std::make_pair(std::move(config), std::move(compilation_database));
}

std::unique_ptr<clanguml::sequence_diagram::model::diagram>
generate_sequence_diagram(clang::tooling::CompilationDatabase &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    using diagram_config = clanguml::config::sequence_diagram;
    using diagram_model = clanguml::sequence_diagram::model::diagram;
    using diagram_visitor =
        clanguml::sequence_diagram::visitor::translation_unit_visitor;

    inject_diagram_options(diagram);

    auto model = clanguml::common::generators::plantuml::generate<diagram_model,
        diagram_config, diagram_visitor>(db, diagram->name,
        dynamic_cast<clanguml::config::sequence_diagram &>(*diagram),
        diagram->get_translation_units(std::filesystem::current_path()));

    return model;
}

std::unique_ptr<clanguml::class_diagram::model::diagram> generate_class_diagram(
    clang::tooling::CompilationDatabase &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    using diagram_config = clanguml::config::class_diagram;
    using diagram_model = clanguml::class_diagram::model::diagram;
    using diagram_visitor =
        clanguml::class_diagram::visitor::translation_unit_visitor;

    inject_diagram_options(diagram);

    auto model = clanguml::common::generators::plantuml::generate<diagram_model,
        diagram_config, diagram_visitor>(db, diagram->name,
        dynamic_cast<diagram_config &>(*diagram),
        diagram->get_translation_units(std::filesystem::current_path()));

    return model;
}

std::unique_ptr<clanguml::package_diagram::model::diagram>
generate_package_diagram(clang::tooling::CompilationDatabase &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    using diagram_config = clanguml::config::package_diagram;
    using diagram_model = clanguml::package_diagram::model::diagram;
    using diagram_visitor =
        clanguml::package_diagram::visitor::translation_unit_visitor;

    inject_diagram_options(diagram);

    return clanguml::common::generators::plantuml::generate<diagram_model,
        diagram_config, diagram_visitor>(db, diagram->name,
        dynamic_cast<diagram_config &>(*diagram),
        diagram->get_translation_units(std::filesystem::current_path()));
}

std::unique_ptr<clanguml::include_diagram::model::diagram>
generate_include_diagram(clang::tooling::CompilationDatabase &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    using diagram_config = clanguml::config::include_diagram;
    using diagram_model = clanguml::include_diagram::model::diagram;
    using diagram_visitor =
        clanguml::include_diagram::visitor::translation_unit_visitor;

    inject_diagram_options(diagram);

    return clanguml::common::generators::plantuml::generate<diagram_model,
        diagram_config, diagram_visitor>(db, diagram->name,
        dynamic_cast<diagram_config &>(*diagram),
        diagram->get_translation_units(std::filesystem::current_path()));
}

std::string generate_sequence_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::sequence_diagram::model::diagram &model)
{
    using namespace clanguml::sequence_diagram::generators::plantuml;

    std::stringstream ss;

    ss << generator(
        dynamic_cast<clanguml::config::sequence_diagram &>(*config), model);

    return ss.str();
}

std::string generate_class_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::class_diagram::model::diagram &model)
{
    using namespace clanguml::class_diagram::generators::plantuml;

    std::stringstream ss;

    ss << generator(
        dynamic_cast<clanguml::config::class_diagram &>(*config), model);

    return ss.str();
}

std::string generate_package_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::package_diagram::model::diagram &model)
{
    using namespace clanguml::package_diagram::generators::plantuml;

    std::stringstream ss;

    assert(config.get() != nullptr);

    ss << generator(
        dynamic_cast<clanguml::config::package_diagram &>(*config), model);

    return ss.str();
}

std::string generate_include_puml(
    std::shared_ptr<clanguml::config::diagram> config,
    clanguml::include_diagram::model::diagram &model)
{
    using namespace clanguml::include_diagram::generators::plantuml;

    std::stringstream ss;

    assert(config.get() != nullptr);

    ss << generator(
        dynamic_cast<clanguml::config::include_diagram &>(*config), model);

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

///
/// Class diagram tests
///
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
#include "t00034/test_case.h"
#include "t00035/test_case.h"
#include "t00036/test_case.h"
#include "t00037/test_case.h"
#include "t00038/test_case.h"
#include "t00039/test_case.h"
#include "t00040/test_case.h"
#include "t00041/test_case.h"
#include "t00042/test_case.h"
#include "t00043/test_case.h"
#include "t00044/test_case.h"
#include "t00045/test_case.h"
#include "t00046/test_case.h"
#include "t00047/test_case.h"
#include "t00048/test_case.h"
#include "t00049/test_case.h"
#include "t00050/test_case.h"

///
/// Sequence diagram tests
///
#include "t20001/test_case.h"
#include "t20002/test_case.h"
#include "t20003/test_case.h"

///
/// Package diagram tests
///
#include "t30001/test_case.h"
#include "t30002/test_case.h"
#include "t30003/test_case.h"
#include "t30004/test_case.h"
#include "t30005/test_case.h"
#include "t30006/test_case.h"
#include "t30007/test_case.h"
#include "t30008/test_case.h"

///
/// Include diagram tests
///
#include "t40001/test_case.h"
#include "t40002/test_case.h"
#include "t40003/test_case.h"

///
/// Other tests (e.g. configuration file)
///
#include "t90000/test_case.h"

///
/// Main test function
///
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

    clanguml::util::setup_logging(debug_log);

    return session.run();
}
