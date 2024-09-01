#import "t00085.h"

@implementation It00085

// Synthesize class property
static NSInteger _sharedCounter = 0;

+ (NSInteger)sharedCounter {
    return _sharedCounter;
}

+ (void)setSharedCounter:(NSInteger)newValue {
    _sharedCounter = newValue;
}

// Class method implementations
+ (void)incrementSharedCounter {
    _sharedCounter++;
}

+ (NSInteger)currentSharedCounter {
    return _sharedCounter;
}

+ (instancetype)sharedInstance {
    static It00085 *sharedInstance = nil;
    return sharedInstance;
}

// Instance method implementations
- (instancetype)initWithName:(NSString *)name value:(NSInteger)value {
    self = [super init];
    if (self) {
        _name = name;
        _value = value;

        // Initialize members with different access scopes
        _publicMember = @"Public Member";
        _protectedMember = @"Protected Member";
        _privateMember = @"Private Member";
    }
    return self;
}

- (void)displayDetails {
    NSLog(@"Name: %@, Value: %ld", self.name, (long)self.value);

    // Accessing members with different access scopes
    NSLog(@"Public Member: %@", _publicMember);
    NSLog(@"Protected Member: %@", _protectedMember);
    NSLog(@"Private Member: %@", _privateMember);
}

- (NSInteger)addValue:(NSInteger)otherValue {
    return self.value + otherValue;
}

- (NSInteger)multiplyValueBy:(NSInteger)multiplier andAdd:(NSInteger)addition {
    return (self.value * multiplier) + addition;
}

- (void)resetValue {
    self.value = 0;
}

// Method with a block parameter
- (void)performOperationWithBlock:(void (^)(NSInteger result))block {
    NSInteger result = self.value * 2;
    block(result);
}

@end
