namespace clanguml {
namespace t30004 {

/// @uml{style[#green]}
namespace A {

/// @uml{note[ bottom ] Package AAA.}
namespace AAA {
}

/// \uml{note[right] Package BBB.}
namespace BBB {
}

///
/// @uml{note:t30004_package[bottom] CCCC package note.}
/// This is package CCC.
namespace CCC {
}

/// \uml{skip}
namespace DDD {
}

/// @uml{style[#pink;line:red;line.bold;text:red]}
/// \uml{note[top] We skipped DDD.}
namespace EEE {
}

/// \uml{note[top] Another CCC note.}
namespace CCC {
}
}
}
}