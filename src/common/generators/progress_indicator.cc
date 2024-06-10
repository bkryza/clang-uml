/**
 * @file src/common/generators/progress_indicator.cc
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

#include "progress_indicator.h"

#include <util/util.h>

namespace clanguml::common::generators {

progress_indicator::progress_indicator()
    : progress_indicator(std::cout)
{
}

progress_indicator::progress_indicator(std::ostream &ostream)
    : ostream_(ostream)
{
    progress_bars_.set_option(indicators::option::HideBarWhenComplete{false});
}

void progress_indicator::add_progress_bar(
    const std::string &name, size_t max, indicators::Color color)
{
    auto postfix_text = max > 0 ? fmt::format("{}/{}", 0, max) : std::string{};

    const auto kBarWidth = 35U;
    const auto kPrefixTextWidth = 25U;

    auto bar = std::make_shared<indicators::ProgressBar>(
        indicators::option::Stream{ostream_},
        indicators::option::BarWidth{kBarWidth},
        indicators::option::ForegroundColor{color},
        indicators::option::ShowElapsedTime{true},
#if _MSC_VER
        indicators::option::Fill{"="}, indicators::option::Lead{">"},
#else
        indicators::option::Fill{"█"}, indicators::option::Lead{"█"},
#endif
        indicators::option::Remainder{"-"},
        indicators::option::PrefixText{
            fmt::format("{:<25}", util::abbreviate(name, kPrefixTextWidth))},
        indicators::option::PostfixText{postfix_text});

    progress_bars_mutex_.lock();

    progress_bars_.push_back(*bar);
    bars_.push_back(bar);
    auto bar_index = bars_.size() - 1;
    progress_bar_index_.emplace(name, progress_state{bar_index, 0, max});

    progress_bars_mutex_.unlock();
}

void progress_indicator::increment(const std::string &name)
{
    const auto kASTTraverseProgressPercent = 95U;

    progress_bars_mutex_.lock();

    if (progress_bar_index_.count(name) == 0) {
        progress_bars_mutex_.unlock();
        return;
    }

    auto &p = progress_bar_index_.at(name);
    auto &bar = progress_bars_[p.index];

    p.progress++;

    bar.set_progress((p.progress * kASTTraverseProgressPercent) / p.max);
    bar.set_option(indicators::option::PostfixText{
        fmt::format("{}/{}", p.progress, p.max)});

    progress_bars_mutex_.unlock();
}

void progress_indicator::stop()
{
    progress_bars_mutex_.lock();

    for (auto &[name, p] : progress_bar_index_) {
        progress_bars_[p.index].mark_as_completed();
    }

    progress_bars_mutex_.unlock();
}

void progress_indicator::complete(const std::string &name)
{
    const auto kCompleteProgressPercent = 100U;

    progress_bars_mutex_.lock();

    if (progress_bar_index_.count(name) == 0) {
        progress_bars_mutex_.unlock();
        return;
    }

    auto &p = progress_bar_index_.at(name);
    auto &bar = progress_bars_[p.index];

    p.progress = p.max;

    bar.set_progress(kCompleteProgressPercent);

#if _MSC_VER
    const auto postfix_text = fmt::format("{}/{} OK", p.progress, p.max);
#else
    const auto postfix_text = fmt::format("{}/{} ✔", p.progress, p.max);
#endif
    bar.set_option(indicators::option::PostfixText{postfix_text});
    bar.set_option(
        indicators::option::ForegroundColor{indicators::Color::green});
    bar.mark_as_completed();

    progress_bars_mutex_.unlock();
}

void progress_indicator::fail(const std::string &name)
{
    progress_bars_mutex_.lock();

    if (progress_bar_index_.count(name) == 0) {
        progress_bars_mutex_.unlock();
        return;
    }

    auto &p = progress_bar_index_.at(name);
    auto &bar = progress_bars_[p.index];

#if _MSC_VER
    const auto postfix_text = fmt::format("{}/{} FAILED", p.progress, p.max);
#else
    const auto postfix_text = fmt::format("{}/{} ✗", p.progress, p.max);
#endif
    bar.set_option(indicators::option::ForegroundColor{indicators::Color::red});
    bar.set_option(indicators::option::PostfixText{postfix_text});
    bar.mark_as_completed();

    progress_bars_mutex_.unlock();
}

} // namespace clanguml::common::generators