/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */
/*

  Author(s): Simon Leonard
  Created on: Jan 11 2012

  (C) Copyright 2008 Johns Hopkins University (JHU), All Rights
  Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <saw3Dconnexion/osa3Dconnexion.h>

#include <cisstCommon/cmnAssert.h>
#include <cisstCommon/cmnLogger.h>

#if (CISST_OS == CISST_LINUX)
#include <string.h>           // for memset
#include <fcntl.h>            // for open/close read/write O_RDWR
#include <libgen.h>           // for basename/dirname 
#include <dirent.h>           // for opendir/closedir
#include <linux/joystick.h>   // for joystick event
#else
#endif

// OS dependent structure
struct osa3Dconnexion::Internals{

#if (CISST_OS == CISST_LINUX)
    std::string inputfn;     // input filename (i.e. /dev/input/js?)
    std::string eventfn;     // event filename (i.e. /dev/input/event?)
    int inputfd;             // file descriptor for input device (data)
    int eventfd;             // file descriptor for event device (LED)
    long long data[6];       // state of the device (events are per axis)
#else
#endif

};


osa3Dconnexion::osa3Dconnexion() :
    internals( NULL ) { 
    
    // allocate the internal structure
    try{ internals = new osa3Dconnexion::Internals; }
    catch( std::bad_alloc& )
        { CMN_LOG_RUN_ERROR << "Failed to allocate internals" << std::endl; }

    // initialize the structure
#if (CISST_OS == CISST_LINUX)

    internals->inputfd = -1;
    internals->eventfd = -1;
    for( size_t i=0; i<6; i++ ){ internals->data[i] = 0; }

#else
#endif

}

osa3Dconnexion::~osa3Dconnexion(){

    if( internals != NULL ){
        
#if (CISST_OS == CISST_LINUX)

        Close();

#else
#endif

        delete internals;
        
    }

}

osa3Dconnexion::Errno osa3Dconnexion::Open( const std::string& filename ){

    if( internals != NULL ){

#if (CISST_OS == CISST_LINUX)

        // only open if device is closed
        if( internals->inputfd == -1 ){

            if( !filename.empty() ){

                // try to open the /dev/input/js?
                internals->inputfn = filename;
                internals->inputfd = open( filename.c_str(), O_RDONLY );
                if( internals->inputfd == -1 ){
                    CMN_LOG_RUN_ERROR << "Failed to open " << filename << std::endl;
                    return osa3Dconnexion::EFAILURE;
                }
                
                // Get the /dev/input/js? dirname and basename
                char devinputjs[128];
                strcpy( devinputjs, filename.c_str() );
                const char* js = basename( devinputjs );
                const char* devinput = dirname( devinputjs );

                // find the corresponding event file in /sys/class/input
                char sysfs[128];
                for( size_t i=0; i<99; i++ ){
                    memset( sysfs, 0, sizeof( sysfs ) );
                    sprintf( sysfs, "/sys/class/input/%s/device/event%d", js, i );
                    DIR* dir = opendir( sysfs );
                    if( dir != NULL ){
                        closedir( dir );
                        break;
                    }
                }

                // set the corresponding file /dev/input/event
                const char* event = basename( sysfs );
                char devinputevent[128];
                sprintf( devinputevent, "%s/%s", devinput, event );

                // try to open /dev/input/event (not critical)
                internals->eventfn = std::string( devinputevent );
                internals->eventfd = open( devinputevent, O_RDWR);
                if( internals->inputfd != -1 )
                    { LEDOn(); }
                else
                    { CMN_LOG_RUN_ERROR << "Failed to open " << devinputevent << std::endl; }
                
            }

        }

#else
#endif

    }

    return osa3Dconnexion::ESUCCESS;
}

osa3Dconnexion::Errno osa3Dconnexion::Close(){

    if( internals != NULL ){
        
#if (CISST_OS == CISST_LINUX)
        
        // close the device if not already closed
        if( internals->inputfd != -1 ){
            if( close( internals->inputfd ) == -1 )
                { CMN_LOG_RUN_ERROR << "Failed to close input." << std::endl; }
        }
    
        // close the device if not already closed
        if( internals->eventfd != -1 ){
            LEDOff();
            if( close( internals->eventfd ) == -1 )
                { CMN_LOG_RUN_ERROR << "Failed to close event." << std::endl; }
        }
        
#else
#endif

    }

    return osa3Dconnexion::ESUCCESS;

}

osa3Dconnexion::Errno osa3Dconnexion::LEDOn(){

    if( internals != NULL ){

#if (CISST_OS == CISST_LINUX)

        // event device must be opened
        if( internals->eventfd != -1) {
            struct input_event ev;

            memset(&ev, 0, sizeof ev);
            ev.type = EV_LED;
            ev.code = LED_MISC;
            ev.value = 1;
        
            if( write( internals->eventfd, &ev, sizeof(ev) ) == -1){
                perror("");
                CMN_LOG_RUN_ERROR << "Failed to write event" << std::endl;
                return osa3Dconnexion::EFAILURE;
            }
            
        }
        else{
            CMN_LOG_RUN_ERROR << "Event device not opened" << std::endl;
            return osa3Dconnexion::EFAILURE;
        }
    
#else
#endif

    }

    return osa3Dconnexion::ESUCCESS;

}

osa3Dconnexion::Errno osa3Dconnexion::LEDOff(){

    if( internals != NULL ){

#if (CISST_OS == CISST_LINUX)

        // event device must be opened
        if( internals->eventfd != -1) {
            struct input_event ev;

            memset(&ev, 0, sizeof ev);
            ev.type = EV_LED;
            ev.code = LED_MISC;
            ev.value = 0;
        
            if( write( internals->eventfd, &ev, sizeof(ev) ) == -1){
                CMN_LOG_RUN_ERROR << "Failed to write event" << std::endl;
                return osa3Dconnexion::EFAILURE;
            }
            
        }
        else{
            CMN_LOG_RUN_ERROR << "Event device not opened" << std::endl;
            return osa3Dconnexion::EFAILURE;
        }
    
#else
#endif

    }

    return osa3Dconnexion::ESUCCESS;

}

osa3Dconnexion::Event osa3Dconnexion::WaitForEvent(){

    osa3Dconnexion::Event event;
    event.type = osa3Dconnexion::Event::UNKNOWN;

    if( internals != NULL ){

#if (CISST_OS == CISST_LINUX)
        
        // check the file descriptor
        if( internals->inputfd != -1 ){
            
            // read the event
            struct js_event e; 
            if( read( internals->inputfd, &e, sizeof(struct js_event) ) != -1 ){
                
                // copty the timestamp
                event.timestamp = e.time;
                
                // Button event
                if( e.type == JS_EVENT_BUTTON ){
                    
                    // Find which button event
                    if( e.value == 0 )
                        { event.type = osa3Dconnexion::Event::BUTTON_RELEASED; }
                    if( e.value == 1 )
                        { event.type = osa3Dconnexion::Event::BUTTON_PRESSED; }
                    
                    // Find which button
                    if( e.number == 0 )
                        { event.button = osa3Dconnexion::Event::BUTTON1; }
                    if( e.number == 1 )
                        { event.button = osa3Dconnexion::Event::BUTTON2; }
                    
                }
                
                // Axis event
                if( e.type == JS_EVENT_AXIS ){
                    
                    event.type = osa3Dconnexion::Event::MOTION;
                    // accumulate the axis value to the internals
                    internals->data[ e.number ] += e.value; 
                    // copy all the axis to the event
                    for( size_t i=0; i<6; i++ )
                        { event.data[i] = internals->data[ i ]; }

                }
                
            }
            else { CMN_LOG_RUN_ERROR << "Failed to read device" << std::endl; }
        }
        else { CMN_LOG_RUN_ERROR << "Invalid device" << std::endl; }
#else
#endif

    }

    return event;
}

