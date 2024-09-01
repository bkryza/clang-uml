#import <Foundation/Foundation.h>

@interface It00085 : NSObject {
    int _defaultMember;

    @public
    NSString *_publicMember;

    @protected
    NSString *_protectedMember;

    @private
    NSString *_privateMember;
}

@property (class, nonatomic, assign) NSInteger sharedCounter;

@property (nonatomic, strong) NSString *name;
@property (nonatomic, assign) NSInteger value;

+ (void)incrementSharedCounter;
+ (NSInteger)currentSharedCounter;
+ (instancetype)sharedInstance;

- (instancetype)initWithName:(NSString *)name value:(NSInteger)value;
- (void)displayDetails;
- (NSInteger)addValue:(NSInteger)otherValue;
- (NSInteger)multiplyValueBy:(NSInteger)multiplier andAdd:(NSInteger)addition;
- (void)resetValue;

- (void)performOperationWithBlock:(void (^)(NSInteger result))block;

@end