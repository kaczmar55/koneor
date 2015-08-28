#include "eor_cfg.h"

eorkonf_hdr_t               eorkonf_hdr;
general_cfg_t               general_cfg;
user_cfg_t                  user_cfg[USER_COUNT];
io_module_cfg_t             io_module_cfg[IO_MODULE_COUNT];
jsn2_module_cfg_t           jsn2_module_cfg[JSN2_MODULE_COUNT];
meter_cfg_t                 meter_cfg[METER_COUNT];
general_weather_measure_cfg_t   general_weather_measure_cfg;
weather_autom_cfg_t         weather_autom_cfg[WEATHER_AUTOM_COUNT];
temperatures_cfg_t          temperatures_cfg;
circuit_cfg_t               circuit_cfg[CIRCUIT_COUNT];
group_cfg_t                 group_cfg;
io_cfg_t                    io_cfg;
can_cfg_t                   can_cfg;
modbus_slave_cfg_t          modbus_slave_cfg;
tgfm_cfg_t                  tgfm_cfg;
rs_cfg_t                    rs_cfg[RS_COUNT];
eth_cfg_t                   eth_cfg;

const int32_t eorkonf_data_size = sizeof(general_cfg_t) +
                                sizeof(user_cfg_t) * USER_COUNT +
                                sizeof(io_module_cfg_t) * IO_MODULE_COUNT +
                                sizeof(jsn2_module_cfg_t) * JSN2_MODULE_COUNT +
                                sizeof(meter_cfg_t) * METER_COUNT +
                                sizeof(general_weather_measure_cfg_t) +
                                sizeof(weather_autom_cfg_t) * WEATHER_AUTOM_COUNT +
                                sizeof(temperatures_cfg_t) +
                                sizeof(circuit_cfg_t) * CIRCUIT_COUNT +
                                sizeof(group_cfg_t) +
                                sizeof(io_cfg_t) +
                                sizeof(can_cfg_t) +
                                sizeof(modbus_slave_cfg_t) +
                                sizeof(tgfm_cfg_t) +
                                sizeof(rs_cfg_t) * RS_COUNT +
                                sizeof(eth_cfg_t);
