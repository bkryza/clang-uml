namespace clanguml {
namespace t00037 {

struct ST {
    struct {
        double t;
        double x;
        double y;
        double z;
    } dimensions;
};

struct A {
    A()
    {
        st.dimensions.t = -1;
        st.dimensions.x = 1;
        st.dimensions.y = 1;
        st.dimensions.z = 1;
    }

    ST st;
};

} // namespace t00037
} // namespace clanguml