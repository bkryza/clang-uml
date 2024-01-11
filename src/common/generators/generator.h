/**
 * @file src/common/generators/generator.h
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

#include <ostream>

namespace clanguml::common::generators {

/**
 * @brief Common diagram generator interface
 *
 * This class defines common interface for all diagram generators.
 */
template <typename ConfigType, typename DiagramType> class generator {
public:
    /**
     * @brief Constructor
     *
     * @param config Reference to instance of @link clanguml::config::diagram
     * @param model Reference to instance of @link clanguml::model::diagram
     */
    generator(ConfigType &config, DiagramType &model)
        : config_{config}
        , model_{model}
    {
    }

    virtual ~generator() = default;

    /**
     * @brief Generate diagram
     *
     * This is the main diagram generation entrypoint. It is responsible for
     * calling other methods in appropriate order to generate the diagram into
     * the output stream. It generates diagram elements, that are common
     * to all types of diagrams in a given generator.
     *
     * @param ostr Output stream
     */
    virtual void generate(std::ostream &ostr) const = 0;

    /**
     * @brief Get reference to diagram config
     *
     * @return Diagram config
     */
    const ConfigType &config() const { return config_; }

    /**
     * @brief Get reference to diagram model
     *
     * @return Diagram model
     */
    const DiagramType &model() const { return model_; }

private:
    ConfigType &config_;
    DiagramType &model_;
};

} // namespace clanguml::common::generators