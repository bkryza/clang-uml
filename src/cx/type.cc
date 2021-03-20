/**
 * src/cx/type.cc
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
#include "type.h"
#include "cursor.h"

namespace clanguml {
namespace cx {

cursor type::type_declaration() const
{
    return {clang_getTypeDeclaration(m_type)};
}

std::string type::instantiation_template() const
{
    assert(is_template_instantiation());

    auto s = spelling();
    auto it = s.find('<');
    auto template_base_name = s.substr(0, it);

    auto cur = type_declaration();

    return cur.fully_qualified();
}

bool type::is_template_instantiation() const
{
    auto s = spelling();
    auto it = s.find('<');
    return it != std::string::npos &&
        referenced().type_declaration().kind() != CXCursor_ClassTemplate;
}
}
}
