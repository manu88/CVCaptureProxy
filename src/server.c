//
//  main.c
//  testGBStreamOpenCV
//
//  Created by Manuel Deneu on 09/03/2017.
//  Copyright © 2017 Unlimited Development. All rights reserved.
//

#include <opencv2/core/core_c.h>

#ifdef OPENCV3
#include <opencv2/videoio/videoio_c.h>
#else
#include <opencv2/highgui/highgui_c.h>
#endif



#include <stdio.h>
#include <GBStreamService.h>
#include <GBRunLoop.h>
#include <stdio.h>


#include "Commons.h"

static CvCapture* camera = NULL;


static void serviceSourceCallback( GBRunLoopSource* source , GBRunLoopSourceNotification notification);

static GBSize streamServiceGetFrame( GBStreamService* stream , void* ptr)
{
    assert(stream);
    assert(ptr);
    printf("[streamServiceSendFrame] \n");
    
    IplImage* img = cvQueryFrame( camera );
    
    assert( (GBSize) img->imageSize == GBStreamServiceGetSize(stream));
    /*
    uint8_t* buf = ptr;
    for (GBIndex i = 0; i < GBStreamServiceGetSize(stream) ; ++i)
    {
        buf[i] = (uint8_t)i;
    }
     */
    memcpy( ptr, img->imageData, img->imageSize);
    
    return (GBSize) img->imageSize;
}

static void ServiceInvokeRequest( GBRunLoop*runLoop, void* data)
{
    GBStreamService* service = data;
    assert(service);
    if( service)
    {
        const GBSize numClients = GBStreamServiceGetNumClients(service);
        if( numClients > 0)
        {
            if( GBStreamServiceRequestSend(service))
            {
                GBRunLoopDispatchAfter(runLoop, ServiceInvokeRequest, service, interval);
            }
        }
        /*
        else if( numClients == 0 )
        {
            GBRunLoopStop(runLoop);
        }
         */
    }
    
}
static void serviceSourceCallback( GBRunLoopSource* source , GBRunLoopSourceNotification notification)
{
    assert(source);
    GBStreamService* service = GBRunLoopSourceGetUserContext(source);
    assert(service);
    switch (notification)
    {
        case GBRunLoopSourceCanRead:
        {
            printf("Server Request \n");
            GBSocket* clt =  GBSocketCreate(GBSocketGetType(source),NULL);
            if(GBSocketAccept(source, clt))
            {
                if(GBStreamServiceAddSource(service , clt))
                {
                    GBRelease(clt);
                }
            }
            
            GBRunLoopDispatchAsync(GBRunLoopSourceGetRunLoop(source),
                                   ServiceInvokeRequest,
                                   service
                                   );
            
        }
            break;
            
        default:
            printf("[Server] socket notif %i \n" , notification);
            break;
    }
    
    
}

BOOLEAN_RETURN uint8_t setupCamera( CvCapture* camera , double width , double height)
{
    const double nativeW = cvGetCaptureProperty( camera, CV_CAP_PROP_FRAME_WIDTH);
    const double nativeH = cvGetCaptureProperty( camera, CV_CAP_PROP_FRAME_HEIGHT);
    
    printf("Native Stream %f %f \n", nativeW , nativeH );
    
    
    cvSetCaptureProperty( camera, CV_CAP_PROP_FRAME_WIDTH ,width);
    cvSetCaptureProperty( camera, CV_CAP_PROP_FRAME_HEIGHT ,height);
    
    double ww = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH);
    
    while (ww != width)
    {
        
        cvSetCaptureProperty( camera, CV_CAP_PROP_FRAME_WIDTH ,width);
        cvSetCaptureProperty( camera, CV_CAP_PROP_FRAME_HEIGHT ,height);
        sleep(1);
        ww = cvGetCaptureProperty( camera, CV_CAP_PROP_FRAME_WIDTH);
    }
    
    const double hh = cvGetCaptureProperty( camera, CV_CAP_PROP_FRAME_HEIGHT);
    printf("Device stream w %f , h %f \n" , ww  ,hh);
    
    return hh == height;
}

int main(int argc, const char * argv[])
{
    printf("--- Start Service --- \n");
    
    camera = cvCreateCameraCapture(0);
    
    setupCamera(camera , 640 , 480);
    
    
    IplImage* img = cvQueryFrame( camera );
    printf("Image Size %i \n" , img->imageSize);
    printf("Channels %i \n" , img->nChannels);
    printf("Width %i \n", img->width);
    printf("height %i \n", img->height);
    printf("widthStep %i \n", img->widthStep);
    assert(img->nSize == sizeof(IplImage));
    
    
    
    GBStreamServiceCallbacks callbacks;
    callbacks.getFrame = streamServiceGetFrame;
    GBStreamService* service = GBStreamServiceInit( callbacks , frameSize );

    GBRunLoop* runLoop = GBRunLoopInit();

    GBSocket* listener = GBTCPSocketCreateListener(1230, 10, serviceSourceCallback);
    GBRunLoopSourceSetUserContext(listener, service);
    GBRunLoopAddSource(runLoop, listener);
    GBRunLoopRun( runLoop);
    
    GBRelease(listener);
    GBRelease(runLoop);
    GBRelease(service);
    
    cvReleaseCapture(&camera);
    
    return 0;
}
