#include "t00019_base.h"
#include "t00019_layer1.h"
#include "t00019_layer2.h"
#include "t00019_layer3.h"

#include <memory>

namespace clanguml {
namespace t00019 {

class A {
public:
    std::unique_ptr<Layer1<Layer2<Layer3<Base>>>> layers;
};
}
}
