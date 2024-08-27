#include <Foundation/Foundation.h>

/// \uml{note[top] Somehow it's ok to have a C struct and an ObjC
/// interface with  the same name.}
struct CULogger { };

@protocol PAdd
- (void)add;
@end

@protocol PSub
- (void)subtract;
@end

@class CULogger;

@interface CUArithmetic : NSObject <PAdd, PSub> {
@protected
    CULogger *logger;
}
@end

struct Result {
    int result;
};

struct Serializer { };

@interface CUIntArithmetic : CUArithmetic {
    int a;
    int b;
    struct Result result;
    struct Serializer *s18n;
}
+ create;

@property (nonatomic) int a, b;

- (int)get;

@end

#define ROWS 4
#define COLUMNS 4
struct Value {
    int value;
};

@interface CUMatrixArithmetic : CUArithmetic {
    struct Value data[ROWS][COLUMNS];
}

@end

@interface CULogger : NSObject {
}
@end

@interface CUMatrixArithmetic (MatrixOps)
- (void)transpose;
- (NSInteger)trace;
@end