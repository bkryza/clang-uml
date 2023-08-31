namespace clanguml {
namespace t20035 {

int c(int arg) { return arg; }

int b1(int arg) { return c(arg); }

int b2(int arg) { return c(arg); }

int a(int arg) { return b1(arg); }

int tmain(int argc, char **argv) { return a(argc); }
}
}