//
//  Commons.h
//  testGBStreamOpenCV
//
//  Created by Manuel Deneu on 09/03/2017.
//  Copyright © 2017 Unlimited Development. All rights reserved.
//

#ifndef Commons_h
#define Commons_h

#include <GBTypes.h>

GBTimeMS interval  = 40;
GBSize   frameSize = 1228800;

typedef struct
{
    uint32_t frameSize ;
}StreamDescription;

#endif /* Commons_h */
