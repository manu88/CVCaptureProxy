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
#include <opencv2/core/types_c.h>

GBTimeMS interval  = 40;

typedef enum
{
    StreamRaw  = 0,
    StreamJPEG = 1
}StreamType;

typedef struct
{
    uint32_t frameSize ;
    uint16_t width;
    uint16_t height;
    uint8_t nChannels;
    
    uint8_t type; /* StreamType */
    
} StreamDescription;

#endif /* Commons_h */
