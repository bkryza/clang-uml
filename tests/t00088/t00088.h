#import <Foundation/Foundation.h>

struct It00088_Foo { };

@protocol Pr00088

@end

@interface It00088_Foo : NSObject {
    int _fooMember;
    int _barMember;
}

+ (void)foo;
+ (void)bar;
+ (void)baz:(int)b with:(int)c;

@end

@interface It00088_Bar : NSObject {
}

@end