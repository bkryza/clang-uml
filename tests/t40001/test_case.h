/**
 * tests/t40001/test_case.cc
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

TEST_CASE("t40001", "[test-case][include]")
{
    auto [config, db] = load_config("t40001");

    auto diagram = config.diagrams["t40001_include"];

    REQUIRE(diagram->name == "t40001_include");

    auto model = generate_include_diagram(*db, diagram);

    REQUIRE(model->name() == "t40001_include");

    {
        auto src = generate_include_puml(diagram, *model);

        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));
        REQUIRE_THAT(src, HasTitle("Basic include diagram example"));

        REQUIRE_THAT(src, IsFolder("lib1"));
        REQUIRE_THAT(src, IsFile("lib1.h"));
        REQUIRE_THAT(src, IsFile("t40001.cc"));
        REQUIRE_THAT(src, IsFile("t40001_include1.h"));

        REQUIRE_THAT(src, IsFile("string"));
        REQUIRE_THAT(src, IsFile("yaml-cpp/yaml.h"));

        REQUIRE_THAT(
            src, IsAssociation(_A("t40001.cc"), _A("t40001_include1.h")));
        REQUIRE_THAT(src, IsAssociation(_A("t40001_include1.h"), _A("lib1.h")));

        REQUIRE_THAT(src, IsDependency(_A("t40001_include1.h"), _A("string")));

        REQUIRE_THAT(src, HasComment("t40001 test diagram of type include"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_include_json(diagram, *model);

        using namespace json;

        REQUIRE(HasTitle(j, "Basic include diagram example"));

        REQUIRE(IsFolder(j, "include"));
        REQUIRE(IsFolder(j, "include/lib1"));
        REQUIRE(IsFolder(j, "src"));

        REQUIRE(IsFile(j, "include/lib1/lib1.h"));
        REQUIRE(IsFile(j, "include/t40001_include1.h"));
        REQUIRE(IsFile(j, "src/t40001.cc"));
        REQUIRE(IsFile(j, "yaml-cpp/yaml.h"));

        REQUIRE(IsFile(j, "string"));

        REQUIRE(IsAssociation(j, "src/t40001.cc", "include/t40001_include1.h"));
        REQUIRE(IsAssociation(
            j, "include/t40001_include1.h", "include/lib1/lib1.h"));
        REQUIRE(IsDependency(j, "include/t40001_include1.h", "string"));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_include_mermaid(diagram, *model);

        mermaid::AliasMatcher _A(src);
        using mermaid::HasComment;
        using mermaid::HasTitle;
        using mermaid::IsFile;
        using mermaid::IsFolder;
        using mermaid::IsIncludeDependency;

        REQUIRE_THAT(src, HasTitle("Basic include diagram example"));

        REQUIRE_THAT(src, IsFolder(_A("lib1")));
        REQUIRE_THAT(src, IsFile(_A("lib1.h")));
        REQUIRE_THAT(src, IsFile(_A("t40001.cc")));
        REQUIRE_THAT(src, IsFile(_A("t40001_include1.h")));

        REQUIRE_THAT(src, IsFile(_A("string")));
        REQUIRE_THAT(src, IsFile(_A("yaml-cpp/yaml.h")));

        REQUIRE_THAT(
            src, IsAssociation(_A("t40001.cc"), _A("t40001_include1.h")));
        REQUIRE_THAT(src, IsAssociation(_A("t40001_include1.h"), _A("lib1.h")));

        REQUIRE_THAT(
            src, IsIncludeDependency(_A("t40001_include1.h"), _A("string")));

        REQUIRE_THAT(src, HasComment("t40001 test diagram of type include"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }
}
