/************************************************************************/
/* The MIT License (MIT) */
/* ===================== */

/* Copyright (c) 2017 PolySync Technologies, Inc.  All Rights Reserved. */

/* Permission is hereby granted, free of charge, to any person */
/* obtaining a copy of this software and associated documentation */
/* files (the “Software”), to deal in the Software without */
/* restriction, including without limitation the rights to use, */
/* copy, modify, merge, publish, distribute, sublicense, and/or sell */
/* copies of the Software, and to permit persons to whom the */
/* Software is furnished to do so, subject to the following */
/* conditions: */

/* The above copyright notice and this permission notice shall be */
/* included in all copies or substantial portions of the Software. */

/* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES */
/* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND */
/* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT */
/* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, */
/* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR */
/* OTHER DEALINGS IN THE SOFTWARE. */
/************************************************************************/

/**
 * @file oscc_interface.c
 * @brief OSCC interface source- The main command* functions and
 *        the update function should be called on at least a
 *        50ms period.  The expectation is that if there is not
 *        some kind of communication from the controller to the
 *        OSCC modules in that time, then the OSCC modules will
 *        disable and return control back to the driver.
 */


#include <stdio.h>
#include <stdlib.h>
#include <canlib.h>

#include "macros.h"
#include "control_protocol_can.h"


// *****************************************************
// static global types/macros
// *****************************************************

/**
 * @brief OSCC interface data - container for the various CAN 
 *        messages that are used to control the brakes, steering
 *        and throttle.  In addition, there are additional
 *        variables to store the CAN parameters, handle and
 *        channel.
 *  
 *        The entire structure is packed at the single byte
 *        level because of the need to send it on the wire to
 *        a receiver that is expecting a specific layout.
 */

#pragma pack(push)
#pragma pack(1)

struct oscc_interface_data_s
{
    ps_ctrl_brake_command_msg brake_cmd;
    ps_ctrl_throttle_command_msg throttle_cmd;
    ps_ctrl_steering_command_msg steering_cmd;

    canHandle can_handle;
    int can_channel;
};

// restore alignment
#pragma pack(pop)


// *****************************************************
// static global data
// *****************************************************

static struct oscc_interface_data_s oscc_interface_data;
static struct oscc_interface_data_s* oscc = NULL;


// *****************************************************
// static definitions
// *****************************************************

// *****************************************************
// Function:    oscc_can_write
// 
// Purpose:     Wrapper around the canWrite routine from the CAN library
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  id - ID of the CAN message ot send
//              msg - pointer to the buffer to send
//              dlc - size of the buffer
//
// *****************************************************
static int oscc_can_write( long id, void* msg, unsigned int dlc )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        canStatus status = canWrite( oscc->can_handle, id, msg, dlc, 0 );

        if ( status == canOK )
        {
            return_code = NOERR;
        }
    }
    return return_code;
}

// *****************************************************
// Function:    oscc_interface_init_can
// 
// Purpose:     Initialize the OSCC communication layer with known values
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  channel - for now, the CAN channel to use when interacting
//              with the OSCC modules
//
// *****************************************************
static int oscc_init_can( int channel )
{
    int return_code = ERROR;

    canHandle handle = canOpenChannel( channel, canOPEN_EXCLUSIVE );

    if ( handle >= 0 ) 
    {
        canBusOff( handle );

        canStatus status = canSetBusParams( handle, BAUD_500K,
                                            0, 0, 0, 0, 0 );
        if ( status == canOK )
        {
            status = canSetBusOutputControl( handle, canDRIVER_NORMAL );

            if ( status == canOK )
            {
                status = canBusOn( handle );

                if ( status == canOK )
                {
                    oscc_interface_data.can_handle = handle;
                    oscc_interface_data.can_channel = channel;
                    return_code = NOERR;
                }
                else
                {
                    printf( "canBusOn failed\n" );
                }
            }
            else
            {
                printf( "canSetBusOutputControl failed\n" );
            }
        }
        else
        {
            printf( "canSetBusParams failed\n" );
        }
    }
    else
    {
        printf( "canOpenChannel %d failed\n", channel );
    }
    return return_code;
}

// *****************************************************
// public definitions
// *****************************************************

