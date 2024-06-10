namespace clanguml {

/// Vivamus integer non suscipit taciti mus
class A { };

namespace t00050 {

/// Lorem ipsum dolor sit
class A { };

/**
 * \brief Lorem ipsum
 *
 * Lorem ipsum dolor sit amet consectetur adipiscing elit, urna consequat felis
 * vehicula class ultricies mollis dictumst, aenean non a in donec nulla.
 * Phasellus ante pellentesque erat cum risus consequat imperdiet aliquam,
 * integer placerat et turpis mi eros nec lobortis taciti, vehicula nisl litora
 * tellus ligula porttitor metus.
 *
 * \todo 1. Write meaningful comment
 * \todo 2. Write tests
 * \todo 3. Implement
 */
class B { };

/// \brief Long comment example
///
/// Lorem ipsum dolor sit amet consectetur adipiscing elit, urna consequat felis
/// vehicula class ultricies mollis dictumst, aenean non a in donec nulla.
/// Phasellus ante pellentesque erat cum risus consequat imperdiet aliquam,
/// integer placerat et turpis mi eros nec lobortis taciti, vehicula nisl litora
/// tellus ligula porttitor metus.
///
/// Vivamus integer non suscipit taciti mus etiam at primis tempor sagittis sit,
/// euismod libero facilisi aptent elementum felis blandit cursus gravida sociis
/// erat ante, eleifend lectus nullam dapibus netus feugiat curae curabitur est
/// ad. Massa curae fringilla porttitor quam sollicitudin iaculis aptent leo
/// ligula euismod dictumst, orci penatibus mauris eros etiam praesent erat
/// volutpat posuere hac. Metus fringilla nec ullamcorper odio aliquam lacinia
/// conubia mauris tempor, etiam ultricies proin quisque lectus sociis id
/// tristique, integer phasellus taciti pretium adipiscing tortor sagittis
/// ligula.
///
/// Mollis pretium lorem primis senectus habitasse lectus scelerisque
/// donec, ultricies tortor suspendisse adipiscing fusce morbi volutpat
/// pellentesque, consectetur mi risus molestie curae malesuada cum. Dignissim
/// lacus convallis massa mauris enim ad mattis magnis senectus montes, mollis
/// taciti phasellus accumsan bibendum semper blandit suspendisse faucibus nibh
/// est, metus lobortis morbi cras magna vivamus per risus fermentum. Dapibus
/// imperdiet praesent magnis ridiculus congue gravida curabitur dictum
/// sagittis, enim et magna sit inceptos sodales parturient pharetra mollis,
/// aenean vel nostra tellus commodo pretium sapien sociosqu.
class C { };

/// Mollis pretium lorem primis
namespace utils {

/// Lorem ipsum dolor sit amet consectetur adipiscing elit, urna consequat felis
/// vehicula class ultricies mollis dictumst, aenean non a in donec nulla.
/// Phasellus ante pellentesque erat cum risus consequat imperdiet aliquam,
/// integer placerat et turpis mi eros nec lobortis taciti, vehicula nisl litora
/// tellus ligula porttitor metus.
///
/// \todo Implement...
class D { };

} // namespace utils

/// Mollis pretium lorem primis
enum class E { E1, E2, E3 };

/// \brief Simple array wrapper.
///
/// This class is just for testing tparam parsing, it serves no other
/// purpose.
///
/// \tparam T Type of array elements.
/// \tparam V Type of regular element.
/// \tparam N Size of T array.
///
template <typename T, typename V, int N> class F {
    T t[N];
    V v;

    /// \brief Set value of v
    ///
    /// \param v_ New value for v
    V set_value(V v_) const { return v = v_; }
};

/// This is a short description of class G.
///
/// This is an intermediate description of class G.
///
/// This is a long description of class G.
class G { };

class NoComment { };

} // namespace t00050
} // namespace clanguml