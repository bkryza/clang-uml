/**
 * tests/t00038/test_case.cc
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

TEST_CASE("t00038", "[test-case][class]")
{
    auto [config, db] = load_config("t00038");

    auto diagram = config.diagrams["t00038_class"];

    REQUIRE(diagram->name == "t00038_class");
    REQUIRE(diagram->generate_packages() == false);

    auto model = generate_class_diagram(*db, diagram);

    REQUIRE(model->name() == "t00038_class");

    {
        auto src = generate_class_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("thirdparty::ns1::E")));
        REQUIRE_THAT(src, IsClass(_A("key_t")));
        REQUIRE_THAT(src, IsClassTemplate("map", "T"));
        REQUIRE_THAT(src,
            IsClassTemplate("map",
                "std::integral_constant<property_t,property_t::property_a>"));
        REQUIRE_THAT(src,
            IsClassTemplate("map",
                "std::vector<std::integral_constant<property_t,property_t::"
                "property_b>>"));
        REQUIRE_THAT(src,
            IsClassTemplate("map",
                "std::map<key_t,std::vector<std::integral_constant<property_t,"
                "property_t::property_c>>>"));

        REQUIRE_THAT(src, IsEnum(_A("property_t")));

        REQUIRE_THAT(src,
            IsInstantiation(_A("map<T>"),
                _A("map<std::map<key_t,std::vector<std::integral_constant<"
                   "property_"
                   "t,property_t::property_c>>>>")));

        REQUIRE_THAT(src,
            IsDependency(_A("map<std::integral_constant<property_t,property_t::"
                            "property_a>>"),
                _A("property_t")));

        REQUIRE_THAT(src,
            IsDependency(_A("map<"
                            "std::vector<std::integral_constant<property_t,"
                            "property_t::property_b>>>"),
                _A("property_t")));

        REQUIRE_THAT(src,
            IsDependency(
                _A("map<std::map<key_t,std::vector<std::integral_constant<"
                   "property_t,property_t::property_c>>>>"),
                _A("property_t")));

        REQUIRE_THAT(src,
            IsDependency(
                _A("map<std::map<key_t,std::vector<std::integral_constant<"
                   "property_t,property_t::property_c>>>>"),
                _A("key_t")));

        REQUIRE_THAT(src,
            IsDependency(
                _A("map<std::integral_constant<thirdparty::ns1::color_t,"
                   "thirdparty::ns1::color_t::red>>"),
                _A("thirdparty::ns1::color_t")));

        REQUIRE_THAT(src,
            IsBaseClass(_A("thirdparty::ns1::E"),
                _A("map<std::integral_constant<thirdparty::ns1::color_t,"
                   "thirdparty::ns1::color_t::red>>")));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }
    {
        auto j = generate_class_json(diagram, *model);

        using namespace json;

        REQUIRE(IsClass(j, "A"));
        REQUIRE(IsClass(j, "B"));
        REQUIRE(IsClass(j, "C"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }
    {
        auto src = generate_class_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::IsEnum;

        REQUIRE_THAT(src, IsClass(_A("A")));
        REQUIRE_THAT(src, IsClass(_A("B")));
        REQUIRE_THAT(src, IsClass(_A("C")));
        REQUIRE_THAT(src, IsClass(_A("thirdparty::ns1::E")));
        REQUIRE_THAT(src, IsClass(_A("key_t")));
        REQUIRE_THAT(src, IsClass(_A("map<T>")));
        REQUIRE_THAT(src,
            IsClass(_A("map<std::integral_constant<property_t,property_t::"
                       "property_a>>")));
        REQUIRE_THAT(src,
            IsClass(_A(
                "map<std::vector<std::integral_constant<property_t,property_t::"
                "property_b>>>")));
        REQUIRE_THAT(src,
            IsClass(_A("map<std::map<key_t,std::vector<std::integral_constant<"
                       "property_t,"
                       "property_t::property_c>>>>")));

        REQUIRE_THAT(src, IsEnum(_A("property_t")));

        REQUIRE_THAT(src,
            IsInstantiation(_A("map<T>"),
                _A("map<std::map<key_t,std::vector<std::integral_constant<"
                   "property_"
                   "t,property_t::property_c>>>>")));

        REQUIRE_THAT(src,
            IsDependency(_A("map<std::integral_constant<property_t,property_t::"
                            "property_a>>"),
                _A("property_t")));

        REQUIRE_THAT(src,
            IsDependency(_A("map<"
                            "std::vector<std::integral_constant<property_t,"
                            "property_t::property_b>>>"),
                _A("property_t")));

        REQUIRE_THAT(src,
            IsDependency(
                _A("map<std::map<key_t,std::vector<std::integral_constant<"
                   "property_t,property_t::property_c>>>>"),
                _A("property_t")));

        REQUIRE_THAT(src,
            IsDependency(
                _A("map<std::map<key_t,std::vector<std::integral_constant<"
                   "property_t,property_t::property_c>>>>"),
                _A("key_t")));

        REQUIRE_THAT(src,
            IsDependency(
                _A("map<std::integral_constant<thirdparty::ns1::color_t,"
                   "thirdparty::ns1::color_t::red>>"),
                _A("thirdparty::ns1::color_t")));

        REQUIRE_THAT(src,
            IsBaseClass(_A("thirdparty::ns1::E"),
                _A("map<std::integral_constant<thirdparty::ns1::color_t,"
                   "thirdparty::ns1::color_t::red>>")));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}