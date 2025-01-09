import t00072.app;

#include <memory>

namespace clanguml::t00072 {

class App {
public:
    std::unique_ptr<B> b;
};
}