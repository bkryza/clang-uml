#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <string>
#include <complex>

TEST_CASE("Test add", "[unit-test]"){
    // not very good tests, but oh well...
    REQUIRE(2+5 == 5);
    std::cout << "RUNNING TEST" << std::endl;
}
