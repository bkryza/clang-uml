#include "../D1/d1.h"
#include "../D2/d2.h"
#include "../D3/d3.h"
#include "../D4/d4.h"
#include "../D5/d5.h"
#include "../D6/d6.h"
#include "../D7/d7.h"
#include "../D10/d10.h"
#include "../D11/d11.h"
#include "../D12/d12.h"

@interface t30016_D : NSObject<D6> {
    struct D4 *d4_;
}

@property (nonatomic, assign) D1 *d1_;
@property (nonatomic, assign) struct D3 *d3_;
@property (class, strong, nonatomic) D12 *d12;

@property (weak) id <D7> d7;

+ (void)fromD11:(D11 *)d11;
- (void)withD2:(D2 *)d2;

- (D5*)getD5;

@end