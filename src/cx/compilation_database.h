/**
 * src/cx/compilation_database.h
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
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
#pragma once

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <functional>
#include <memory>
#include <string>

namespace clanguml {
namespace cx {

class compilation_database {
public:
    compilation_database(CXCompilationDatabase &&d);
    ~compilation_database();

    compilation_database(compilation_database &&d);

    CXCompilationDatabase &db();

    CXIndex &index();

    CXTranslationUnit parse_translation_unit(
        const std::string &path);

    static compilation_database from_directory(const std::string &dir);

private:
    CXCompilationDatabase m_database;
    CXIndex m_index;
};
}
}
