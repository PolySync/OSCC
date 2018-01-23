/**
 * @file brake_control.cpp
 *
 */


#include <Arduino.h>
#include <stdint.h>

#include "can_protocols/brake_can_protocol.h"
#include "brake_control.h"
#include "communications.h"
#include "debug.h"
#include "dtc.h"
#include "globals.h"
#include "status.h"
#include "oscc_dac.h"
#include "vehicles.h"

static unsigned long TIME_LAST_SEEN = 0;
const unsigned long DELAY = 100; //ms of delay to ensure override still requested

/*
 * @brief Number of consecutive faults that can occur when reading the
 *        sensors before control is disabled.
 *
 */
#define SENSOR_VALIDITY_CHECK_FAULT_COUNT ( 4 )


static void read_brake_pedal_position_sensor(
    brake_pedal_position_s * const value );

static uint8_t check_brake_pedal_position_data(
    brake_pedal_position_s * const value );

void check_for_operator_override( void )
{
    if ( g_brake_control_state.enabled == true
        || g_brake_control_state.operator_override == true )
    {
        brake_pedal_position_s brake_pedal_position;

        read_brake_pedal_position_sensor( &brake_pedal_position );

        uint32_t brake_pedal_position_average =
            (brake_pedal_position.A + brake_pedal_position.B) / 2;

        if ( brake_pedal_position_average >= BRAKE_PEDAL_OVERRIDE_THRESHOLD )
        {
            unsigned long current_time = millis();

            if ( TIME_LAST_SEEN == 0 )
            {
              TIME_LAST_SEEN = millis();
            }
            else if ( current_time - TIME_LAST_SEEN > DELAY )
            {
                disable_control( );

                status_setGreenLed(0);
                status_setRedLed(1);

                DTC_SET(
                    g_brake_control_state.dtcs,
                    OSCC_BRAKE_DTC_OPERATOR_OVERRIDE );

                publish_fault_report( );

                g_brake_control_state.operator_override = true;

                DEBUG_PRINTLN( "Operator override" );
            }
        }
        else
        {
            TIME_LAST_SEEN = 0; //Start over no deviation

            DTC_CLEAR(
                g_brake_control_state.dtcs,
                OSCC_BRAKE_DTC_OPERATOR_OVERRIDE );

            g_brake_control_state.operator_override = false;
        }
    }
}


void check_for_sensor_faults( void )
{
    if ( (g_brake_control_state.enabled == true)
        || DTC_CHECK(g_brake_control_state.dtcs, OSCC_BRAKE_DTC_INVALID_SENSOR_VAL) )
    {
        static int fault_count = 0;

        brake_pedal_position_s brake_pedal_position;

        read_brake_pedal_position_sensor( &brake_pedal_position );

        if(check_brake_pedal_position_data( &brake_pedal_position ))
        {
            ++fault_count;

            if( fault_count >= SENSOR_VALIDITY_CHECK_FAULT_COUNT )
            {
                disable_control( );

                status_setGreenLed(0);
                status_setRedLed(1);

                DTC_SET(
                    g_brake_control_state.dtcs,
                    OSCC_BRAKE_DTC_INVALID_SENSOR_VAL );

                publish_fault_report( );

                DEBUG_PRINTLN( "Bad value read from brake pedal position sensor" );
            }
        }
        else
        {
            DTC_CLEAR(
                    g_brake_control_state.dtcs,
                    OSCC_BRAKE_DTC_INVALID_SENSOR_VAL );

            fault_count = 0;
        }
    }
}


void update_brake(
    uint16_t spoof_command_A,
    uint16_t spoof_command_B )
{
    if ( g_brake_control_state.enabled == true )
    {
        status_setGreenLed(0);

        uint16_t spoof_A =
            constrain(
                spoof_command_A,
                BRAKE_SPOOF_A_SIGNAL_RANGE_MIN,
                BRAKE_SPOOF_A_SIGNAL_RANGE_MAX );

        uint16_t spoof_B =
            constrain(
                spoof_command_B,
                BRAKE_SPOOF_B_SIGNAL_RANGE_MIN,
                BRAKE_SPOOF_B_SIGNAL_RANGE_MAX );

        if( (spoof_A > BRAKE_LIGHT_SPOOF_A_THRESHOLD)
            || (spoof_B > BRAKE_LIGHT_SPOOF_B_THRESHOLD) )
        {
            cli();
            digitalWrite(PIN_BRAKE_LIGHT_ENABLE, HIGH);
            sei();
        }
        else
        {
            cli();
            digitalWrite(PIN_BRAKE_LIGHT_ENABLE, LOW);
            sei();
        }

        cli();
        g_dac.outputA( spoof_A );
        g_dac.outputB( spoof_B );
        sei();

        status_setGreenLed(1);
    }
}


void enable_control( void )
{
    if( g_brake_control_state.enabled == false
        && g_brake_control_state.operator_override == false )
    {
        const uint16_t num_samples = 20;
        prevent_signal_discontinuity(
            g_dac,
            num_samples,
            PIN_BRAKE_PEDAL_POSITION_SENSOR_A,
            PIN_BRAKE_PEDAL_POSITION_SENSOR_B );

        cli();
        digitalWrite( PIN_SPOOF_ENABLE, HIGH );
        sei();

        g_brake_command_timeout = false;
        g_brake_control_state.enabled = true;

        DEBUG_PRINTLN( "Control enabled" );
    }
}


void disable_control( void )
{
    if( g_brake_control_state.enabled == true )
    {
        const uint16_t num_samples = 20;
        prevent_signal_discontinuity(
            g_dac,
            num_samples,
            PIN_BRAKE_PEDAL_POSITION_SENSOR_A,
            PIN_BRAKE_PEDAL_POSITION_SENSOR_B );

        cli();
        digitalWrite( PIN_SPOOF_ENABLE, LOW );
        sei();

        g_brake_command_timeout = false;
        g_brake_control_state.enabled = false;

        DEBUG_PRINTLN( "Control disabled" );
    }
}


static void read_brake_pedal_position_sensor(
    brake_pedal_position_s * const value )
{
    cli();
    value->A = analogRead( PIN_BRAKE_PEDAL_POSITION_SENSOR_A );
    value->B = analogRead( PIN_BRAKE_PEDAL_POSITION_SENSOR_B );
    sei();
}

uint8_t check_brake_pedal_position_data(
    brake_pedal_position_s * const value )
{
    uint8_t error_count = 0;
    if( value->A > (BRAKE_SPOOF_A_SIGNAL_RANGE_MAX >> 2))
        error_count++;
    if( value->A < (BRAKE_SPOOF_A_SIGNAL_RANGE_MIN >> 2))
        error_count++;

    if( value->B > (BRAKE_SPOOF_B_SIGNAL_RANGE_MAX >> 2))
        error_count++;
    if( value->B < (BRAKE_SPOOF_B_SIGNAL_RANGE_MIN >> 2))
        error_count++;

    return 0;

    return( error_count );

}
