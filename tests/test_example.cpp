#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include <source_file.hpp>
#include <string>
#include <complex>

TEST_CASE("Test add", "[unit-test]"){
    // not very good tests, but oh well...
    REQUIRE(add(2, 3) == 5);
    REQUIRE(add(2., 3.) == 5.);
    REQUIRE(add(0, 0) == 0);
    std::cout << "RUNNING TEST" << std::endl;
    std::complex<double> a(2., 3.);
    std::complex<double> b(-1., 20.);
    std::complex<double> c(1., 23.);
    REQUIRE(add(a,b) == c);

}
