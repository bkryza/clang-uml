#pragma once

namespace clanguml {
namespace t00065 {

template <typename T>
concept bconcept = requires(T t) {
                       T{};
                       t.b();
                   };

}
}