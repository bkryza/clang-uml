namespace clanguml::t00102 {

template <typename T> struct target {
    static void call() { }
};

template <template <typename> typename C, typename T> struct wrapper {
    void accept(C<T>) { }

    void invoke() { C<T>::call(); }
};

void tmain()
{
    wrapper<target, int> value;
    value.accept(target<int>{});
    value.invoke();
}

} // namespace clanguml::t00102