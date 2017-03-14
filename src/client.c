//
//  main.c
//  testGBStreamOpenCV
//
//  Created by Manuel Deneu on 09/03/2017.
//  Copyright © 2017 Unlimited Development. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <GBThread.h>
#include <GBRunLoop.h>
#include <GBStreamClient.h>
#include <GBSocket.h>

#include <opencv2/core/core_c.h> // IplImage def
#include <opencv2/highgui/highgui_c.h>
#include "Commons.h"


static IplImage img;
StreamType streamType = 0;

static void ClientDidReceiveFrame( GBStreamClient* client);
static void ClientDidReceiveData( GBStreamClient* client , const void* msg );
static void ClientDidReceiveNotification( GBStreamClient* client , GBRunLoopSourceNotification notification );

static void ClientDidReceiveData( GBStreamClient* client , const void* msg )
{
    assert(client);
    printf("Got Stream Description \n");
    
    
    const StreamDescription* desc = msg;
    DEBUG_ASSERT(desc);
    
    if(desc->type == StreamRaw)
    {
        streamType = desc->type;
        
        printf("Raw Encoding \n");
    }
    else if(desc->type == StreamJPEG)
    {
        printf("JPEG Encoding \n");
    }
    const int magicCoefToFindOut = 4;
    img.nChannels = desc->nChannels;
    img.width     = desc->width;
    img.height    = desc->height;
    
    img.widthStep = desc->width * magicCoefToFindOut;// 2560 @ w = 640;
    img.nSize = sizeof(IplImage);// 144;
    img.imageData = malloc( desc->frameSize );
    img.imageSize = desc->frameSize;
    
    const GBSize frameSize = (GBSize)img.imageSize;
    
    printf("Image wStep %i \n" , img.widthStep);
    printf("Frame Size %zi \n" , frameSize);
    
    GBStreamClientSetFrameSize(client,frameSize  );
     
    
    
}
static void ClientDidReceiveFrame( GBStreamClient* client)
{
    assert(client);
    const char* frame = GBStreamClientGetBuff(client);
    
    assert( frame);
    
    
    
    IplImage* _img = NULL;
    
    if( streamType == StreamJPEG)
    {
        _img = cvDecodeImage(frame, CV_LOAD_IMAGE_COLOR);
    }
    else if( streamType == StreamRaw)
    {
        memcpy(img.imageData, frame, GBStreamClientGetFrameSize(client) );
        _img = &img;
    }

    
    
    cvShowImage("Video", _img);
    if(cvWaitKey((int) interval) > 0)
    {
        printf("[Client] Stop request \n");
        GBRunLoopStop( GBRunLoopSourceGetRunLoop( GBStreamClientGetSource(client)));
    }
    
}


static void ClientDidReceiveNotification( GBStreamClient* client , GBRunLoopSourceNotification notification )
{
    printf("Client Got notification %i \n" , notification);
    
    if( notification == GBRunLoopSourceRemovedFromRunLoop)
    {
        GBRunLoopStop( GBRunLoopSourceGetRunLoop( GBStreamClientGetSource(client)));
        
    }
}



int main(int argc, const char * argv[])
{
    printf("--- Start Client --- \n");
    
    memset(&img, 0, sizeof(IplImage));
    
    
    
    GBStreamClientCallbacks callbacks;
    callbacks.didReceiveFrame        = ClientDidReceiveFrame;
    callbacks.didReceiveData         = ClientDidReceiveData;
    callbacks.didReceiveNotification = ClientDidReceiveNotification;
    GBStreamClient* client = GBStreamClientInit( callbacks );
    
    assert(client);
    
    cvNamedWindow("Video",1); // create window
    
    GBRunLoop* runLoop = GBRunLoopInit();
    
    GBSocket* clientSocket = GBSocketCreate( GBSocketTypeTCP, NULL);
    
    GBRunLoopSourceSetUserContext(clientSocket, client);

    const char *ip = argc>=2? argv[1] : "127.0.0.1";

    printf("Connect to '%s' \n" , ip);

    if(GBTCPSocketConnectToEndPoint( clientSocket, GBSTR( ip), 1230))
    {
        printf("Connected To StreamService \n");
        GBStreamClientSetSource(client , clientSocket);
        GBRunLoopAddSource(runLoop, clientSocket);
        GBRunLoopRun(runLoop);
    }
    else
    {
        printf("Error : Unable to connect to service \n");
    }
    
    GBRelease(runLoop);
    GBRelease(clientSocket);
    GBRelease(client);
    
    free(img.imageData);
    
    GBObjectIntrospection(1);
    return 0;
}
