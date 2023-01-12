/**
 * tests/test_model.cc
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

#include "common/model/namespace.h"


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