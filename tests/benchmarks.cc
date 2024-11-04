
#include "util/util.h"

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench/nanobench.h>

#include <atomic>

int main()
{
    {
        using std::filesystem::path;
        using namespace clanguml::util;

        path child{"/a/b/c/d/include.h"};
        path base1{"/a/b/c"};

        ankerl::nanobench::Bench().run("is_relative_to",
            [&] { starts_with(child, base1); });
    }
}