// *****************************************************
// Function:    oscc_interface_set_defaults
// 
// Purpose:     Initialize the OSCC communication layer with known values
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  void
//
// *****************************************************
int oscc_interface_set_defaults( )
{
    int return_code = NOERR;

    ps_ctrl_brake_command_msg* brake = &oscc_interface_data.brake_cmd;

    brake->brake_on = 0;
    brake->clear = 0;
    brake->count = 0;
    brake->enabled = 0;
    brake->ignore = 0;
    brake->pedal_command = 0;

    ps_ctrl_throttle_command_msg* throttle = &oscc_interface_data.throttle_cmd;

    throttle->clear = 0;
    throttle->count = 0;
    throttle->enabled = 0;
    throttle->ignore = 0;
    throttle->pedal_command = 0;

    ps_ctrl_steering_command_msg* steering = &oscc_interface_data.steering_cmd;

    steering->clear = 0;
    steering->count = 0;
    steering->enabled = 0;
    steering->ignore = 0;
    steering->steering_wheel_angle_command = 0;
    steering->steering_wheel_max_velocity = 0;

    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_init
// 
// Purpose:     Initialize the OSCC interface - CAN communication
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  channel - integer value containing the CAN channel to openk
//
// *****************************************************
int oscc_interface_init( int channel )
{
    int return_code = ERROR;

    oscc_interface_set_defaults();

    return_code = oscc_init_can( channel );

    if ( return_code == NOERR )
    {
        oscc = &oscc_interface_data;
    }
    return ( return_code );
}

// *****************************************************
// Function:    oscc_interface_close
// 
// Purpose:     Release resources and close the interface
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  void
//
// *****************************************************
void oscc_interface_close( )
{
    if ( oscc != NULL )
    {
        canWriteSync( oscc->can_handle, 1000 );
        canClose( oscc->can_handle );
    }

    oscc = NULL;
}

// *****************************************************
// Function:    oscc_interface_enable
// 
// Purpose:     Cause the initialized interface to enable control of the
//              vehicle using the OSCC modules
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  void
//
// *****************************************************
int oscc_interface_enable( )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        return_code = NOERR;

        oscc->brake_cmd.enabled = 1;
        oscc->throttle_cmd.enabled = 1;
        oscc->steering_cmd.enabled = 1;
    }

    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_command_brakes
// 
// Purpose:     Send a CAN message to set the brakes to a commanded value
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  brake_setpoint - unsigned value
//              The value is range limited between 0 and 52428
//
// *****************************************************
int oscc_interface_command_brakes( unsigned int brake_setpoint )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        oscc->brake_cmd.pedal_command = ( uint16_t )brake_setpoint;

        return_code = oscc_can_write( PS_CTRL_MSG_ID_BRAKE_COMMAND,
                                      (void *) &oscc->brake_cmd,
                                      sizeof( ps_ctrl_brake_command_msg ) );
    }
    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_command_throttle
// 
// Purpose:     Send a CAN message to set the throttle to a commanded value
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  throttle_setpoint - unsigned value
//              The value is range limited between 0 and 19660
//
// *****************************************************
int oscc_interface_command_throttle( unsigned int throttle_setpoint )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        oscc->throttle_cmd.pedal_command = ( uint16_t )throttle_setpoint;

        return_code = oscc_can_write( PS_CTRL_THROTTLE_COMMAND_ID,
                                      (void *) &oscc->throttle_cmd,
                                      sizeof( ps_ctrl_throttle_command_msg ) );
    }

    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_command_steering
// 
// Purpose:     Send a CAN message to set the steering to a commanded value
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  angle - signed value: the steering angle in degrees
//              rate - unsigned value; the steering rate in degrees/sec
// 
//              angle is range limited between -4700 to 4700
//              rate is range limited between 20 to 254
//
// *****************************************************
int oscc_interface_command_steering( int angle, unsigned int rate )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        oscc->steering_cmd.steering_wheel_angle_command = ( int16_t )angle;
        oscc->steering_cmd.steering_wheel_max_velocity = ( uint16_t )rate;

        return_code = oscc_can_write( PS_CTRL_MSG_ID_STEERING_COMMAND,
                                      (void *) &oscc->steering_cmd,
                                      sizeof( ps_ctrl_steering_command_msg ) );
    }
    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_disable_brakes
