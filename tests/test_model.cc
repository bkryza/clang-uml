/**
 * @file tests/test_model.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"

#include "class_diagram/model/class.h"
#include "common/model/namespace.h"
#include "common/model/package.h"
#include "common/model/path.h"
#include "common/model/template_parameter.h"

TEST_CASE("Test namespace_")
{
    using clanguml::common::model::namespace_;

    namespace_ ns1{};
    CHECK(ns1.is_empty());

    namespace_ ns2{"aaa", "bbb", "ccc"};
    CHECK(ns2.size() == 3);
    CHECK(ns2[0] == "aaa");
    CHECK(ns2[1] == "bbb");
    CHECK(ns2[2] == "ccc");

    namespace_ ns3 = ns1 | "aaa" | "bbb" | "ccc";
    CHECK(ns3.size() == 3);
    CHECK(ns3[0] == "aaa");
    CHECK(ns3[1] == "bbb");
    CHECK(ns3[2] == "ccc");

    namespace_ ns4 = namespace_{"aaa", "bbb"} | namespace_{"ccc"};
    CHECK(ns4.size() == 3);
    CHECK(ns4[0] == "aaa");
    CHECK(ns4[1] == "bbb");
    CHECK(ns4[2] == "ccc");

    namespace_ ns5{"aaa::bbb::ccc"};
    CHECK(ns5.size() == 3);
    CHECK(ns5[0] == "aaa");
    CHECK(ns5[1] == "bbb");
    CHECK(ns5[2] == "ccc");

    CHECK(ns4 == ns5);

    namespace_ ns6a{"aaa::bbb::ccc"};
    namespace_ ns6b{"aaa::bbb::ddd::eee"};
    auto ns6 = ns6a.common_path(ns6b);

    CHECK(ns6 == namespace_{"aaa", "bbb"});
    CHECK(ns6b.starts_with({"aaa", "bbb", "ddd"}));

    namespace_ ns7a{"aaa::bbb"};
    namespace_ ns7b{"aaa::bbb::ccc::ddd"};
    CHECK(ns7b.relative_to(ns7a) == namespace_{"ccc", "ddd"});
    CHECK(ns7a.relative_to(ns7b) == namespace_{"aaa::bbb"});

    namespace_ ns8{"aaa::bbb"};
    const std::string name{"aaa::bbb::ccc<std::unique_ptr<aaa::bbb::ddd>>"};
    CHECK(ns8.relative(name) == "ccc<std::unique_ptr<ddd>>");
}

TEST_CASE("Test class_::calculate_specialization_match")
{
    using clanguml::class_diagram::model::class_;
    using clanguml::common::model::template_parameter;

    {
        auto c = class_({});
        c.set_name("A");
        c.add_template(template_parameter::make_argument("int"));
        c.add_template(template_parameter::make_argument("double"));
        c.add_template(template_parameter::make_argument("int"));

        auto t = class_({});
        t.set_name("A");
        t.add_template(
            template_parameter::make_template_type("Args", {}, true));
        t.add_template(template_parameter::make_argument("int"));

        CHECK(c.calculate_template_specialization_match(t));
    }

    {
        auto c = class_({});
        c.set_name("A");
        c.add_template(template_parameter::make_argument("double"));
        c.add_template(template_parameter::make_argument("int"));

        auto s = class_({});
        s.set_name("A");
        s.add_template(template_parameter::make_argument("double"));
        s.add_template(template_parameter::make_template_type("V"));

        auto t = class_({});
        t.set_name("A");
        t.add_template(template_parameter::make_template_type("T"));
        t.add_template(template_parameter::make_template_type("V"));

        CHECK(c.calculate_template_specialization_match(s) >
            c.calculate_template_specialization_match(t));
    }
}

TEST_CASE("Test template_parameter::calculate_specialization_match")
{
    using clanguml::common::model::template_parameter;

    {
        auto tp1 = template_parameter::make_template_type("T");
        auto tp2 = template_parameter::make_argument("int");

        CHECK(tp2.calculate_specialization_match(tp1) > 0);
    }

    {
        auto tp1 = template_parameter::make_template_type("T");
        auto tp2 = template_parameter::make_argument("vector");
        tp2.add_template_param(template_parameter::make_argument("int"));

        CHECK(tp2.calculate_specialization_match(tp1));
    }

    {
        auto tp1 = template_parameter::make_argument("vector");
        tp1.add_template_param(template_parameter::make_template_type("T"));

        auto tp2 = template_parameter::make_argument("vector");
        tp2.add_template_param(template_parameter::make_argument("int"));

        CHECK(tp2.calculate_specialization_match(tp1));
    }

    {
        auto tp1 = template_parameter::make_argument("vector");
        tp1.add_template_param(template_parameter::make_template_type("T"));

        auto tp2 = template_parameter::make_argument("string");
        tp2.add_template_param(template_parameter::make_argument("char"));

        CHECK(!tp2.calculate_specialization_match(tp1));
    }

    {
        auto tp1 = template_parameter::make_argument("tuple");
        tp1.add_template_param(
            template_parameter::make_template_type("Args", {}, true));

        auto tp2 = template_parameter::make_argument("tuple");
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_argument("int"));
        tp2.add_template_param(template_parameter::make_argument("double"));

        CHECK(tp2.calculate_specialization_match(tp1));
    }

    {
        auto tp1 = template_parameter::make_argument("tuple");
        tp1.add_template_param(
            template_parameter::make_template_type("Args", {}, true));
        tp1.add_template_param(template_parameter::make_argument("int"));

        auto tp2 = template_parameter::make_argument("tuple");
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_argument("double"));
        tp2.add_template_param(template_parameter::make_argument("int"));

        CHECK(tp2.calculate_specialization_match(tp1));
    }

    {
        auto tp1 = template_parameter::make_argument("tuple");
        tp1.add_template_param(
            template_parameter::make_template_type("Args", {}, true));
        tp1.add_template_param(template_parameter::make_argument("int"));

        auto tp2 = template_parameter::make_argument("tuple");
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_argument("int"));
        tp2.add_template_param(template_parameter::make_argument("double"));

        CHECK(tp2.calculate_specialization_match(tp1) == 0);
    }

    {
        auto tp1 = template_parameter::make_argument("tuple");
        tp1.add_template_param(template_parameter::make_template_type("T1"));
        tp1.add_template_param(template_parameter::make_template_type("T2"));

        auto tp2 = template_parameter::make_argument("tuple");
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_argument("int"));
        tp2.add_template_param(template_parameter::make_argument("double"));

        CHECK(tp2.calculate_specialization_match(tp1) == 0);
    }

    {
        auto tp1 = template_parameter::make_template_type({});
        tp1.is_function_template(true);
        tp1.add_template_param(template_parameter::make_template_type("Ret"));
        tp1.add_template_param(template_parameter::make_template_type("Arg1"));
        tp1.add_template_param(template_parameter::make_template_type("Arg2"));

        auto tp2 = template_parameter::make_argument({});
        tp2.is_function_template(true);
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_argument("int"));
        tp2.add_template_param(template_parameter::make_argument("double"));

        CHECK(tp2.calculate_specialization_match(tp1));
    }

    {
        auto tp1 = template_parameter::make_template_type({});
        tp1.is_function_template(true);
        tp1.add_template_param(template_parameter::make_template_type("Ret"));
        tp1.add_template_param(template_parameter::make_template_type("Arg1"));
        tp1.add_template_param(template_parameter::make_template_type("Arg2"));

        auto tp2 = template_parameter::make_argument({});
        tp2.is_function_template(false);
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_argument("int"));
        tp2.add_template_param(template_parameter::make_argument("double"));

        CHECK(!tp2.calculate_specialization_match(tp1));
    }

    {
        auto tp1 = template_parameter::make_template_type({});
        tp1.is_function_template(true);
        tp1.add_template_param(template_parameter::make_template_type("Ret"));
        tp1.add_template_param(
            template_parameter::make_template_type("Args", {}, true));

        auto tp2 = template_parameter::make_argument({});
        tp2.is_function_template(true);
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_argument("int"));
        tp2.add_template_param(template_parameter::make_argument("double"));

        CHECK(tp2.calculate_specialization_match(tp1));
    }

    {
        auto sink_t = template_parameter::make_argument("sink");
        auto sh1 =
            template_parameter::make_argument("ns1::ns2::signal_handler");
        auto sh1_t1 = template_parameter::make_template_type({});
        sh1_t1.is_function_template(true);
        sh1_t1.add_template_param(
            template_parameter::make_template_type("Ret"));
        sh1_t1.add_template_param(
            template_parameter::make_template_type("Args", {}, true));
        auto sh1_t2 = template_parameter::make_template_type("A");
        sh1.add_template_param(sh1_t1);
        sh1.add_template_param(sh1_t2);
        sink_t.add_template_param(sh1);

        auto sink_s = template_parameter::make_argument("sink");
        auto sh2 =
            template_parameter::make_argument("ns1::ns2::signal_handler");
        auto sh2_a1 = template_parameter::make_argument({});
        sh2_a1.is_function_template(true);
        sh2_a1.add_template_param(template_parameter::make_argument("void"));
        sh2_a1.add_template_param(template_parameter::make_argument("int"));
        auto sh2_a2 = template_parameter::make_argument("bool");
        sh2.add_template_param(sh2_a1);
        sh2.add_template_param(sh2_a2);
        sink_s.add_template_param(sh2);

        CHECK(sink_s.calculate_specialization_match(sink_t));
    }

    {
        auto tp1 = template_parameter::make_template_type({});
        tp1.is_function_template(true);
        tp1.add_template_param(template_parameter::make_template_type("Ret"));
        tp1.add_template_param(template_parameter::make_template_type("C"));
        tp1.add_template_param(template_parameter::make_template_type("Arg0"));

        auto tp2 = template_parameter::make_template_type({});
        tp2.is_function_template(true);
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_template_type("C"));
        tp2.add_template_param(template_parameter::make_argument("double"));

        CHECK(tp2.calculate_specialization_match(tp1));
    }

    {
        auto tp1 = template_parameter::make_template_type({});
        tp1.is_function_template(true);
        tp1.add_template_param(template_parameter::make_template_type("Ret"));
        tp1.add_template_param(template_parameter::make_template_type("C"));
        tp1.add_template_param(template_parameter::make_template_type("Arg0"));

        auto tp2 = template_parameter::make_template_type({});
        tp2.is_function_template(true);
        tp2.add_template_param(template_parameter::make_argument("char"));
        tp2.add_template_param(template_parameter::make_template_type("C"));
        tp2.add_template_param(template_parameter::make_argument("double"));

        CHECK(tp2.calculate_specialization_match(tp1));
    }

    {
        using clanguml::common::model::context;
        using clanguml::common::model::rpqualifier;

        auto tp1 = template_parameter::make_template_type({});
        tp1.is_data_pointer(true);
        tp1.add_template_param(template_parameter::make_template_type("T"));
        tp1.add_template_param(template_parameter::make_template_type("C"));

        CHECK(tp1.is_specialization());

        auto tp2 = template_parameter::make_template_type({});
        tp2.is_data_pointer(true);
        tp2.add_template_param(template_parameter::make_argument("T"));
        tp2.add_template_param(template_parameter::make_template_type("C"));
        tp2.push_context(context{.pr = rpqualifier::kRValueReference});

        CHECK(tp2.is_specialization());

        CHECK(tp2.calculate_specialization_match(tp1) == 0);
        CHECK(tp1.calculate_specialization_match(tp2) == 0);
    }

    {
        using clanguml::common::model::context;
        using clanguml::common::model::rpqualifier;

        auto tp1 = template_parameter::make_template_type("T");
        tp1.push_context(context{.pr = rpqualifier::kRValueReference});
        CHECK(tp1.is_specialization());

        auto tp2 = template_parameter::make_template_type({});
        tp2.is_data_pointer(true);
        tp2.add_template_param(template_parameter::make_template_type("T"));
        tp2.add_template_param(template_parameter::make_template_type("C"));
        tp2.push_context(context{.pr = rpqualifier::kRValueReference});

        CHECK(tp2.is_specialization());

        CHECK(tp2.calculate_specialization_match(tp1) > 1);
        CHECK(tp1.calculate_specialization_match(tp2) == 0);
    }

    {
        using clanguml::common::model::context;
        using clanguml::common::model::rpqualifier;

        auto tp1 = template_parameter::make_template_type({});
        tp1.is_array(true);
        tp1.add_template_param(template_parameter::make_template_type("int"));
        tp1.add_template_param(template_parameter::make_template_type("N"));

        CHECK(tp1.to_string({}, false) == "int[N]");
        CHECK(tp1.is_specialization());

        auto tp2 = template_parameter::make_argument({});
        tp2.is_array(true);
        tp2.add_template_param(template_parameter::make_argument("int"));
        tp2.add_template_param(template_parameter::make_argument("1000"));

        CHECK(tp2.to_string({}, false) == "int[1000]");
        CHECK(tp2.is_specialization());

        CHECK(tp2.calculate_specialization_match(tp1) > 1);
        CHECK(tp1.calculate_specialization_match(tp2) == 0);
    }

    {
        using clanguml::common::model::context;
        using clanguml::common::model::rpqualifier;

        auto tp1 = template_parameter::make_template_type("T");
        tp1.push_context(context{.pr = rpqualifier::kLValueReference});
        CHECK(tp1.is_specialization());
        CHECK(tp1.to_string({}, false) == "T &");

        auto tp2 = template_parameter::make_template_type({});
        tp2.is_array(true);
        tp2.add_template_param(template_parameter::make_template_type("int"));
        tp2.add_template_param(template_parameter::make_template_type("N"));

        CHECK(tp2.is_specialization());
        CHECK(tp2.to_string({}, false) == "int[N]");

        CHECK(tp2.calculate_specialization_match(tp1) == 0);
        CHECK(tp1.calculate_specialization_match(tp2) == 0);
    }
}

TEST_CASE("Test common::model::package full_name")
{
    using clanguml::common::model::package;
    using clanguml::common::model::path;
    using clanguml::common::model::path_type;

    {
        auto using_namespace = path{"A::B::C"};
        auto pkg = package(using_namespace);
        pkg.set_name("G");
        pkg.set_namespace(path{"A::B::C::D::E::F"});

        CHECK(pkg.full_name(false) == "A::B::C::D::E::F::G");
        CHECK(pkg.full_name(true) == "D::E::F::G");

        CHECK(pkg.doxygen_link().value() ==
            "namespaceA_1_1B_1_1C_1_1D_1_1E_1_1F_1_1G.html");
    }

#if !defined(_MSC_VER)
    {
        auto using_namespace = path{"/A/B/C", path_type::kFilesystem};
        auto pkg = package(using_namespace, path_type::kFilesystem);
        pkg.set_name("G");
        pkg.set_namespace(path{"/A/B/C/D/E/F", path_type::kFilesystem});

        CHECK(pkg.full_name(false) == "A/B/C/D/E/F/G");
        CHECK(pkg.full_name(true) == "D/E/F/G");
    }
#else
    {
        auto using_namespace = path{"A\\B\\C", path_type::kFilesystem};
        auto pkg = package(using_namespace, path_type::kFilesystem);
        pkg.set_name("G");
        pkg.set_namespace(path{"A\\B\\C\\D\\E\\F", path_type::kFilesystem});

        CHECK(pkg.full_name(false) == "A\\B\\C\\D\\E\\F\\G");
        CHECK(pkg.full_name(true) == "D\\E\\F\\G");
    }
#endif

    {
        auto using_namespace = path{"A.B.C", path_type::kModule};
        auto pkg = package(using_namespace, path_type::kModule);
        pkg.set_name("G");
        pkg.set_namespace(path{"A.B.C.D:E.F", path_type::kModule});

        CHECK(pkg.full_name(false) == "A.B.C.D:E.F.G");
        CHECK(pkg.full_name(true) == "D:E.F.G");
    }

    {
        auto using_namespace = path{"A.B.C", path_type::kModule};
        auto pkg = package(using_namespace, path_type::kModule);
        pkg.set_name(":D");
        pkg.set_namespace(path{"A.B.C", path_type::kModule});

        CHECK(pkg.full_name(false) == "A.B.C:D");
        CHECK(pkg.full_name(true) == ":D");
    }
}

TEST_CASE("Test path_type")
{
    using namespace clanguml::common::model;

    REQUIRE_EQ(to_string(path_type::kModule), "module");
    REQUIRE_EQ(to_string(path_type::kFilesystem), "directory");
    REQUIRE_EQ(to_string(path_type::kNamespace), "namespace");

    // Check that assiging a namespace path to a filesystem path throws
    auto p1 = path{"A::B::C", path_type::kNamespace};
    auto p2 = path{"A/B/C/D", path_type::kFilesystem};

    REQUIRE_THROWS_AS(p1 = p2, std::runtime_error);
}