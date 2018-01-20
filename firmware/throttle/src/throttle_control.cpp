/**
 * @file throttle_control.cpp
 *
 */


#include <Arduino.h>
#include <stdint.h>

#include "can_protocols/global_can_protocol.h"
#include "can_protocols/throttle_can_protocol.h"
#include "communications.h"
#include "debug.h"
#include "dtc.h"
#include "globals.h"
#include "oscc_dac.h"
#include "oscc_eeprom.h"
#include "throttle_control.h"
#include "vehicles.h"


/*
 * @brief Number of consecutive faults that can occur when reading the
 *        sensors before control is disabled.
 *
 */
#define SENSOR_VALIDITY_CHECK_FAULT_COUNT ( 4 )


static void read_accelerator_position_sensor(
    accelerator_position_s * const value );


void check_for_operator_override( void )
{
    if ( g_throttle_control_state.enabled == true
        || g_throttle_control_state.operator_override == true )
    {
        accelerator_position_s accelerator_position;

        read_accelerator_position_sensor( &accelerator_position );

        uint32_t accelerator_position_average =
            (accelerator_position.low + accelerator_position.high) / 2;

        if ( accelerator_position_average >= g_eeprom_config.pedal_override_threshold )
        {
            disable_control( );

            DTC_SET(
                g_throttle_control_state.dtcs,
                OSCC_THROTTLE_DTC_OPERATOR_OVERRIDE );

            publish_fault_report( );

            g_throttle_control_state.operator_override = true;

            DEBUG_PRINTLN( "Operator override" );
        }
        else
        {
            DTC_CLEAR(
                g_throttle_control_state.dtcs,
                OSCC_THROTTLE_DTC_OPERATOR_OVERRIDE );

            g_throttle_control_state.operator_override = false;
        }
    }
}


void check_for_sensor_faults( void )
{
    if ( (g_throttle_control_state.enabled == true)
        || DTC_CHECK(g_throttle_control_state.dtcs, OSCC_THROTTLE_DTC_INVALID_SENSOR_VAL) )
    {
        static int fault_count = 0;

        accelerator_position_s accelerator_position;

        read_accelerator_position_sensor( &accelerator_position );

        // sensor pins tied to ground - a value of zero indicates disconnection
        if( (accelerator_position.high == 0)
            || (accelerator_position.low == 0) )
        {
            ++fault_count;

            if( fault_count >= SENSOR_VALIDITY_CHECK_FAULT_COUNT )
            {
                disable_control( );

                DTC_SET(
                    g_throttle_control_state.dtcs,
                    OSCC_THROTTLE_DTC_INVALID_SENSOR_VAL );

                publish_fault_report( );

                DEBUG_PRINTLN( "Bad value read from accelerator position sensor" );
            }
        }
        else
        {
            DTC_CLEAR(
                    g_throttle_control_state.dtcs,
                    OSCC_THROTTLE_DTC_INVALID_SENSOR_VAL );

            fault_count = 0;
        }
    }
}


void update_throttle(
    uint16_t spoof_command_high,
    uint16_t spoof_command_low )
{
    if ( g_throttle_control_state.enabled == true )
    {
        uint16_t spoof_high =
            constrain(
                spoof_command_high,
                g_eeprom_config.spoof_high_signal_range_min,
                g_eeprom_config.spoof_high_signal_range_max );

        uint16_t spoof_low =
            constrain(
                spoof_command_low,
                g_eeprom_config.spoof_low_signal_range_min,
                g_eeprom_config.spoof_low_signal_range_max );


        cli();
        g_dac.outputA( spoof_high );
        g_dac.outputB( spoof_low );
        sei();
    }
}


void enable_control( void )
{
    if( g_throttle_control_state.enabled == false
        && g_throttle_control_state.operator_override == false )
    {
        const uint16_t num_samples = 20;
        prevent_signal_discontinuity(
            g_dac,
            num_samples,
            PIN_ACCELERATOR_POSITION_SENSOR_LOW,
            PIN_ACCELERATOR_POSITION_SENSOR_HIGH );

        cli();
        digitalWrite( PIN_SPOOF_ENABLE, HIGH );
        sei();

        g_throttle_control_state.enabled = true;

        DEBUG_PRINTLN( "Control enabled" );
    }
}


void disable_control( void )
{
    if( g_throttle_control_state.enabled == true )
    {
        const uint16_t num_samples = 20;
        prevent_signal_discontinuity(
            g_dac,
            num_samples,
            PIN_ACCELERATOR_POSITION_SENSOR_LOW,
            PIN_ACCELERATOR_POSITION_SENSOR_HIGH );

        cli();
        digitalWrite( PIN_SPOOF_ENABLE, LOW );
        sei();

        g_throttle_control_state.enabled = false;

        DEBUG_PRINTLN( "Control disabled" );
    }
}


static void read_accelerator_position_sensor(
    accelerator_position_s * const value )
{
    cli();
    value->high = analogRead( PIN_ACCELERATOR_POSITION_SENSOR_HIGH );
    value->low = analogRead( PIN_ACCELERATOR_POSITION_SENSOR_LOW );
    sei();
}
