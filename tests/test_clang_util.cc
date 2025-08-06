/**
 * @file tests/test_clang_util.cc
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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "common/clang_utils.h"
#include "common/generators/clang_tool.h"

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <nlohmann/json.hpp>

#include "doctest/doctest.h"

TEST_CASE("Test to_string DiagnosticsEngine::Level")
{
    using namespace clang;

    CHECK(to_string(DiagnosticsEngine::Ignored) == "IGNORED");
    CHECK(to_string(DiagnosticsEngine::Note) == "NOTE");
    CHECK(to_string(DiagnosticsEngine::Remark) == "REMARK");
    CHECK(to_string(DiagnosticsEngine::Warning) == "WARNING");
    CHECK(to_string(DiagnosticsEngine::Error) == "ERROR");
    CHECK(to_string(DiagnosticsEngine::Fatal) == "FATAL");
}

TEST_CASE("Test to_string diagnostic")
{
    using namespace clanguml::generators;
    using namespace clanguml::common::model;

    {
        diagnostic d;
        d.level = clang::DiagnosticsEngine::Error;
        d.description = "test error message";

        CHECK(to_string(d) == "[ERROR] test error message");
    }

    {
        diagnostic d;
        d.level = clang::DiagnosticsEngine::Warning;
        d.description = "test warning message";
        d.location = source_location{};
        d.location->set_file("/path/to/file.cpp");
        d.location->set_line(42);

        CHECK(to_string(d) ==
            "[WARNING] /path/to/file.cpp:42: test warning message");
    }

    {
        diagnostic d;
        d.level = clang::DiagnosticsEngine::Note;
        d.description = "test note message";
        d.location = source_location{};
        d.location->set_file("/absolute/path/to/file.cpp");
        d.location->set_file_relative("relative/path/to/file.cpp");
        d.location->set_line(123);

        CHECK(to_string(d) ==
            "[NOTE] relative/path/to/file.cpp:123: test note message");
    }
}

TEST_CASE("Test to_json diagnostic")
{
    using namespace clanguml::generators;
    using namespace clanguml::common::model;

    {
        diagnostic d;
        d.level = clang::DiagnosticsEngine::Error;
        d.description = "test error message";

        nlohmann::json j;
        to_json(j, d);

        CHECK(j["level"] == clang::DiagnosticsEngine::Error);
        CHECK(j["description"] == "test error message");
        CHECK(!j.contains("location"));
    }

    {
        diagnostic d;
        d.level = clang::DiagnosticsEngine::Warning;
        d.description = "test warning message";
        d.location = source_location{};
        d.location->set_file("/path/to/file.cpp");
        d.location->set_line(42);
        d.location->set_column(10);

        nlohmann::json j;
        to_json(j, d);

        CHECK(j["level"] == clang::DiagnosticsEngine::Warning);
        CHECK(j["description"] == "test warning message");
        CHECK(j["location"]["file"] == "/path/to/file.cpp");
        CHECK(j["location"]["line"] == 42);
        CHECK(j["location"]["column"] == 10);
    }

    {
        diagnostic d;
        d.level = clang::DiagnosticsEngine::Note;
        d.description = "message with \"quotes\" and \\backslashes\\";

        nlohmann::json j;
        to_json(j, d);

        CHECK(j["level"] == clang::DiagnosticsEngine::Note);
        CHECK(j["description"] ==
            "message with \\\"quotes\\\" and \\\\backslashes\\\\");
        CHECK(!j.contains("location"));
    }
}

TEST_CASE("Test diagnostic_consumer")
{
    using namespace clanguml::generators;
    using namespace clang;

    diagnostic_consumer consumer("/tmp");

    CHECK_FALSE(consumer.failed);
    CHECK(consumer.diagnostics.empty());

    auto fs = llvm::vfs::getRealFileSystem();
    FileSystemOptions file_opts;
    FileManager file_mgr(file_opts, fs);
    auto diagnostic_engine = DiagnosticsEngine(
        llvm::IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
        new DiagnosticOptions());
    SourceManager source_mgr(diagnostic_engine, file_mgr);

    DiagnosticsEngine diag_engine(
        llvm::IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
        new DiagnosticOptions());

    diag_engine.setClient(&consumer, false);
    auto id_note = diag_engine.getCustomDiagID(
        DiagnosticsEngine::Note, "test note message");

    StoredDiagnostic note_diag(
        DiagnosticsEngine::Note, id_note, "test note message");
    diag_engine.Report(note_diag);

    CHECK_FALSE(consumer.failed);
    CHECK(consumer.diagnostics.size() == 1);
    CHECK(consumer.diagnostics[0].level == DiagnosticsEngine::Note);
    CHECK(consumer.diagnostics[0].description == "test note message");

    id_note = diag_engine.getCustomDiagID(
        DiagnosticsEngine::Note, "test warning message");
    StoredDiagnostic warning_diag(
        DiagnosticsEngine::Warning, id_note, "test warning message");
    diag_engine.Report(warning_diag);

    CHECK_FALSE(consumer.failed);
    CHECK(consumer.diagnostics.size() == 2);
    CHECK(consumer.diagnostics[1].level == DiagnosticsEngine::Warning);
    CHECK(consumer.diagnostics[1].description == "test warning message");

    id_note = diag_engine.getCustomDiagID(
        DiagnosticsEngine::Note, "test error message");
    StoredDiagnostic error_diag(
        DiagnosticsEngine::Error, id_note, "test error message");
    diag_engine.Report(error_diag);

    CHECK(consumer.failed);
    CHECK(consumer.diagnostics.size() == 3);
    CHECK(consumer.diagnostics[2].level == DiagnosticsEngine::Error);
    CHECK(consumer.diagnostics[2].description == "test error message");

    id_note = diag_engine.getCustomDiagID(
        DiagnosticsEngine::Note, "test fatal message");
    StoredDiagnostic fatal_diag(
        DiagnosticsEngine::Fatal, id_note, "test fatal message");
    diag_engine.Report(fatal_diag);

    CHECK(consumer.failed);
    CHECK(consumer.diagnostics.size() == 4);
    CHECK(consumer.diagnostics[3].level == DiagnosticsEngine::Fatal);
    CHECK(consumer.diagnostics[3].description == "test fatal message");
}