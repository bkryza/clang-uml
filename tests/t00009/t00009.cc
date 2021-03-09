#include <string>
#include <vector>

/*
@startuml

class "A<T>" as C_0000000046
class C_0000000046 {
+T value
}

class "A<int>" as ABC
class "A<std::string>" as ABCD

class "B" as C_0000000047
class C_0000000047 {
+A<int> aint
+A<std::string> astring
}

C_0000000046 <|.. ABC
C_0000000046 <|.. ABCD

ABC <-- C_0000000047 : aint
ABCD <-- C_0000000047 : astring

@enduml
*/
namespace clanguml {
namespace t00009 {

template <typename T> class A {
public:
    T value;
};

class B {
public:
    A<int> aint;
    A<std::string> astring;
};
}
}
