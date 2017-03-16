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
#include <GBFDSource.h>
#include "Commons.h"




static const StreamType imgType =  StreamJPEG;
static int jpegQual = 50;

/**/
static IplImage* img = NULL;
static GBSize frameSize = GBSizeInvalid;

static void ServiceInvokeRequest( GBRunLoop*runLoop, void* data);
static void ServiceInvokeFirstRequest( GBRunLoop*runLoop, void* data);

static CvCapture* camera = NULL;


static void serviceSourceCallback( GBRunLoopSource* source , GBRunLoopSourceNotification notification);


static GBSize streamServiceGetData ( GBStreamService* stream , void* ptr)
{
    //img = cvQueryFrame( camera );
    
    DEBUG_ASSERT(img);
    
    StreamDescription *desc = ptr;
    desc->frameSize = (uint32_t) frameSize;
    desc->width     = img->width;
    desc->height    = img->height;
    desc->nChannels = img->nChannels;
    desc->nChannels = 3;
    desc->type = imgType;
    
    return sizeof(StreamDescription);
    
}

static CvMat* encodeImage( IplImage* img , int quality)
{
    const int params[] = {CV_IMWRITE_JPEG_QUALITY , quality , 0};
    return cvEncodeImage(".jpg", img, params);
}

static GBSize streamServiceGetFrame( GBStreamService* stream , void* ptr)
{
    assert(stream);
    assert(ptr);
    
    img = cvQueryFrame( camera );
    
    if( imgType == StreamJPEG)
    {
        CvMat* encoded = encodeImage(img, jpegQual );
        const size_t imgSize = encoded->step*encoded->rows;
        
        memcpy(ptr, (const char*) encoded->data.ptr, imgSize);
        
        //cvReleaseMat(&encoded);
        return (GBSize) imgSize;
        
    }
    else if( imgType == StreamRaw)
    {
        memcpy( ptr, img->imageData, img->imageSize);
        
        return (GBSize) img->imageSize;
    }
    
    
    return GBSizeInvalid;
}



static void ServiceInvokeFirstRequest( GBRunLoop*runLoop, void* data)
{
    GBStreamService* service = data;
    assert(service);
    if( service == NULL)
    {
        return;
    }
    
    if( GBStreamServiceRequestDataSend(service))
    {
        GBRunLoopDispatchAfter(runLoop, ServiceInvokeRequest, service, interval);
    }
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
            if( GBStreamServiceRequestFrameSend(service))
            {
                GBRunLoopDispatchAfter(runLoop, ServiceInvokeRequest, service, interval);
            }
            else
            {
                printf("[GBStreamServiceRequestFrameSend] Error\n");
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
                                   ServiceInvokeFirstRequest,
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
   
    if (width == nativeW && height == nativeH)
        return 1;
    
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

static void inputCallback( GBRunLoopSource* source , GBRunLoopSourceNotification notification)
{
    static char buf[16];
    memset(buf, 0, 16);
    
    const GBSize size =  GBRunLoopSourceRead(source, buf, 16 );
    
    if( size)
    {
        
        const GBString* raw = GBStringInitWithFormat("%s" , buf);
        const GBString* cmd = GBStringByRemovingChars(raw, "\n");
    
        int qual = -1;
        if( GBStringEqualsCStr(cmd, "q"))
        {
            GBRunLoopStop( GBRunLoopSourceGetRunLoop(source));
        }
        if( GBStringScan(cmd, "%i" , &qual) == 1)
        {
            printf("Set Quality to %i\n" , qual);
            jpegQual  = qual;
        }
        
        //printf("'%s'\n" , GBStringGetCStr(cmd) );
        GBRelease(cmd);
        GBRelease(raw);
    }
    
}

int main(int argc, const char * argv[])
{
    printf("--- Start Service --- \n");
    
    camera = cvCreateCameraCapture(0);
    
    setupCamera(camera , 640 , 480);
    
    
    img = cvQueryFrame( camera );
    
    
    
    if( imgType == StreamJPEG)
    {
        printf("USING JPEG ENCODING\n");
        /*const CvMat* encoded = encodeImage(img, 50);
        frameSize = encoded->step*encoded->rows;*/
        frameSize = img->imageSize;
    }
    else if( imgType == StreamRaw)
    {
        frameSize = img->imageSize;
    }
    printf("Image Size %zi \n" , frameSize);
    printf("Channels %i \n" , img->nChannels);
    printf("Width %i \n", img->width);
    printf("height %i \n", img->height);
    printf("widthStep %i \n", img->widthStep);
    
    
    assert(img->nSize == sizeof(IplImage));
    
    
    
    GBStreamServiceCallbacks callbacks;
    callbacks.getFrame = streamServiceGetFrame;
    callbacks.getData = streamServiceGetData;
    GBStreamService* service = GBStreamServiceInit( callbacks , frameSize );

    GBRunLoop* runLoop = GBRunLoopInit();

    GBFDSource* input = GBFDSourceInitWithFD( fileno( stdin), inputCallback);
    
    GBSocket* listener = GBTCPSocketCreateListener(1230, 10, serviceSourceCallback);
    GBRunLoopSourceSetUserContext(listener, service);
    GBRunLoopAddSource(runLoop, listener);
    GBRunLoopAddSource(runLoop, input);
    GBRunLoopRun( runLoop);
    
    GBRelease(listener);
    GBRelease(runLoop);
    GBRelease(service);
    GBRelease(input);
    cvReleaseCapture(&camera);
    
    
    GBObjectIntrospection(1);
    return 0;
}
