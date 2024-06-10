namespace clanguml {
namespace t00037 {

constexpr auto LENGTH{10ULL};

struct S {
    double x;
    double y;
};

class ST {
public:
    struct {
        double t;
        double x;
        double y;
        double z;
    } dimensions;

    struct {
        int len;
        int flags;
    }
#ifndef _MSC_VER
    __attribute__((packed))
#endif
    bars[LENGTH];

private:
    struct {
        double c{1.0};
        double h{1.0};
    } units;

    S s[4][3][2];
};

struct A {
    A()
    {
        st.dimensions.t = -1.0;
        st.dimensions.x = 1.0;
        st.dimensions.y = 1.0;
        st.dimensions.z = 1.0;
    }

    ST st;
};

} // namespace t00037
} // namespace clanguml
