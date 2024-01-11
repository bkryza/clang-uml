/**
 * tests/t40002/test_case.cc
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

TEST_CASE("t40002", "[test-case][include]")
{
    auto [config, db] = load_config("t40002");

    auto diagram = config.diagrams["t40002_include"];

    REQUIRE(diagram->name == "t40002_include");

    auto model = generate_include_diagram(*db, diagram);

    REQUIRE(model->name() == "t40002_include");

    {
        auto src = generate_include_puml(diagram, *model);

        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        REQUIRE_THAT(src, IsFolder("lib1"));
        REQUIRE_THAT(src, IsFolder("lib2"));
        REQUIRE_THAT(src, IsFile("lib1.h"));
        REQUIRE_THAT(src, IsFile("lib2.h"));
        REQUIRE_THAT(src, !IsFile("lib2_detail.h"));
        REQUIRE_THAT(src, IsFile("t40002.cc"));
        REQUIRE_THAT(src, IsFile("lib1.cc"));
        REQUIRE_THAT(src, IsFile("lib2.cc"));

        REQUIRE_THAT(src, !IsFile("string"));

        REQUIRE_THAT(src, IsAssociation(_A("t40002.cc"), _A("lib1.h")));
        REQUIRE_THAT(src, IsAssociation(_A("lib1.h"), _A("lib2.h")));
        REQUIRE_THAT(src, IsAssociation(_A("lib1.cc"), _A("lib1.h")));
        REQUIRE_THAT(src, IsAssociation(_A("lib2.cc"), _A("lib2.h")));

        REQUIRE_THAT(src,
            HasLink(_A("t40002.cc"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t40002/src/t40002.cc#L0",
                    clanguml::util::get_git_commit()),
                "t40002.cc"));

        REQUIRE_THAT(src,
            HasLink(_A("lib1.cc"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t40002/src/lib1/lib1.cc#L0",
                    clanguml::util::get_git_commit()),
                "lib1.cc"));

        REQUIRE_THAT(src,
            HasLink(_A("lib1.h"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t40002/include/lib1/lib1.h#L0",
                    clanguml::util::get_git_commit()),
                "lib1.h"));

        REQUIRE_THAT(src,
            HasLink(_A("lib2.h"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t40002/include/lib2/lib2.h#L0",
                    clanguml::util::get_git_commit()),
                "lib2.h"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_include_json(diagram, *model);

        using namespace json;

        REQUIRE(IsFolder(j, "include"));
        REQUIRE(IsFolder(j, "include/lib1"));
        REQUIRE(IsFolder(j, "include/lib2"));
        REQUIRE(IsFolder(j, "src"));
        REQUIRE(IsFolder(j, "src/lib1"));
        REQUIRE(IsFolder(j, "src/lib2"));
        REQUIRE(IsFile(j, "include/lib1/lib1.h"));
        REQUIRE(IsFile(j, "include/lib2/lib2.h"));
        REQUIRE(!IsFile(j, "include/lib2/lib2_detail.h"));
        REQUIRE(IsFile(j, "src/lib1/lib1.cc"));
        REQUIRE(IsFile(j, "src/lib2/lib2.cc"));
        REQUIRE(IsFile(j, "src/t40002.cc"));

        REQUIRE(!IsFile(j, "string"));

        REQUIRE(IsAssociation(j, "src/t40002.cc", "include/lib1/lib1.h"));
        REQUIRE(IsAssociation(j, "include/lib1/lib1.h", "include/lib2/lib2.h"));
        REQUIRE(IsAssociation(j, "src/lib1/lib1.cc", "include/lib1/lib1.h"));
        REQUIRE(IsAssociation(j, "src/lib2/lib2.cc", "include/lib2/lib2.h"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_include_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::HasLink;
        using mermaid::IsFile;
        using mermaid::IsFolder;

        REQUIRE_THAT(src, IsFolder(_A("lib1")));
        REQUIRE_THAT(src, IsFolder(_A("lib2")));
        REQUIRE_THAT(src, IsFile(_A("lib1.h")));
        REQUIRE_THAT(src, IsFile(_A("lib2.h")));
        REQUIRE_THAT(src, !IsFile(_A("lib2_detail.h")));
        REQUIRE_THAT(src, IsFile(_A("t40002.cc")));
        REQUIRE_THAT(src, IsFile(_A("lib1.cc")));
        REQUIRE_THAT(src, IsFile(_A("lib2.cc")));

        REQUIRE_THAT(src, !IsFile(_A("string")));

        REQUIRE_THAT(src, IsAssociation(_A("t40002.cc"), _A("lib1.h")));
        REQUIRE_THAT(src, IsAssociation(_A("lib1.h"), _A("lib2.h")));
        REQUIRE_THAT(src, IsAssociation(_A("lib1.cc"), _A("lib1.h")));
        REQUIRE_THAT(src, IsAssociation(_A("lib2.cc"), _A("lib2.h")));

        REQUIRE_THAT(src,
            HasLink(_A("t40002.cc"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t40002/src/t40002.cc#L0",
                    clanguml::util::get_git_commit()),
                "t40002.cc"));

        REQUIRE_THAT(src,
            HasLink(_A("lib1.cc"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t40002/src/lib1/lib1.cc#L0",
                    clanguml::util::get_git_commit()),
                "lib1.cc"));

        REQUIRE_THAT(src,
            HasLink(_A("lib1.h"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t40002/include/lib1/lib1.h#L0",
                    clanguml::util::get_git_commit()),
                "lib1.h"));

        REQUIRE_THAT(src,
            HasLink(_A("lib2.h"),
                fmt::format("https://github.com/bkryza/clang-uml/blob/{}/tests/"
                            "t40002/include/lib2/lib2.h#L0",
                    clanguml::util::get_git_commit()),
                "lib2.h"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
