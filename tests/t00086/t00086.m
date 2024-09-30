#import <Foundation/NSObject.h>

@interface t00086_a : NSObject <NSCopying, NSMutableCopying> {
    @public
    enum Color { Red, Green, Blue };
    struct Nested {
        int _n;
    };
    struct {
        NSUInteger _one : 1;
        NSUInteger _two : 1;
        NSUInteger _reserved : 30;
    } _flagSet;
    union {
        struct {
            char *_foo;
        } _foo;
        struct {
            void *_bar1;
            enum Color _bar2;
        } _bar;
    } _data;

    struct Nested *_nested;
}
@end
