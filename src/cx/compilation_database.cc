#include "compilation_database.h"

#include <filesystem>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace clanguml {
namespace cx {
compilation_database::compilation_database(CXCompilationDatabase &&d)
    : m_database{std::move(d)}
    , m_index{clang_createIndex(0, 1)}
{
}

compilation_database::~compilation_database()
{
    clang_CompilationDatabase_dispose(m_database);
}

compilation_database compilation_database::from_directory(
    const std::string &dir)
{
    CXCompilationDatabase_Error error;
    auto path = std::filesystem::path{dir};
    CXCompilationDatabase cdb =
        clang_CompilationDatabase_fromDirectory(path.c_str(), &error);

    if (error != CXCompilationDatabase_Error::CXCompilationDatabase_NoError) {
        throw std::runtime_error(fmt::format(
            "Cannot load compilation database database from: {}", dir));
    }

    return compilation_database{std::move(cdb)};
}

CXTranslationUnit compilation_database::parse_translation_unit(
    const std::string &path)
{
    const auto p = std::filesystem::canonical(path);

    CXCompileCommands compile_commands =
        clang_CompilationDatabase_getCompileCommands(m_database, p.c_str());

    unsigned int compile_commands_count =
        clang_CompileCommands_getSize(compile_commands);

    int i;
    // for (i = 0; i < compile_commands_count; i++) {
    CXCompileCommand compile_command =
        clang_CompileCommands_getCommand(compile_commands, 0);

    auto cc_filename = clang_CompileCommand_getFilename(compile_command);
    spdlog::debug(
        "Processing compile command file: {}", clang_getCString(cc_filename));

    auto num_args = clang_CompileCommand_getNumArgs(compile_command);

    char **arguments = NULL;
    if (num_args) {
        int j;
        arguments = (char **)malloc(sizeof(char *) * num_args);
        for (j = 0; j < num_args; ++j) {
            CXString arg = clang_CompileCommand_getArg(compile_command, j);
            spdlog::debug("Processing argument: {}", clang_getCString(arg));
            arguments[j] = strdup(clang_getCString(arg));
            clang_disposeString(arg);
        }
    }

    CXTranslationUnit tu = clang_parseTranslationUnit(m_index, nullptr,
        (const char *const *)arguments, num_args, NULL, 0,
        CXTranslationUnit_None);

    if (num_args) {
        int j;
        for (j = 0; j < num_args; ++j) {
            free(arguments[j]);
        }
        free(arguments);
    }
    //}

    return tu;
}

CXCompilationDatabase &compilation_database::db() { return m_database; }
}
}
