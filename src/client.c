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

static void ClientDidReceiveFrame( GBStreamClient* client);

static void ClientDidReceiveFrame( GBStreamClient* client)
{
    assert(client);
    const char* frame = GBStreamClientGetBuff(client);
    
    assert( frame);
    
    
    memcpy(img.imageData, frame, GBStreamClientGetSize(client) );
    
    
    cvShowImage("Video", &img);
    if(cvWaitKey((int) interval) > 0)
    {
        printf("[Client] Stop request \n");
        GBRunLoopStop( GBRunLoopSourceGetRunLoop( GBStreamClientGetSocket(client)));
    }
    
}




int main(int argc, const char * argv[])
{
    printf("--- Start Client --- \n");
    
    memset(&img, 0, sizeof(IplImage));
    
    const int magicCoefToFindOut = 4;
    img.nChannels = 3;
    img.width     = 640;
    img.height    = 480;
    img.widthStep = 640 * magicCoefToFindOut;// 2560 @ w = 640;
    img.nSize = sizeof(IplImage);// 144;
    img.imageData = malloc(frameSize);
    
    GBStreamClientCallbacks callbacks;
    callbacks.didReceiveFrame = ClientDidReceiveFrame;
    GBStreamClient* client = GBStreamClientInit( callbacks, frameSize);
    
    assert(client);
    
    cvNamedWindow("Video",1); // create window
    
    GBRunLoop* runLoop = GBRunLoopInit();
    
    GBSocket* clientSocket = GBSocketCreate( GBSocketTypeTCP, NULL);
    
    GBRunLoopSourceSetUserContext(clientSocket, client);
    if(GBTCPSocketConnectToEndPoint( clientSocket, GBSTR("127.0.0.1"), 1230))
    {
        printf("Connected To StreamService \n");
        GBStreamClientSetSocket(client , clientSocket);
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
