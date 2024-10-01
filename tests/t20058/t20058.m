#import <Foundation/Foundation.h>

@interface t20058_A : NSObject
- (NSString *)a:(NSString *)inputString;
@end

@implementation t20058_A
- (NSString *)a:(NSString *)inputString
{
    return [NSString stringWithFormat:@"a%@", inputString];
}
@end

@interface t20058_B : NSObject
- (NSString *)b:(NSString *)inputString withInt:(int)number;
@end

@implementation t20058_B {
    t20058_A *a;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        a = [[t20058_A alloc] init];
    }
    return self;
}

- (NSString *)b:(NSString *)inputString withInt:(int)number
{
    NSString *modifiedString =
        [inputString stringByAppendingFormat:@"%d", number];

    NSString *result = [a a:modifiedString];

    return result;
}
@end

int t20058_tmain()
{
    @autoreleasepool {
        t20058_B *b = [[t20058_B alloc] init];
        NSString *result = [b b:@"t20058_string" withInt:42];

        t20058_A *a = [[t20058_A alloc] init];

        NSString *result2 = [a a:@"t20058_string"];

        (void)result;
        (void)result2;
    }
    return 0;
}