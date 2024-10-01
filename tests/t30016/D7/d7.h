#pragma once

#include "../D8/d8.h"
#include "../D9/d9.h"

@protocol D7 <D9>
- (void)withD8:(D8 *)d8;
@end