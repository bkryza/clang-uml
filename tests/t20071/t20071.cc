#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace clanguml::t20071 {

auto switch_to_new_thread(std::thread &out)
{
    struct awaitable_on_thread {
        std::thread *p_out;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h)
        {
            auto &out = *p_out;
            if (out.joinable())
                throw std::runtime_error("Output thread parameter not empty");
            out = std::thread([h] { h.resume(); });
        }
        void await_resume() { }
    };

    return awaitable_on_thread{&out};
}

struct task {
    struct promise_type {
        task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() { }
        void unhandled_exception() { }
    };
};

task resuming_on_new_thread(std::thread &out)
{
    std::cout << "Coroutine started on thread: " << std::this_thread::get_id()
              << '\n';
    co_await switch_to_new_thread(out);
    // awaiter destroyed here
    std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id()
              << '\n';
}

int tmain()
{
    std::thread out;
    resuming_on_new_thread(out);
    return 0;
}
} // namespace clanguml::t20071
