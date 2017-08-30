/**
 * @file globals.h
 * @brief Module globals.
 *
 */


#ifndef _OSCC_KIA_SOUL_CAN_GATEWAY_GLOBALS_H_
#define _OSCC_KIA_SOUL_CAN_GATEWAY_GLOBALS_H_


#include "mcp_can.h"
#include "gateway_can_protocol.h"
#include "chassis_state_can_protocol.h"


/*
 * @brief Chip select pin of the OBD CAN IC.
 *
 */
#define PIN_OBD_CAN_CHIP_SELECT ( 9 )

/*
 * @brief Chip select pin of the Control CAN IC.
 *
 */
#define PIN_CONTROL_CAN_CHIP_SELECT ( 10 )


#ifdef GLOBAL_DEFINED
    MCP_CAN g_obd_can( PIN_OBD_CAN_CHIP_SELECT );
    MCP_CAN g_control_can( PIN_CONTROL_CAN_CHIP_SELECT );

    #define EXTERN
#else
    extern MCP_CAN g_obd_can;
    extern MCP_CAN g_control_can;

    #define EXTERN extern
#endif


EXTERN oscc_report_heartbeat_s g_tx_heartbeat;
EXTERN oscc_report_chassis_state_1_s g_tx_chassis_state_1;
EXTERN oscc_report_chassis_state_2_s g_tx_chassis_state_2;
EXTERN oscc_report_chassis_state_3_s g_tx_chassis_state_3;

EXTERN uint32_t g_obd_steering_wheel_angle_rx_timestamp;
EXTERN uint32_t g_obd_wheel_speed_rx_timestamp;
EXTERN uint32_t g_obd_brake_pressure_rx_timestamp;
EXTERN uint32_t g_obd_turn_signal_rx_timestamp;
EXTERN uint32_t g_obd_engine_rpm_temp_rx_timestamp;
EXTERN uint32_t g_obd_gear_position_rx_timestamp;
EXTERN uint32_t g_obd_accelerator_pedal_position_rx_timestamp;
EXTERN uint32_t g_obd_vehicle_speed_rx_timestamp;

#endif /* _OSCC_KIA_SOUL_CAN_GATEWAY_GLOBALS_H_ */
