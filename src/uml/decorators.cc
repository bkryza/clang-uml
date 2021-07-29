/**
 * src/uml/decorators.cc
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
#include "decorators.h"

#include "util/util.h"

#include <string>
#include <string_view>

namespace clanguml {
namespace decorators {

const std::string note::label = "note";
const std::string skip::label = "skip";
const std::string skip_relationship::label = "skiprelationship";
const std::string style::label = "style";
const std::string aggregation::label = "aggregation";

std::shared_ptr<decorator> decorator::from_string(std::string_view c)
{
    if (c.find(note::label) == 0) {
        return note::from_string(c);
    }
    else if (c.find(skip_relationship::label) == 0) {
        return skip_relationship::from_string(c);
    }
    else if (c.find(skip::label) == 0) {
        return skip::from_string(c);
    }
    else if (c.find(style::label) == 0) {
        return style::from_string(c);
    }
    else if (c.find(aggregation::label) == 0) {
        return aggregation::from_string(c);
    }

    return {};
}

std::shared_ptr<decorator> note::from_string(std::string_view c)
{
    auto res = std::make_shared<note>();
    auto it = c.begin();
    std::advance(it, note::label.size());

    if (*it == '[') {
        std::advance(it, 1);

        auto pos = std::distance(c.begin(), it);
        auto note_position = c.substr(pos, c.find("]", pos) - pos);
        if (!note_position.empty())
            res->position = note_position;

        std::advance(it, note_position.size() + 1);
    }
    else if (std::isspace(*it)) {
        std::advance(it, 1);
    }
    else {
        LOG_WARN("Invalid note decorator: {}", c);
        return {};
    }

    auto pos = std::distance(c.begin(), it);
    res->text = c.substr(pos, c.find("}", pos) - pos);

    res->position = util::trim(res->position);
    res->text = util::trim(res->text);

    return res;
}

std::shared_ptr<decorator> skip::from_string(std::string_view c)
{
    auto res = std::make_shared<skip>();
    return res;
}

std::shared_ptr<decorator> skip_relationship::from_string(std::string_view c)
{
    auto res = std::make_shared<skip_relationship>();
    return res;
}

std::shared_ptr<decorator> style::from_string(std::string_view c)
{
    auto res = std::make_shared<style>();
    auto it = c.begin();
    std::advance(it, style::label.size());

    if (*it != '[')
        return {};

    std::advance(it, 1);

    auto pos = std::distance(c.begin(), it);
    res->spec = c.substr(pos, c.find("]", pos) - pos);

    res->spec = util::trim(res->spec);

    return res;
}

std::shared_ptr<decorator> aggregation::from_string(std::string_view c)
{
    auto res = std::make_shared<aggregation>();
    auto it = c.begin();
    std::advance(it, aggregation::label.size());

    if (*it != '[')
        return {};

    std::advance(it, 1);

    auto pos = std::distance(c.begin(), it);
    res->multiplicity = c.substr(pos, c.find("]", pos) - pos);

    res->multiplicity = util::trim(res->multiplicity);

    return res;
}

std::vector<std::shared_ptr<decorator>> parse(std::string documentation_block)
{
    std::vector<std::shared_ptr<decorator>> res;
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
            decorator::from_string(block_view.substr(c_begin + 1, c_end - 2));

        if (com)
            res.emplace_back(std::move(com));

        pos = block_view.find("@clanguml{", c_end);
    }

    return res;
};

} // namespace decorators
} // namespace clanguml

