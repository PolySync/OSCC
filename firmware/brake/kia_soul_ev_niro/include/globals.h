/**
 * @file globals.h
 * @brief Module globals.
 *
 */


#ifndef _OSCC_BRAKE_GLOBALS_H_
#define _OSCC_BRAKE_GLOBALS_H_


#include "brake_control.h"
#include "DAC_MCP49xx.h"
#include "mcp_can.h"

/*
 * @brief Green LED Pin.
 *
 */
#define PIN_LED_GREEN ( 17 )

/*
 * @brief Green LED Pin.
 *
 */
#define PIN_LED_RED ( 30 )

/*
 * @brief Chip select pin of the DAC IC.
 *
 */
#define PIN_DAC_CHIP_SELECT ( 9 )

/*
 * @brief Chip select pin of the CAN IC.
 *
 */
#define PIN_CAN_CHIP_SELECT ( 10 )

/*
 * @brief High signal pin of the brake pedal position sensor.
 *
 */
#define PIN_BRAKE_PEDAL_POSITION_SENSOR_A ( A1 )

/*
 * @brief Low signal pin of the brake pedal position sensor.
 *
 */
#define PIN_BRAKE_PEDAL_POSITION_SENSOR_B ( A0 )

/*
 * @brief High signal pin of the brake pedal position spoof output.
 *
 */
#define PIN_BRAKE_PEDAL_POSITION_SPOOF_A ( A2 )

/*
 * @brief Low signal pin of the brake pedal position spoof output.
 *
 */
#define PIN_BRAKE_PEDAL_POSITION_SPOOF_B ( A3 )

/*
 * @brief Relay enable pin for the spoof output.
 *
 */
#define PIN_SPOOF_ENABLE ( 6 )

/*
 * @brief Relay enable pin for the brake lights.
 *
 */
#define PIN_BRAKE_LIGHT_ENABLE ( 5 )


#ifdef GLOBAL_DEFINED
    DAC_MCP49xx g_dac( DAC_MCP49xx::MCP4922, PIN_DAC_CHIP_SELECT );
    MCP_CAN g_control_can( PIN_CAN_CHIP_SELECT );

    #define EXTERN
#else
    extern DAC_MCP49xx g_dac;
    extern MCP_CAN g_control_can;

    #define EXTERN extern
#endif


EXTERN volatile bool g_brake_command_timeout;

EXTERN volatile brake_control_state_s g_brake_control_state;


#endif /* _OSCC_BRAKE_GLOBALS_H_ */
