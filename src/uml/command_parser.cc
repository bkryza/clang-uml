/**
 * src/uml/command_parser.cc
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
#include "command_parser.h"

#include "util/util.h"

#include <string>
#include <string_view>

namespace clanguml {
namespace command_parser {

std::shared_ptr<command> command::from_string(std::string_view c)
{
    if (c.find("note") == 0) {
        return note::from_string(c);
    }
    else if (c.find("style") == 0) {
        return style::from_string(c);
    }
    else if (c.find("aggregation") == 0) {
        return aggregation::from_string(c);
    }

    return {};
}

std::shared_ptr<command> note::from_string(std::string_view c)
{
    auto res = std::make_shared<note>();
    auto it = c.begin();
    // Skip 'note'
    std::advance(it, 4);

    if (*it != '[')
        return {};

    std::advance(it, 1);

    auto pos = std::distance(c.begin(), it);
    res->position = c.substr(pos, c.find("]", pos) - pos);

    std::advance(it, res->position.size() + 1);

    pos = std::distance(c.begin(), it);
    res->text = c.substr(pos, c.find("}", pos) - pos);

    res->position = util::trim(res->position);
    res->text = util::trim(res->text);

    return res;
}

std::shared_ptr<command> style::from_string(std::string_view c)
{
    auto res = std::make_shared<style>();
    auto it = c.begin();
    // Skip 'style'
    std::advance(it, 5);

    if (*it != '[')
        return {};

    std::advance(it, 1);

    auto pos = std::distance(c.begin(), it);
    res->spec = c.substr(pos, c.find("]", pos) - pos);

    res->spec = util::trim(res->spec);

    return res;
}

std::shared_ptr<command> aggregation::from_string(std::string_view c)
{
    auto res = std::make_shared<aggregation>();
    auto it = c.begin();
    // Skip 'aggregation'
    std::advance(it, 11);

    if (*it != '[')
        return {};

    std::advance(it, 1);

    auto pos = std::distance(c.begin(), it);
    res->multiplicity = c.substr(pos, c.find("]", pos) - pos);

    res->multiplicity = util::trim(res->multiplicity);

    return res;
}

std::vector<std::shared_ptr<command>> parse(std::string documentation_block)
{
    std::vector<std::shared_ptr<command>> res;
    const std::string begin_tag{"@clanguml"};
    const auto begin_tag_size = begin_tag.size();

    // First replace all \clanguml occurences with @clanguml
    util::replace_all(documentation_block, "\\clanguml", "@clanguml");
    documentation_block = util::trim(documentation_block);

    std::string_view block_view{documentation_block};

    auto pos = block_view.find("@clanguml{");
    while (pos < documentation_block.size()) {
        auto c_begin = pos + begin_tag_size;
        auto c_end = documentation_block.find("}", c_begin);

        if (c_end == std::string::npos)
            return res;

        auto com =
            command::from_string(block_view.substr(c_begin + 1, c_end - 2));

        if (com)
            res.emplace_back(std::move(com));

        pos = block_view.find("@clanguml{", c_end);
    }

    return res;
};

} // namespace command_parser
} // namespace clanguml