// 
// Purpose:     Send a specific CAN message to set the brake enable value
//              to 0.  Included with this is a safe brake setting
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  void
//
// *****************************************************
int oscc_interface_disable_brakes( )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        oscc->brake_cmd.enabled = 0;

        printf( "brake: %d %d\n", oscc->brake_cmd.enabled,
                oscc->brake_cmd.pedal_command );

        return_code = oscc_interface_command_brakes( 0 );
    }
    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_disable_throttle
// 
// Purpose:     Send a specific CAN message to set the throttle enable value
//              to 0.  Included with this is a safe throttle setting
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  void
//
// *****************************************************
int oscc_interface_disable_throttle( )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        oscc->throttle_cmd.enabled = 0;

        printf( "throttle: %d %d\n", oscc->throttle_cmd.enabled,
                oscc->throttle_cmd.pedal_command );

        return_code = oscc_interface_command_throttle( 0 );
    }
    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_disable_steering
// 
// Purpose:     Send a specific CAN message to set the steering enable value
//              to 0.  Included with this is a safe steering angle and rate
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  void
//
// *****************************************************
int oscc_interface_disable_steering( )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        oscc->steering_cmd.enabled = 0;

        printf( "steering: %d %d %d\n",
                oscc->steering_cmd.enabled,
                oscc->steering_cmd.steering_wheel_angle_command,
                oscc->steering_cmd.steering_wheel_max_velocity );

        return_code = oscc_interface_command_steering( 0, 0 );
    }
    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_disable
// 
// Purpose:     Send a series of CAN messages to disable all of the OSCC
//              modules.  Mostly a wrapper around the existing specific
//              disable functions
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  void
//
// *****************************************************
int oscc_interface_disable( )
{
    int return_code = oscc_interface_disable_brakes( );

    if ( return_code == NOERR )
    {
        return_code = oscc_interface_disable_throttle( );

        if ( return_code == NOERR )
        {
            return_code = oscc_interface_disable_steering( );
        }
    }
    return ( return_code );
}


// *****************************************************
// Function:    oscc_interface_update_status
// 
// Purpose:     Read CAN messages from the OSCC modules and check for any
//              driver overrides
// 
// Returns:     int - ERROR or NOERR
// 
// Parameters:  override - pointer to an integer value that is filled out if
//              the OSCC modules indicate any override status
//
// *****************************************************
int oscc_interface_update_status( int* override )
{
    int return_code = ERROR;

    if ( oscc != NULL )
    {
        long can_id;
        unsigned int msg_dlc;
        unsigned int msg_flag;
        unsigned long tstamp; 
        unsigned char buffer[ 8 ];
        
        canStatus status = canRead( oscc->can_handle,
                                    &can_id,
                                    buffer,
                                    &msg_dlc,
                                    &msg_flag,
                                    &tstamp );

        if ( status == canOK )
        {
            return_code = NOERR;

            int local_override = 0;

            if ( can_id == PS_CTRL_MSG_ID_BRAKE_REPORT )
            {
                ps_ctrl_brake_report_msg* report =
                    ( ps_ctrl_brake_report_msg* )buffer;

                local_override = (int) report->override;
            }
            else if ( can_id == PS_CTRL_MSG_ID_THROTTLE_REPORT )
            {
                ps_ctrl_throttle_report_msg* report =
                    ( ps_ctrl_throttle_report_msg* )buffer;

                local_override = (int) report->override;
            }
            else if ( can_id == PS_CTRL_MSG_ID_STEERING_REPORT )
            {
                ps_ctrl_steering_report_msg* report =
                    ( ps_ctrl_steering_report_msg* )buffer;

                local_override = (int) report->override;
            }

            if ( ( *override ) == 0 )
            {
                *override = local_override;
            }
        }
        else if( ( status == canERR_NOMSG ) || ( status == canERR_TIMEOUT ) )
        {
            // Do nothing
            return_code = NOERR;
        }
        else
        {
            return_code = ERROR;
        }
    }
    return return_code;
}

