namespace clanguml {
namespace t00037 {

class ST {
public:
    struct {
        double t;
        double x;
        double y;
        double z;
    } dimensions;

private:
    struct {
        double c{1.0};
        double h{1.0};
    } units;
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
