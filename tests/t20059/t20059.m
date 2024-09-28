#import <Foundation/Foundation.h>

@protocol t20059_CommonProtocol <NSObject>
- (void)print;
@end

@interface t20059_A : NSObject <t20059_CommonProtocol>
@end

@implementation t20059_A

- (void)printImpl
{
    NSLog(@"t20059_A: Called print_impl method");
}

- (void)print
{
    [self printImpl];
}

@end

@interface t20059_B : NSObject <t20059_CommonProtocol>
@end

@implementation t20059_B

- (void)customPrintMethod
{
    NSLog(@"t20059_B: Called customPrintMethod method");
}

- (void)print
{
    [self customPrintMethod];
}

@end

@interface t20059_Base : NSObject
- (void)print;
@end

@implementation t20059_Base
- (void)print
{
}
@end

@interface t20059_C : t20059_Base
- (void)print;
@end

@implementation t20059_C
- (void)print
{
}
@end

@interface t20059_D : t20059_Base
- (void)print;
@end

@implementation t20059_D
- (void)print
{
}
@end

int t20059_tmain()
{
    @autoreleasepool {
        id<t20059_CommonProtocol> obj1 = [[t20059_A alloc] init];
        t20059_B *obj2 = [[t20059_B alloc] init];

        id obj3 = [[t20059_C alloc] init];
        t20059_D *obj4 = [[t20059_D alloc] init];

        [(t20059_A *)obj1 print];
        [obj2 print];
        [(t20059_C *)obj3 print];
        [obj4 print];
    }
    return 0;
}
