namespace clanguml::t20067 {
int baz() { return 0; }

int bar() { return baz(); }

int foo_baz() { return baz(); }

int bar_baz() { return baz(); }

int foo_bar() { return bar(); }

int bar_bar() { return bar(); }
} // namespace clanguml::t20067