#include <iostream>
#include <memory>
#include <unordered_map>

namespace clanguml {
namespace t00026 {

template <typename T> class Memento {
public:
    Memento(T &&v)
        : m_value(std::forward<T>(v))
    {
    }

    T value() const { return m_value; }

private:
    T m_value;
};

template <typename T> class Originator {
public:
    Originator(T &&v)
        : m_value(std::forward<T>(v))
    {
    }

    Memento<T> memoize_value() const { return Memento<T>{m_value}; }

    void load(const Memento<T> &m) { m_value = m.value(); }

    void print() const { std::cout << m_value << std::endl; }

    void set(T &&v) { m_value = std::forward<T>(v); }

private:
    T m_value;
};

template <typename T> class Caretaker {
public:
    Memento<T> &state(const std::string &n) { return m_mementos.at(n); }

    void set_state(const std::string &s, Memento<T> &&m)
    {
        m_mementos.try_emplace(s, std::move(m));
    }

private:
    std::unordered_map<std::string, Memento<T>> m_mementos;
};

struct StringMemento {
    Caretaker<std::string> caretaker;
    Originator<std::string> originator;
};
} // namespace t00026
} // namespace clanguml
