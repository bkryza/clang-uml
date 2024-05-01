/**
 * @file src/decorators/decorators.cc
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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

namespace clanguml::decorators {

std::shared_ptr<decorator> decorator::from_string(std::string_view c)
{
    if (c.find(note::label) == 0) {
        return note::from_string(c);
    }
    if (c.find(skip_relationship::label) == 0) {
        return skip_relationship::from_string(c);
    }
    if (c.find(skip::label) == 0) {
        return skip::from_string(c);
    }
    if (c.find(style::label) == 0) {
        return style::from_string(c);
    }
    if (c.find(aggregation::label) == 0) {
        return aggregation::from_string(c);
    }
    if (c.find(composition::label) == 0) {
        return composition::from_string(c);
    }
    if (c.find(association::label) == 0) {
        return association::from_string(c);
    }
    if (c.find(call::label) == 0) {
        return call::from_string(c);
    }

    return {};
}

bool decorator::applies_to_diagram(const std::string &name)
{
    return diagrams.empty() ||
        (std::find(diagrams.begin(), diagrams.end(), name) != diagrams.end());
}

decorator_toks decorator::tokenize(const std::string &label, std::string_view c)
{
    decorator_toks res;
    res.label = label;
    size_t pos{};
    std::string_view::const_iterator it = c.begin();
    std::advance(it, label.size());

    if (*it == ':') {
        std::advance(it, 1);

        pos = std::distance(c.begin(), it);
        // If the diagram list is provided after ':', [] is mandatory
        // even if empty
        auto d = c.substr(pos, c.find('[', pos) - pos);
        if (!d.empty()) {
            std::string d_str{d};
            d_str.erase(std::remove_if(d_str.begin(), d_str.end(),
                            static_cast<int (*)(int)>(std::isspace)),
                d_str.end());
            res.diagrams = util::split(d_str, ",");
        }

        std::advance(it, d.size());
    }

    if (*it == '[') {
        std::advance(it, 1);

        pos = std::distance(c.begin(), it);
        res.param = c.substr(pos, c.find(']', pos) - pos);

        std::advance(it, res.param.size() + 1);
    }
    else if (std::isspace(*it) != 0) {
        std::advance(it, 1);
    }

    pos = std::distance(c.begin(), it);
    res.text = c.substr(pos, c.find('}', pos) - pos);
    res.text = util::trim(res.text);
    res.param = util::trim(res.param);

    return res;
}

std::shared_ptr<decorator> note::from_string(std::string_view c)
{
    auto res = std::make_shared<note>();
    auto toks = res->tokenize(note::label, c);

    res->diagrams = toks.diagrams;

    if (!toks.param.empty())
        res->position = toks.param;

    res->text = toks.text;

    return res;
}

std::shared_ptr<decorator> skip::from_string(std::string_view /*c*/)
{
    return std::make_shared<skip>();
}

std::shared_ptr<decorator> skip_relationship::from_string(
    std::string_view /*c*/)
{
    return std::make_shared<skip_relationship>();
}

std::shared_ptr<decorator> style::from_string(std::string_view c)
{
    auto res = std::make_shared<style>();
    auto toks = res->tokenize(style::label, c);

    res->diagrams = toks.diagrams;
    res->spec = toks.param;

    return res;
}

std::shared_ptr<decorator> aggregation::from_string(std::string_view c)
{
    auto res = std::make_shared<aggregation>();
    auto toks = res->tokenize(aggregation::label, c);

    res->diagrams = toks.diagrams;
    res->multiplicity = toks.param;

    return res;
}

std::shared_ptr<decorator> composition::from_string(std::string_view c)
{
    auto res = std::make_shared<composition>();
    auto toks = res->tokenize(composition::label, c);

    res->diagrams = toks.diagrams;
    res->multiplicity = toks.param;

    return res;
}

std::shared_ptr<decorator> association::from_string(std::string_view c)
{
    auto res = std::make_shared<association>();
    auto toks = res->tokenize(association::label, c);

    res->diagrams = toks.diagrams;
    res->multiplicity = toks.param;

    return res;
}

std::shared_ptr<decorator> call::from_string(std::string_view c)
{
    auto res = std::make_shared<call>();
    auto toks = res->tokenize(call::label, c);

    res->diagrams = toks.diagrams;
    res->callee = util::trim(toks.text);

    return res;
}

std::pair<std::vector<std::shared_ptr<decorator>>, std::string> parse(
    std::string documentation_block, const std::string &clanguml_tag)
{
    std::vector<std::shared_ptr<decorator>> res;
    std::string stripped_comment;

    const std::string begin_tag{"@" + clanguml_tag};
    const auto begin_tag_size = begin_tag.size();

    // First replace all \uml occurences with @uml
    util::replace_all(
        documentation_block, "\\" + clanguml_tag, "@" + clanguml_tag);
    documentation_block = util::trim(documentation_block);

    const std::string_view block_view{documentation_block};

    auto pos = block_view.find("@" + clanguml_tag + "{");

    if (pos == std::string::npos) {
        // This comment had no uml directives
        return {{}, util::trim(documentation_block)};
    }

    size_t last_end_pos{0};
    while (pos < documentation_block.size()) {
        auto c_begin = pos + begin_tag_size;
        auto c_end = block_view.find('}', c_begin);

        if (c_end == std::string::npos) {
            return {res, util::trim(stripped_comment)};
        }

        auto com =
            decorator::from_string(block_view.substr(c_begin + 1, c_end - 2));

        if (com)
            res.emplace_back(std::move(com));

        const auto in_between_length = pos - last_end_pos;
        stripped_comment += block_view.substr(last_end_pos, in_between_length);

        last_end_pos = pos + (c_end - c_begin + begin_tag_size + 1);

        pos = block_view.find("@" + clanguml_tag + "{", c_end);
    }

    return {res, util::trim(stripped_comment)};
};

} // namespace clanguml::decorators
