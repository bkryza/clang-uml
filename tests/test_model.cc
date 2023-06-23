/**
 * @file tests/test_model.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
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
#define CATCH_CONFIG_MAIN

#include "catch.h"

#include "class_diagram/model/class.h"
#include "common/model/namespace.h"
#include "common/model/template_parameter.h"

TEST_CASE("Test namespace_", "[unit-test]")
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

TEST_CASE("Test class_::calculate_specialization_match", "[unit-test]")
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

TEST_CASE(
    "Test template_parameter::calculate_specialization_match", "[unit-test]")
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
