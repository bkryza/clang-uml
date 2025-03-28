/**
 * @file tests/test_cases.cc
 *
 * Copyright (c) 2021-2025 Bartek Kryza <bkryza@gmail.com>
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

#include "cli/cli_handler.h"
#include "common/compilation_database.h"
#include "common/generators/generators.h"
#include "util/util.h"

#include <spdlog/spdlog.h>

#include <vector>

std::pair<clanguml::config::config_ptr,
    clanguml::common::compilation_database_ptr>
load_config(const std::string &test_name)
{
    using clanguml::common::string_or_regex;

    std::pair<clanguml::config::config_ptr,
        clanguml::common::compilation_database_ptr>
        res;

    // Load configuration file from the project source tree instead of
    // the test build directory
    const auto test_config_path =
        fmt::format("../../tests/{}/.clang-uml", test_name);

    const auto compilation_database_dir = canonical(
        std::filesystem::current_path() / std::filesystem::path{".."});
    const auto output_directory = weakly_canonical(
        std::filesystem::current_path() / std::filesystem::path{"diagrams"});

    res.first = std::make_unique<clanguml::config::config>(
        clanguml::config::load(test_config_path, true, false, true));

    assert(res.first.get() != nullptr);

    res.first->compilation_database_dir.set(compilation_database_dir.string());
    res.first->output_directory.set(output_directory.string());

    LOG_DBG("Loading compilation database from {}",
        res.first->compilation_database_dir());

    std::vector<string_or_regex> remove_compile_flags{
        string_or_regex{"-Wno-class-memaccess"},
        string_or_regex{"-forward-unknown-to-host-compiler"},
        string_or_regex{
            std::regex{"--generate-code=.*"}, "--generate-code=.*"}};

    res.first->remove_compile_flags.set(remove_compile_flags);

    res.second =
        clanguml::common::compilation_database::auto_detect_from_directory(
            *res.first);

    return res;
}

namespace detail {
template <typename DiagramConfig>
auto generate_diagram_impl(clanguml::common::compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    LOG_INFO("All paths will be evaluated relative to {}",
        diagram->root_directory().string());

    using diagram_config = DiagramConfig;
    using diagram_model =
        typename clanguml::common::generators::diagram_model_t<
            diagram_config>::type;
    using diagram_visitor =
        typename clanguml::common::generators::diagram_visitor_t<
            diagram_config>::type;

    auto tus = diagram->glob_translation_units(db.getAllFiles());

    auto model = clanguml::common::generators::generate<diagram_model,
        diagram_config, diagram_visitor>(
        db, diagram->name, dynamic_cast<diagram_config &>(*diagram), tus);

    return model;
}

template <typename GeneratorType, typename DiagramConfig, typename DiagramModel>
auto render_diagram(
    std::shared_ptr<clanguml::config::diagram> config, DiagramModel &model)
{
    using diagram_config = DiagramConfig;
    using diagram_model = DiagramModel;
    using diagram_generator =
        typename clanguml::common::generators::diagram_generator_t<
            DiagramConfig, typename GeneratorType::generator_tag>::type;

    std::stringstream ss;

    ss << diagram_generator(dynamic_cast<diagram_config &>(*config), model);

    if constexpr (std::is_same_v<GeneratorType, clanguml::test::json_t>) {
        return nlohmann::json::parse(ss.str());
    }
    else if constexpr (std::is_same_v<GeneratorType,
                           clanguml::test::graphml_t>) {
        pugi::xml_document g;

        g.load(ss);

        return g;
    }
    else {
        return ss.str();
    }
}
} // namespace detail

///
/// @defgroup Diagram generators
/// @{
///

std::unique_ptr<clanguml::class_diagram::model::diagram> generate_class_diagram(
    clanguml::common::compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    return detail::generate_diagram_impl<clanguml::config::class_diagram>(
        db, diagram);
}

std::unique_ptr<clanguml::sequence_diagram::model::diagram>
generate_sequence_diagram(clanguml::common::compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    return detail::generate_diagram_impl<clanguml::config::sequence_diagram>(
        db, diagram);
}

std::unique_ptr<clanguml::package_diagram::model::diagram>
generate_package_diagram(clanguml::common::compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    return detail::generate_diagram_impl<clanguml::config::package_diagram>(
        db, diagram);
}

std::unique_ptr<clanguml::include_diagram::model::diagram>
generate_include_diagram(clanguml::common::compilation_database &db,
    std::shared_ptr<clanguml::config::diagram> diagram)
{
    return detail::generate_diagram_impl<clanguml::config::include_diagram>(
        db, diagram);
}

/// }@

template <typename T>
void save_diagram(const std::filesystem::path &path, const T &diagram)
{
    static_assert(std::is_same_v<T, std::string> ||
        std::is_same_v<T, nlohmann::json> ||
        std::is_same_v<T, clanguml::common::generators::graphml::graphml_t>);

    std::filesystem::create_directories(path.parent_path());
    std::ofstream ofs;
    ofs.open(path, std::ofstream::out | std::ofstream::trunc);
    if constexpr (std::is_same_v<T, nlohmann::json>) {
        ofs << std::setw(2) << diagram;
    }
    else if constexpr (std::is_same_v<T,
                           clanguml::common::generators::graphml::graphml_t>) {
        diagram.save(ofs, " ");
    }
    else {
        ofs << diagram;
    }

    ofs.close();
}

void save_puml(const std::string &path, const std::string &filename,
    const std::string &puml)
{
    std::filesystem::path p{path};
    p /= filename;
    save_diagram(p, puml);
}

void save_json(const std::string &path, const std::string &filename,
    const nlohmann::json &j)
{
    std::filesystem::path p{path};
    p /= filename;
    save_diagram(p, j);
}

void save_mermaid(const std::string &path, const std::string &filename,
    const std::string &mmd)
{
    std::filesystem::path p{path};
    p /= filename;
    save_diagram(p, mmd);
}

void save_graphml(const std::string &path, const std::string &filename,
    const clanguml::common::generators::graphml::graphml_t &g)
{
    std::filesystem::path p{path};
    p /= filename;
    save_diagram(p, g);
}

namespace clanguml::test {

template <typename... Ts> struct diagram_source_storage {
    diagram_source_storage(Ts... diagrams)
        : srcs{std::forward<Ts>(diagrams)...}
    {
    }

    template <typename T> constexpr const T &get() const noexcept
    {
        return std::get<T>(srcs);
    }

    template <typename T> static constexpr bool contains() noexcept
    {
        return (std::is_same_v<T, Ts> || ...);
    }

    std::tuple<Ts...> srcs;
};

template <typename T, typename DSS, typename TC>
void try_run_test_case(const DSS &diagrams, TC &&tc)
{
    if constexpr (DSS::template contains<T>())
        if constexpr (std::is_invocable_v<TC, T>) {
            try {
                tc(diagrams.template get<T>());
            }
            catch (doctest::TestFailureException &e) {
                std::cout << "---------------------------------------------"
                             "--------"
                             "--------------------------\n";
                std::cout << "Test case failed for diagram type "
                          << T::diagram_type_name << ": " << "\n\n";
                std::cout << diagrams.template get<T>().to_string() << "\n";

                throw e;
            }
        }
}

template <typename DiagramType>
DiagramType render_class_diagram(std::shared_ptr<clanguml::config::diagram> c,
    clanguml::class_diagram::model::diagram &model)
{
    auto d = DiagramType{common::model::diagram_t::kClass,
        detail::render_diagram<DiagramType, clanguml::config::class_diagram>(
            c, model)};
    d.generate_packages = c->generate_packages();
    return d;
}

template <typename DiagramType>
DiagramType render_sequence_diagram(
    std::shared_ptr<clanguml::config::diagram> c,
    clanguml::sequence_diagram::model::diagram &model)
{
    return DiagramType{common::model::diagram_t::kSequence,
        detail::render_diagram<DiagramType, clanguml::config::sequence_diagram>(
            c, model)};
}

template <typename DiagramType>
DiagramType render_package_diagram(std::shared_ptr<clanguml::config::diagram> c,
    clanguml::package_diagram::model::diagram &model)
{
    auto d = DiagramType{common::model::diagram_t::kPackage,
        detail::render_diagram<DiagramType, clanguml::config::package_diagram>(
            c, model)};
    d.generate_packages = true;
    return d;
}

template <typename DiagramType>
DiagramType render_include_diagram(std::shared_ptr<clanguml::config::diagram> c,
    clanguml::include_diagram::model::diagram &model)
{
    auto d = DiagramType{common::model::diagram_t::kInclude,
        detail::render_diagram<DiagramType, clanguml::config::include_diagram>(
            c, model)};
    d.generate_packages = c->generate_packages();
    return d;
}

auto CHECK_CLASS_MODEL(
    const std::string &test_name, const std::string &diagram_name)
{
    auto [config, db] = load_config(test_name);

    auto diagram = config->diagrams[diagram_name];

    REQUIRE(diagram->name == diagram_name);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == diagram_name);

    return std::make_tuple(
        std::move(config), std::move(db), std::move(diagram), std::move(model));
}

auto CHECK_SEQUENCE_MODEL(
    const std::string &test_name, const std::string &diagram_name)
{
    auto [config, db] = load_config(test_name);

    auto diagram = config->diagrams[diagram_name];

    REQUIRE(diagram->name == diagram_name);

    auto model = generate_sequence_diagram(*db, diagram);

    REQUIRE(model->name() == diagram_name);

    return std::make_tuple(
        std::move(config), std::move(db), std::move(diagram), std::move(model));
}

auto CHECK_PACKAGE_MODEL(
    const std::string &test_name, const std::string &diagram_name)
{
    auto [config, db] = load_config(test_name);

    auto diagram = config->diagrams[diagram_name];

    REQUIRE(diagram->name == diagram_name);

    auto model = generate_package_diagram(*db, diagram);

    REQUIRE(model->name() == diagram_name);

    return std::make_tuple(
        std::move(config), std::move(db), std::move(diagram), std::move(model));
}

auto CHECK_INCLUDE_MODEL(
    const std::string &test_name, const std::string &diagram_name)
{
    auto [config, db] = load_config(test_name);

    auto diagram = config->diagrams[diagram_name];

    REQUIRE(diagram->name == diagram_name);

    auto model = generate_include_diagram(*db, diagram);

    REQUIRE(model->name() == diagram_name);

    return std::make_tuple(
        std::move(config), std::move(db), std::move(diagram), std::move(model));
}

template <typename DSS, typename TC, typename... TCs>
void CHECK_DIAGRAM_IMPL(const DSS &diagrams, TC &&tc, TCs &&...tcs)
{
    try_run_test_case<plantuml_t>(diagrams, tc);
    try_run_test_case<mermaid_t>(diagrams, tc);
    try_run_test_case<json_t>(diagrams, tc);
    try_run_test_case<graphml_t>(diagrams, tc);

    if constexpr (sizeof...(tcs) > 0) {
        CHECK_DIAGRAM_IMPL(diagrams, std::forward<TCs>(tcs)...);
    }
}

template <typename DiagramConfig, typename DiagramModel, typename... TCs>
void CHECK_CLASS_DIAGRAM(const clanguml::config::config &config,
    DiagramConfig diagram, DiagramModel &model, TCs &&...tcs)
{
    diagram_source_storage diagram_sources{
        render_class_diagram<plantuml_t>(diagram, model),
        render_class_diagram<json_t>(diagram, model),
        render_class_diagram<mermaid_t>(diagram, model),
        render_class_diagram<graphml_t>(diagram, model)};

    CHECK_DIAGRAM_IMPL(diagram_sources, std::forward<TCs>(tcs)...);

    save_puml(config.output_directory(), diagram->name + ".puml",
        diagram_sources.template get<plantuml_t>().src);
    save_json(config.output_directory(), diagram->name + ".json",
        diagram_sources.template get<json_t>().src);
    save_mermaid(config.output_directory(), diagram->name + ".mmd",
        diagram_sources.template get<mermaid_t>().src);
    save_graphml(config.output_directory(), diagram->name + ".graphml",
        diagram_sources.template get<graphml_t>().src);
}

template <typename DiagramConfig, typename DiagramModel, typename... TCs>
void CHECK_SEQUENCE_DIAGRAM(const clanguml::config::config &config,
    DiagramConfig diagram, DiagramModel &model, TCs &&...tcs)
{
    diagram_source_storage diagram_sources{
        render_sequence_diagram<plantuml_t>(diagram, model),
        render_sequence_diagram<json_t>(diagram, model),
        render_sequence_diagram<mermaid_t>(diagram, model)};

    CHECK_DIAGRAM_IMPL(diagram_sources, std::forward<TCs>(tcs)...);

    save_puml(config.output_directory(), diagram->name + ".puml",
        diagram_sources.template get<plantuml_t>().src);
    save_json(config.output_directory(), diagram->name + ".json",
        diagram_sources.template get<json_t>().src);
    save_mermaid(config.output_directory(), diagram->name + ".mmd",
        diagram_sources.template get<mermaid_t>().src);
}

template <typename DiagramConfig, typename DiagramModel, typename... TCs>
void CHECK_PACKAGE_DIAGRAM(const clanguml::config::config &config,
    DiagramConfig diagram, DiagramModel &model, TCs &&...tcs)
{
    diagram_source_storage diagram_sources{
        render_package_diagram<plantuml_t>(diagram, model),
        render_package_diagram<json_t>(diagram, model),
        render_package_diagram<mermaid_t>(diagram, model),
        render_package_diagram<graphml_t>(diagram, model)};

    CHECK_DIAGRAM_IMPL(diagram_sources, std::forward<TCs>(tcs)...);

    save_puml(config.output_directory(), diagram->name + ".puml",
        diagram_sources.template get<plantuml_t>().src);
    save_json(config.output_directory(), diagram->name + ".json",
        diagram_sources.template get<json_t>().src);
    save_mermaid(config.output_directory(), diagram->name + ".mmd",
        diagram_sources.template get<mermaid_t>().src);
    save_graphml(config.output_directory(), diagram->name + ".graphml",
        diagram_sources.template get<graphml_t>().src);
}

template <typename DiagramConfig, typename DiagramModel, typename... TCs>
void CHECK_INCLUDE_DIAGRAM(const clanguml::config::config &config,
    DiagramConfig diagram, DiagramModel &model, TCs &&...tcs)
{
    diagram_source_storage diagram_sources{
        render_include_diagram<plantuml_t>(diagram, model),
        render_include_diagram<json_t>(diagram, model),
        render_include_diagram<mermaid_t>(diagram, model),
        render_include_diagram<graphml_t>(diagram, model)};

    CHECK_DIAGRAM_IMPL(diagram_sources, std::forward<TCs>(tcs)...);

    save_puml(config.output_directory(), diagram->name + ".puml",
        diagram_sources.template get<plantuml_t>().src);
    save_json(config.output_directory(), diagram->name + ".json",
        diagram_sources.template get<json_t>().src);
    save_mermaid(config.output_directory(), diagram->name + ".mmd",
        diagram_sources.template get<mermaid_t>().src);
    save_graphml(config.output_directory(), diagram->name + ".graphml",
        diagram_sources.template get<graphml_t>().src);
}
} // namespace clanguml::test

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
#include "t00051/test_case.h"
#include "t00052/test_case.h"
#include "t00053/test_case.h"
#include "t00054/test_case.h"
#include "t00055/test_case.h"

#if defined(ENABLE_CXX_STD_20_TEST_CASES)
#include "t00056/test_case.h"
#endif
#include "t00057/test_case.h"
#if defined(ENABLE_CXX_STD_20_TEST_CASES)
#include "t00058/test_case.h"
#include "t00059/test_case.h"
#endif
#include "t00060/test_case.h"
#include "t00061/test_case.h"
#include "t00062/test_case.h"
#include "t00063/test_case.h"
#include "t00064/test_case.h"
#if defined(ENABLE_CXX_STD_20_TEST_CASES)
#include "t00065/test_case.h"
#endif
#include "t00066/test_case.h"
#include "t00067/test_case.h"
#include "t00068/test_case.h"
#if defined(ENABLE_CXX_STD_20_TEST_CASES)
#if __has_include(<coroutine>)
#include "t00069/test_case.h"
#endif
#endif
#if defined(ENABLE_CXX_MODULES_TEST_CASES)
#include "t00070/test_case.h"
#include "t00071/test_case.h"
#include "t00072/test_case.h"
#endif
#include "t00073/test_case.h"
#if defined(ENABLE_CXX_STD_20_TEST_CASES)
#include "t00074/test_case.h"
#include "t00075/test_case.h"
#endif
#include "t00076/test_case.h"
#include "t00077/test_case.h"
#include "t00078/test_case.h"
#include "t00079/test_case.h"
#include "t00080/test_case.h"
#include "t00081/test_case.h"
#include "t00082/test_case.h"
#include "t00083/test_case.h"

#if defined(ENABLE_OBJECTIVE_C_TEST_CASES)
#include "t00084/test_case.h"
#include "t00085/test_case.h"
#include "t00086/test_case.h"
#endif

#include "t00087/test_case.h"

#if defined(ENABLE_OBJECTIVE_C_TEST_CASES)
#include "t00088/test_case.h"
#endif

#include "t00089/test_case.h"

#if defined(ENABLE_CXX_STD_20_TEST_CASES)
#include "t00090/test_case.h"
#endif

#include "t00091/test_case.h"
#include "t00092/test_case.h"

///
/// Sequence diagram tests
///
#include "t20001/test_case.h"
#include "t20002/test_case.h"
#include "t20003/test_case.h"
#include "t20004/test_case.h"
#include "t20005/test_case.h"
#include "t20006/test_case.h"
#include "t20007/test_case.h"
#include "t20008/test_case.h"
#include "t20009/test_case.h"
#include "t20010/test_case.h"
#include "t20011/test_case.h"
#include "t20012/test_case.h"
#include "t20013/test_case.h"
#include "t20014/test_case.h"
#include "t20015/test_case.h"
#include "t20016/test_case.h"
#include "t20017/test_case.h"
#include "t20018/test_case.h"
#include "t20019/test_case.h"
#include "t20020/test_case.h"
#include "t20021/test_case.h"
#include "t20022/test_case.h"
#include "t20023/test_case.h"
#include "t20024/test_case.h"
#include "t20025/test_case.h"
#include "t20026/test_case.h"
#include "t20027/test_case.h"
#include "t20028/test_case.h"
#include "t20029/test_case.h"
#include "t20030/test_case.h"
#include "t20031/test_case.h"
#include "t20032/test_case.h"
#include "t20033/test_case.h"
#include "t20034/test_case.h"
#include "t20035/test_case.h"
#include "t20036/test_case.h"
#include "t20037/test_case.h"
#include "t20038/test_case.h"
#include "t20039/test_case.h"
#include "t20040/test_case.h"
#include "t20041/test_case.h"
#include "t20042/test_case.h"
#include "t20043/test_case.h"
#include "t20044/test_case.h"
#include "t20045/test_case.h"
#include "t20046/test_case.h"
#include "t20047/test_case.h"
#include "t20048/test_case.h"

#if defined(ENABLE_CUDA_TEST_CASES)
#include "t20049/test_case.h"
#include "t20050/test_case.h"
#include "t20051/test_case.h"
#endif

#include "t20052/test_case.h"
#include "t20053/test_case.h"
#include "t20054/test_case.h"
#include "t20055/test_case.h"
#include "t20056/test_case.h"

#if defined(ENABLE_OBJECTIVE_C_TEST_CASES)
#include "t20057/test_case.h"
#include "t20058/test_case.h"
#include "t20059/test_case.h"
#endif

#include "t20060/test_case.h"
#include "t20061/test_case.h"
#include "t20062/test_case.h"
#include "t20063/test_case.h"

#if defined(ENABLE_CXX_STD_20_TEST_CASES)
#include "t20064/test_case.h"
#endif

#include "t20065/test_case.h"
#include "t20066/test_case.h"
#include "t20067/test_case.h"
#include "t20068/test_case.h"
#include "t20069/test_case.h"

#if defined(ENABLE_CXX_STD_20_TEST_CASES)
#include "t20070/test_case.h"
#include "t20071/test_case.h"
#endif

#include "t20072/test_case.h"
#include "t20073/test_case.h"

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
#include "t30009/test_case.h"
#include "t30010/test_case.h"
#include "t30011/test_case.h"
#if defined(ENABLE_CXX_MODULES_TEST_CASES)
#include "t30012/test_case.h"
#include "t30013/test_case.h"
#include "t30014/test_case.h"
#include "t30015/test_case.h"
#endif

#if defined(ENABLE_OBJECTIVE_C_TEST_CASES)
#include "t30016/test_case.h"
#endif

#include "t30017/test_case.h"
#include "t30018/test_case.h"

///
/// Include diagram tests
///
#include "t40001/test_case.h"
#include "t40002/test_case.h"
#include "t40003/test_case.h"
#if defined(ENABLE_OBJECTIVE_C_TEST_CASES)
#include "t40004/test_case.h"
#endif

///
/// Other tests (e.g. configuration file)
///
#include "t90000/test_case.h"
#include "t90001/test_case.h"

///
/// Main test function
///
int main(int argc, char *argv[])
{
    doctest::Context context;

    context.applyCommandLine(argc, argv);

    clanguml::cli::cli_handler clih;

    std::vector<const char *> argvv = {
        "clang-uml", "--config", "./test_config_data/simple.yml"};

    bool default_to_quiet{true};
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            argvv.push_back("-v");
            default_to_quiet = false;
        }
        else if (strcmp(argv[i], "-vv") == 0) {
            argvv.push_back("-vv");
            default_to_quiet = false;
        }
        else if (strcmp(argv[i], "-vvv") == 0) {
            argvv.push_back("-vvv");
            default_to_quiet = false;
        }
        else if (strcmp(argv[i], "-q") == 0) {
            argvv.push_back("-q");
            default_to_quiet = false;
        }
        else if (strcmp(argv[i], "-json") == 0) {
            argvv.push_back("--logger");
            argvv.push_back("json");
        }
    }

    if (default_to_quiet)
        argvv.push_back("-q");

    clih.handle_options(argvv.size(), argvv.data());

    int res = context.run();

    if (context.shouldExit())
        return res;

    return res;
}
