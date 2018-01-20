/**
 * @file init.cpp
 *
 */


#include <Arduino.h>

#include "can_protocols/fault_can_protocol.h"
#include "can_protocols/throttle_can_protocol.h"
#include "vehicles.h"
#include "communications.h"
#include "debug.h"
#include "globals.h"
#include "init.h"
#include "oscc_can.h"
#include "oscc_eeprom.h"


void init_globals( void )
{
    g_throttle_control_state.enabled = false;
    g_throttle_control_state.operator_override = false;
    g_throttle_control_state.dtcs = 0;
}


void init_devices( void )
{
    pinMode( PIN_DAC_CHIP_SELECT, OUTPUT );
    pinMode( PIN_ACCELERATOR_POSITION_SENSOR_HIGH, INPUT );
    pinMode( PIN_ACCELERATOR_POSITION_SENSOR_LOW, INPUT );
    pinMode( PIN_ACCELERATOR_POSITION_SPOOF_HIGH, INPUT );
    pinMode( PIN_ACCELERATOR_POSITION_SPOOF_LOW, INPUT );
    pinMode( PIN_SPOOF_ENABLE, OUTPUT );

    cli();
    digitalWrite( PIN_DAC_CHIP_SELECT, HIGH );
    digitalWrite( PIN_SPOOF_ENABLE, LOW );
    sei();
}


void init_communication_interfaces( void )
{
    #ifdef DEBUG
    init_serial( );
    #endif

    DEBUG_PRINT( "init Control CAN - " );
    init_can( g_control_can );

    // Filter CAN messages - accept if (CAN_ID & mask) == (filter & mask)
    // Set buffer 0 to filter only throttle module and global messages
    g_control_can.init_Mask( 0, 0, 0x7F0 ); // Filter for 0x0N0 to 0x0NF
    g_control_can.init_Filt( 0, 0, OSCC_THROTTLE_CAN_ID_INDEX );
    g_control_can.init_Filt( 1, 0, OSCC_FAULT_CAN_ID_INDEX );
    // Accept only CAN Disable when buffer overflow occurs in buffer 0
    g_control_can.init_Mask( 1, 0, 0x7FF ); // Filter for one CAN ID
    g_control_can.init_Filt( 2, 1, OSCC_THROTTLE_DISABLE_CAN_ID );
}


void init_config( void )
{
    uint16_t eeprom_magic = oscc_eeprom_read_u16( OSCC_CONFIG_U16_EEPROM_MAGIC );

    if ( eeprom_magic != OSCC_EEPROM_MAGIC )
    {
        DEBUG_PRINT( "Writing config defaults to EEPROM");

        oscc_eeprom_write_u16( OSCC_CONFIG_U16_THROTTLE_SPOOF_LOW_SIGNAL_RANGE_MIN, THROTTLE_SPOOF_LOW_SIGNAL_RANGE_MIN );
        oscc_eeprom_write_u16( OSCC_CONFIG_U16_THROTTLE_SPOOF_LOW_SIGNAL_RANGE_MAX, THROTTLE_SPOOF_LOW_SIGNAL_RANGE_MAX );
        oscc_eeprom_write_u16( OSCC_CONFIG_U16_THROTTLE_SPOOF_HIGH_SIGNAL_RANGE_MIN, THROTTLE_SPOOF_HIGH_SIGNAL_RANGE_MIN );
        oscc_eeprom_write_u16( OSCC_CONFIG_U16_THROTTLE_SPOOF_HIGH_SIGNAL_RANGE_MAX, THROTTLE_SPOOF_HIGH_SIGNAL_RANGE_MAX );
        oscc_eeprom_write_u16( OSCC_CONFIG_U16_THROTTLE_PEDAL_OVERRIDE_THRESHOLD, THROTTLE_PEDAL_OVERRIDE_THRESHOLD );
        oscc_eeprom_write_u16( OSCC_CONFIG_U16_THROTTLE_FAULT_CHECK_FREQUENCY_IN_HZ, THROTTLE_FAULT_CHECK_FREQUENCY_IN_HZ );
        oscc_eeprom_write_u16( OSCC_CONFIG_U16_THROTTLE_REPORT_PUBLISH_FREQUENCY_IN_HZ, OSCC_THROTTLE_REPORT_PUBLISH_FREQUENCY_IN_HZ );

        oscc_eeprom_write_u16( OSCC_CONFIG_U16_EEPROM_MAGIC, OSCC_EEPROM_MAGIC );
    }

    g_eeprom_config.spoof_low_signal_range_min = oscc_eeprom_read_u16( OSCC_CONFIG_U16_THROTTLE_SPOOF_LOW_SIGNAL_RANGE_MIN );
    g_eeprom_config.spoof_low_signal_range_max = oscc_eeprom_read_u16( OSCC_CONFIG_U16_THROTTLE_SPOOF_LOW_SIGNAL_RANGE_MAX );
    g_eeprom_config.spoof_high_signal_range_min = oscc_eeprom_read_u16( OSCC_CONFIG_U16_THROTTLE_SPOOF_HIGH_SIGNAL_RANGE_MIN );
    g_eeprom_config.spoof_high_signal_range_max = oscc_eeprom_read_u16( OSCC_CONFIG_U16_THROTTLE_SPOOF_HIGH_SIGNAL_RANGE_MAX );
    g_eeprom_config.pedal_override_threshold = oscc_eeprom_read_u16( OSCC_CONFIG_U16_THROTTLE_PEDAL_OVERRIDE_THRESHOLD );
    g_eeprom_config.fault_check_frequency_in_hz = oscc_eeprom_read_u16( OSCC_CONFIG_U16_THROTTLE_FAULT_CHECK_FREQUENCY_IN_HZ );
    g_eeprom_config.report_publish_frequency_in_hz = oscc_eeprom_read_u16( OSCC_CONFIG_U16_THROTTLE_REPORT_PUBLISH_FREQUENCY_IN_HZ );
}
