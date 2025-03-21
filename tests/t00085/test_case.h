/**
 * tests/t00085/test_case.h
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

TEST_CASE("t00085")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_CLASS_MODEL("t00085", "t00085_class");

    CHECK_CLASS_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        // REQUIRE(HasTitle(src, "Basic class diagram example"));
        REQUIRE(IsObjCInterface(src, "NSObject"));

        REQUIRE(IsObjCInterface(src, "It00085"));
        // REQUIRE(IsBaseClass(src, "A", "B"));

        REQUIRE(IsField<Protected>(src, "It00085", "_defaultMember", "int"));
        REQUIRE(IsField<Public>(src, "It00085", "_publicMember", "NSString *"));
        REQUIRE(IsField<Protected>(
            src, "It00085", "_protectedMember", "NSString *"));
        REQUIRE(
            IsField<Private>(src, "It00085", "_privateMember", "NSString *"));

        REQUIRE(IsMethod<Public>(
            src, "It00085", "addValue:", "NSInteger", "NSInteger otherValue"));
        REQUIRE(IsMethod<Public, Static>(
            src, "It00085", "currentSharedCounter", "NSInteger"));
        REQUIRE(IsMethod<Public>(src, "It00085", "displayDetails", "void"));
        REQUIRE(IsMethod<Public, Static>(
            src, "It00085", "incrementSharedCounter", "void"));
        REQUIRE(IsMethod<Public>(src, "It00085", "initWithName:value:",
            "instancetype", "NSString * name, NSInteger value"));
        REQUIRE(IsMethod<Public>(src, "It00085", "multiplyValueBy:andAdd:",
            "NSInteger", "NSInteger multiplier, NSInteger addition"));
        REQUIRE(IsMethod<Public>(src, "It00085", "name", "NSString *"));
        REQUIRE(IsMethod<Public>(src, "It00085",
            "performOperationWithBlock:", "void", "void (^)(NSInteger) block"));
        REQUIRE(IsMethod<Public>(src, "It00085", "resetValue", "void"));
        REQUIRE(IsMethod<Public>(
            src, "It00085", "setName:", "void", "NSString * name"));
        REQUIRE(IsMethod<Public, Static>(src, "It00085",
            "setSharedCounter:", "void", "NSInteger sharedCounter"));
        REQUIRE(IsMethod<Public>(
            src, "It00085", "setValue:", "void", "NSInteger value"));
        REQUIRE(IsMethod<Public, Static>(
            src, "It00085", "sharedCounter", "NSInteger"));
        REQUIRE(IsMethod<Public, Static>(
            src, "It00085", "sharedInstance", "instancetype"));
        REQUIRE(IsMethod<Public>(src, "It00085", "value", "NSInteger"));
    });
}