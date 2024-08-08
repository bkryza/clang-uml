#include <thread>

namespace clanguml {
namespace t00080 {

class Worker : public std::thread {
public:
    using std::thread::thread;

    ~Worker()
    {
        if (this->joinable()) {
            this->join();
        }
    }

    void start(int delay) { }
};

struct R {
    Worker *w;
};
}
}