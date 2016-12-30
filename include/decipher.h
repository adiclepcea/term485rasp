#include <stdbool.h>

typedef enum{
  COOLING,
  HEATING,
  DHW,
  COOLING_PLUS_DHW,
} RUNNING_MODE;

typedef enum{
  HEATING_COOLING,
  ONLY_DHW,
  DHW_HEATING_COOLING,
} HEAT_PUMP_TYPE;

typedef enum{
  DHW_PUMP_CONTROLLING,
  COMPRESSOR_1,
  COMPRESSOR_2,
  DHW_BACKUP_ELECTRIC_HEATER_CONTROLLING,
  HEATING_BACKUP_ELECTRIC_HEATER_CONTROLLING,
  WATER_SOURCE_PUMP_CONTROLLING,
  COMPRESSOR_HEATING_CABLE,
  DHW_REFILL_VALVE_CONTROLLING,
} SIGNAL_OUTPUT_CONDITION;

typedef enum{
  ANTI_FREEZING=5,
  HEAT_PUMP_SYSTEM_ELECTRIC_DEVICES,
  COMPRESSOR_RESTING,
} RUNNING_CONDITION;

typedef enum{
  DHW_INPUT,
  AMBIENT_TEMP,
  WATER_SOURCE_INPUT,
  WATER_SOURCE_OUTPUT,
  HEATING_COOLING_INPUT,
  WATER_FLOW,
  DHW_OUTPUT,
  PHASE_SEQUENCE,
} ERROR_CONDITION1;

typedef enum{
  DHW_WATER_LEVEL_FAULT,
  HEATING_COOLING_WATER_FLOW_FAULT,
  HEATING_COOLING_OUTPUT_WATER_FLOW_FAULT,
  COOLING_WATER_OUTPUT_TOO_LOW,
  HIGH_PRESSURE_1,
  LOW_PRESSURE_1,
  HIGH_PRESSURE_2,
  LOW_PRESSURE_2
} ERROR_CONDITION2;

typedef enum{
  HEATING_COOLING_ANTI_FREEZING,
  DHW_ANTI_FREEZING,
  WATER_SOURCE_FLOW_FAULT,
  DHW_OUTPUT_TEMP_TOO_HIGH,
  WATER_SOURCE_INPUT_TEMP_TOO_LOW,
  WATER_SOURCE_OUTPUT_TEMP_TOO_LOW,
  WATER_SOURCE_INPUT_TEMP_TOO_HIGH,
  WATER_SOURCE_OUTPUT_TEMP_TOO_HIGH
} ERROR_CONDITION3;

typedef struct{
  bool high_presure;
  bool low_presure;
  bool backup_electric_heater;
  bool manual_control_backup_heater;
  bool memory_functions;
  bool dual_heat_pump;
  bool turn_on_heat_pump;
  unsigned char running_mode;
  float water_source_over_low;
  float water_source_over_high;
  unsigned char water_refill_time_interval;
  unsigned char compressor_resting_time;
  float dhw_temp;
  float cooling_water_temp;
  float heating_water_temp;
  float dhw_backlash_temp;
  float heating_cooling_backlash_temp;
  float ambient_temp_turn_on_backup_electric;
  float backup_electric_backlash_temp;
  float water_temp_accuracy_setting;
  float anti_freeze_water_temp;
  float cooling_water_temp_low;
  float heating_water_temp_high;
  float dhw_refill_water_temp;
  float dhw_refill_stop_water;
  unsigned char het_pump_type;
} CC;

typedef struct{
  float dhw_temp_input;
  float ambient_temp;
  float water_source_input_temp;
  float water_source_output_temp;
  float dhw_output_temp;
  float heating_cooling_input_temp;
  float heating_cooling_output_temp;
  unsigned char signal_output_condition;
  unsigned char running_condition;
  unsigned char error_condition1;
  unsigned char error_condition2;
  unsigned char error_condition3;
} DD;
