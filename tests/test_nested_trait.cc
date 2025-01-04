/**
 * @file tests/test_model.cc
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
#define DOCTEST_CONFIG_IMPLEMENT

#include "doctest/doctest.h"

#include "class_diagram/model/class.h"
#include "cli/cli_handler.h"
#include "common/model/diagram.h"
#include "common/model/element_view.h"
#include "common/model/namespace.h"
#include "common/model/package.h"
#include "common/model/path.h"
#include "common/model/template_parameter.h"
#include "config/config.h"

using clanguml::class_diagram::model::class_;
using clanguml::common::model::diagram_element;
using clanguml::common::model::diagram_t;
using clanguml::common::model::eid_t;
using clanguml::common::model::element_view;
using clanguml::common::model::element_views;
using clanguml::common::model::namespace_;
using clanguml::common::model::package;
using clanguml::common::model::path;
using clanguml::common::model::path_type;

namespace clanguml::test {

using nested_trait_ns =
    clanguml::common::model::nested_trait<clanguml::common::model::element,
        clanguml::common::model::namespace_>;

class diagram_model_mock : public element_views<class_>,
                           public nested_trait_ns {
public:
};

auto id()
{
    static uint64_t id_counter{1UL};

    return eid_t{id_counter++};
}

auto root_prefix(const common::model::element *e)
{
    return common::model::needs_root_prefix(*e);
}

void generate(nested_trait_ns &d, std::stringstream &out)
{
    for (const auto &p : d) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            if (!pkg->is_empty()) {
                generate(*pkg, out);
            }
        }
        else {
            out << p->type_name() << " ";
            if (root_prefix(p.get()))
                out << "::";
            out << p->full_name(true) << '\n';
        }
    }
}

void generate_with_packages(nested_trait_ns &d, std::stringstream &out)
{
    for (const auto &p : d) {
        if (auto *pkg = dynamic_cast<package *>(p.get()); pkg) {
            if (!pkg->is_empty()) {
                out << "package ";
                if (root_prefix(p.get()))
                    out << "::";
                out << p->name() << "\n";
                generate_with_packages(*pkg, out);
                out << "end package\n";
            }
        }
        else {
            out << p->type_name() << " ";
            if (p->get_namespace().is_empty() && root_prefix(p.get()))
                out << "::";
            out << p->name() << '\n';
        }
    }
}
} // namespace clanguml::test.e

TEST_CASE("Test nested trait with empty using namespace")
{
    using clanguml::test::diagram_model_mock;
    using clanguml::test::id;

    diagram_model_mock d;

    namespace_ using_namespace{};

    auto p = std::make_unique<package>(using_namespace);
    p->set_name("ns1");
    p->set_id(id());
    auto prel = p->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(p)));

    p = std::make_unique<package>(using_namespace);
    p->set_name("ns2");
    p->set_namespace(namespace_{"ns1"});
    p->set_id(id());
    prel = p->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(p)));

    auto c = std::make_unique<class_>(using_namespace);
    c->set_name("A");
    c->set_id(id());
    prel = c->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(c)));

    c = std::make_unique<class_>(using_namespace);
    c->set_name("B");
    c->set_namespace(namespace_{"ns1"});
    c->set_id(id());
    prel = c->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(c)));

    c = std::make_unique<class_>(using_namespace);
    c->set_name("C");
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_id(id());
    prel = c->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(c)));

    REQUIRE_EQ(d.get_element(namespace_{"A"}).value().name(), "A");
    REQUIRE_EQ(d.get_element(namespace_{"ns1::B"}).value().name(), "B");
    REQUIRE_EQ(d.get_element(namespace_{"ns1::ns2::C"}).value().name(), "C");

    {
        std::stringstream out;

        generate(d, out);

        auto d_str = out.str();

        REQUIRE_EQ(d_str, R"(class ns1::ns2::C
class ns1::B
class A
)");
    }

    {
        std::stringstream out;

        generate_with_packages(d, out);

        REQUIRE_EQ(out.str(), R"(package ns1
package ns2
class C
end package
class B
end package
class A
)");
    }
}

TEST_CASE("Test nested trait with using namespace")
{
    using clanguml::test::diagram_model_mock;
    using clanguml::test::id;
    using namespace std::string_literals;

    diagram_model_mock d;

    namespace_ using_namespace{"ns1::ns2"};

    auto p = std::make_unique<package>(using_namespace);
    p->set_name("ns1");
    p->set_id(id());
    p->is_root(true);
    auto prel = p->path().relative_to(using_namespace);

    REQUIRE(d.add_element(prel, std::move(p)));

    p = std::make_unique<package>(using_namespace);
    p->set_name("ns2");
    p->set_namespace(namespace_{"ns1"});
    p->set_id(id());
    p->is_root(false);
    prel = p->path().relative_to(using_namespace);

    REQUIRE(d.add_element(prel, std::move(p)));

    auto c = std::make_unique<class_>(using_namespace);
    c->set_name("A");
    c->set_id(id());
    const auto A_id = c->id();
    prel = c->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(c)));

    c = std::make_unique<class_>(using_namespace);
    c->set_name("B");
    c->set_namespace(namespace_{"ns1"});
    c->set_id(id());
    const auto ns1_A_id = c->id();
    prel = c->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(c)));

    c = std::make_unique<class_>(using_namespace);
    c->set_name("C");
    c->set_namespace(namespace_{"ns1::ns2"});
    c->set_id(id());
    const auto ns1_ns2_A_id = c->id();
    prel = c->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(c)));

    auto A_ns = namespace_{"A"};
    A_ns.is_root(true);
    REQUIRE_EQ(d.get_element(A_ns).value().id(), A_id);

    REQUIRE_EQ(d.get_element(namespace_{"ns1::B"}.relative_to(using_namespace))
                   .value()
                   .id(),
        ns1_A_id);
    REQUIRE_EQ(
        d.get_element(namespace_{"ns1::ns2::C"}.relative_to(using_namespace))
            .value()
            .id(),
        ns1_ns2_A_id);

    {
        std::stringstream out;

        generate(d, out);

        REQUIRE_EQ(out.str(), R"(class ::ns1::B
class ::A
class C
)");
    }
    {
        std::stringstream out;

        generate_with_packages(d, out);

        CHECK_EQ(out.str(), R"(package ::ns1
class B
end package
class ::A
class C
)");
    }
}

TEST_CASE("Test nested trait with using namespace conflicting nested")
{
    using clanguml::test::diagram_model_mock;
    using clanguml::test::id;
    using namespace std::string_literals;

    diagram_model_mock d;

    namespace_ using_namespace{"ns1"};

    auto p = std::make_unique<package>(using_namespace);
    p->set_name("ns1");
    p->set_id(id());
    p->is_root(true);
    auto prel = p->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(p)));

    p = std::make_unique<package>(using_namespace);
    p->set_name("ns2");
    p->set_namespace(namespace_{"ns1"});
    p->set_id(id());
    prel = p->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(p)));

    p = std::make_unique<package>(using_namespace);
    p->set_name("ns2");
    p->set_id(id());
    p->is_root(true);
    prel = p->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(p)));

    auto c = std::make_unique<class_>(using_namespace);
    c->set_name("A");
    c->set_id(id());
    c->set_namespace(namespace_{"ns1::ns2"});
    const auto A_id = c->id();
    prel = c->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(c)));

    c = std::make_unique<class_>(using_namespace);
    c->set_name("B");
    c->set_namespace(namespace_{"ns2"});
    c->set_id(id());
    const auto ns2_B_id = c->id();
    prel = c->path().relative_to(using_namespace);
    REQUIRE(d.add_element(prel, std::move(c)));

    {
        std::stringstream out;

        generate(d, out);

        REQUIRE_EQ(out.str(), R"(class ns2::A
class ::ns2::B
)");
    }
    {
        std::stringstream out;

        generate_with_packages(d, out);

        REQUIRE_EQ(out.str(), R"(package ns2
class A
end package
package ::ns2
class B
end package
)");
    }
}
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

    argvv.push_back("-vv");

    clih.handle_options(argvv.size(), argvv.data());

    int res = context.run();

    if (context.shouldExit())
        return res;

    return res;
}
