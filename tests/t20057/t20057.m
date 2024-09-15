#import <Foundation/Foundation.h>

// Interface for t20057_A
@interface t20057_A : NSObject
- (void)a;
@end

// Implementation for t20057_A
@implementation t20057_A
- (void)a
{
}
@end

// Interface for t20057_B
@interface t20057_B : NSObject
- (void)b;
@end

// Implementation for t20057_B
@implementation t20057_B {
    t20057_A *a;
}

//- (instancetype)init
//{
//    self = [super init];
//    if (self) {
//        a = [[t20057_A alloc] init];
//    }
//    return self;
//}

- (void)b
{
    [a a]; // Calling method a
}
@end

// Interface for t20057_C
@interface t20057_C : NSObject
- (void)c;
@end

// Implementation for t20057_C
@implementation t20057_C {
    t20057_B *b;
}
//
//- (instancetype)init
//{
//    self = [super init];
//    if (self) {
//        b = [[t20057_B alloc] init];
//    }
//    return self;
//}

- (void)c
{
    [b b]; // Calling method b
}
@end

// Main function
int t20057_tmain()
{
    @autoreleasepool {
        t20057_C *c = [[t20057_C alloc] init];
        [c c];
    }
    return 0;
}