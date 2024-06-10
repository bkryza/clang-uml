/**
 * @file src/common/generators/progress_indicator.h
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
#pragma once

#include <indicators/indicators.hpp>

#include <map>
#include <memory>
#include <vector>

namespace clanguml::common::generators {

/**
 * @brief Container for diagram generation progress indicators
 */
class progress_indicator {
public:
    struct progress_state {
        explicit progress_state(size_t i, size_t p, size_t m)
            : index{i}
            , progress{p}
            , max{m}
        {
        }

        size_t index;
        size_t progress;
        size_t max;
    };

    progress_indicator();

    progress_indicator(std::ostream &ostream);

    /**
     * Add a new progress bar to the indicator set
     *
     * @param name Name (prefix) of the progress bar
     * @param max Total number of steps in the progress bar
     * @param color Color of the progress bar
     */
    void add_progress_bar(
        const std::string &name, size_t max, indicators::Color color);

    /**
     * Increment specified progress bar.
     *
     * @param name Name of the progress bar
     */
    void increment(const std::string &name);

    /**
     * Stop all the progress bars.
     */
    void stop();

    /**
     * Set specified progress bar as complete.
     *
     * @param name Name of the progress bar
     */
    void complete(const std::string &name);

    /**
     * Set progress bar as failed.
     *
     * @param name Name of the progress bar
     */
    void fail(const std::string &name);

private:
    indicators::DynamicProgress<indicators::ProgressBar> progress_bars_;
    std::vector<std::shared_ptr<indicators::ProgressBar>> bars_;
    std::map<std::string, progress_state> progress_bar_index_;
    std::mutex progress_bars_mutex_;
    std::ostream &ostream_;
};
} // namespace clanguml::common::generators
