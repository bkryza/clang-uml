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
