/**
 * tests/t20071/test_case.h
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

TEST_CASE("t20071")
{
    using namespace clanguml::test;
    using namespace std::string_literals;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20071", "t20071_sequence");

    CHECK_SEQUENCE_DIAGRAM(*config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {Entrypoint{}, "t20071.cc", "tmain()"}, //
                {"t20071.cc", "t20071.cc",
                    "resuming_on_new_thread(std::thread &)", Coroutine{}}, //
                {"t20071.cc", "t20071.cc",
                    "switch_to_new_thread(std::thread &)"}, //
#if LLVM_VERSION_MAJOR == 14 || LLVM_VERSION_MAJOR == 15
                {"t20071.cc", "t20071.cc", "struct awaitable_on_thread",
                    Response{}}, //
#else
                {"t20071.cc", "t20071.cc", "awaitable_on_thread",
                    Response{}}, //
#endif
                {"t20071.cc",
                    "switch_to_new_thread(std::thread &)::awaitable_on_thread",
                    "await_resume()", CoAwait{}} //
            }));
    });
}
