#ifndef EOR_CFG_H
#define EOR_CFG_H

#define IO_MODULE_COUNT		24
#define JSN2_MODULE_COUNT	6
#define METER_COUNT		3
#define USER_COUNT		3
#define WEATHER_AUTOM_COUNT	6
#define CIRCUIT_COUNT		20
#define RS_COUNT		6 /* jeden fizyczny na rj45, 4 wirtualne i jedno miejsce rezerwowe */

#include <stdint.h>

typedef struct __attribute((packed)){
    char id_txt[13];	//EOR_KON
    uint8_t res1[3];
    uint8_t ver;
    uint8_t rev;
    uint32_t file_len;
    uint16_t crc16;
    uint8_t res2[4];
} eorkonf_hdr_t;

typedef struct __attribute((packed)){
    uint8_t type;
    uint8_t addr;
    uint8_t res[6];
} io_module_cfg_t;

typedef struct __attribute((packed)){
    uint8_t type;
    uint8_t addr;
    uint8_t res[6];
} jsn2_module_cfg_t;

typedef struct __attribute((packed)){
    uint8_t type;
    char id[15];
    uint8_t res[8];
} meter_cfg_t;

typedef struct __attribute((packed)){
    char name[64];
    char description[256];
    uint8_t cir_count;		//max 20
    uint8_t weather_autom_count;	//max 6
    uint8_t ctrl_group_cnt;		//chyba ustalimy na stałe 6
    uint8_t res1[5];
    uint8_t ind_on_time;		//czas indywidualnego załączenia <1 - 240 min>
    uint8_t phase_asymmetry_check;	//sprawdzania asymetrii faz zasilania dostępne gdy jest licznik energii
    uint8_t phase_assymetry_tolerance; //tolerancja w %
    uint16_t min_phase_voltage;	//minimalne napięcie fazy sprawdzane gdy jest licznik
    uint8_t res2[3];
    uint8_t ctrl_user_level;	//poziom użytkownika umożliwiający sterowanie (0 - bez hasła)
    uint8_t param_edit_user_level;	//poziom użytkownika umożliwiający zmianę parametrów (0 - bez hasła, nie mniejsze niż poziom dla sterowania)
    uint8_t res3[2];
    uint8_t screen_saver_enable;
    uint8_t screen_saver_time;
    uint8_t res4[10];
} general_cfg_t;

typedef struct __attribute((packed)){
    char name[32];
    uint8_t security_level;
    uint16_t pin;
    uint8_t res[8];
} user_cfg_t;

typedef struct __attribute((packed)){
    uint8_t type;			//typy: io (z modułu TH), modbus (np z jsn-2), can ( z innego urządzenia)
    uint8_t addr;			//w can i modbus - adres, w io - id modułu
    uint16_t reg_no;		//numer kanału w TH, rejestru w modbus lub serii can
    uint8_t res[4];
} analog_sensor_cfg_t;

typedef struct __attribute((packed)){
    uint8_t type;			//typy: io, modbus, can
    uint8_t addr;			//w can i modbus - adres, w io - id modułu
    uint16_t reg_no;		//w io nie wykorzystane, nr rejestru w modbus lub nr serii can
    uint8_t bit_no;			//numer bitu w rejestrze lub numer wejścia w io
    uint8_t res[3];
} binary_sensor_cfg_t;

typedef struct __attribute((packed)){
    analog_sensor_cfg_t temperature_sensor;
    binary_sensor_cfg_t humidity_sensor;
    binary_sensor_cfg_t snow_fall_sensor;
    uint8_t res[16];
} general_weather_measure_cfg_t;

typedef struct __attribute((packed)){
    uint8_t active;
    uint8_t module_id;
    uint8_t bit_no;
    uint8_t res;
} binary_io_t;

typedef struct __attribute((packed)){
    analog_sensor_cfg_t t_cold;
    analog_sensor_cfg_t t_hot;
    binary_sensor_cfg_t snow_blow_sensor;
    binary_io_t sensor_pwr_ctrl;	//kontrola zasilania czujników: obecność, nr modułu IO + numer wejścia
    uint16_t res[16];
} weather_autom_cfg_t;

typedef struct __attribute((packed)){
    int16_t t_r_on_fr;		//Załączenie/wyłączenie grzania dla Duży mróz
    int16_t t_r_off_fr;
    int16_t t_r_on_wet;		//Załączenie/wyłączenie grzania dla Deszcz marznący
    int16_t t_r_off_wet;
    int16_t t_r_on_sn;		//Załączenie/wyłączenie grzania dla Śnieg
    int16_t t_r_off_sn;
    int16_t t_frost_on_r;		//Temperatura wejścia/wyjścia do dużego mrozu
    int16_t t_frost_off_r;
    int16_t t_frost_on_l;		//progowa temperatura „dużego mrozu” dla zamknięć
    int16_t t_frost_off_l;		//progowa temperatura wyjścia z „dużego mrozu” dla zamknięć
    uint16_t res[16];
} temperatures_cfg_t;

typedef struct __attribute((packed)){
    uint8_t active;
    uint8_t res1[3];
    binary_io_t conf_input;
    uint8_t cvm_id;
    uint8_t cvm_ch_id;
    uint8_t res2[2];
    uint16_t p_nom;
    uint16_t p_tol;
    uint8_t res3[8];
} phase_cfg_t;

typedef struct __attribute((packed)){
    char name[8];			//nazwa obwodu
    uint8_t active;			//obwód aktywny / rezerwa
    uint8_t type;			//typ obwodu: ogrzewanie rozjazdów lub zamknięć
    uint8_t group_id;		//nr grupy
    uint8_t weather_autom_id;	//nr automatu
    uint8_t reference;		//czy obwód wzorcowy - wzorcowego nie da się tak łatwo wyłączyć
    uint8_t res1[3];
    phase_cfg_t phase_cfg[3];
    binary_io_t relay;
    binary_io_t rel_conf;
    uint8_t res2[16];
} circuit_cfg_t;

typedef struct __attribute((packed)){
    uint8_t type;			//typ grupy - standardowa lub z ograniczeniem co do mocy
    uint8_t cir_count_to_heat;	//jak grupa jest ograniczona, to jeszcze liczba obwodów, które mogą być na raz grzane
//	nr automatu dla grupy
//	lista obwodów dla grupy
    uint8_t res[18];
} group_cfg_t;

//Konfiguracja dla dodatkowych wejść sygnalizacyjnych (może w konfiguracji logika wejścia?):
typedef struct __attribute((packed)){
    binary_io_t dor;		//kontrola drzwi szafy: obecność, nr modułu IO + numer wejścia
    binary_io_t break_in;		//kontrola skrzyni transformatora: obecność, nr modułu IO + numer wejścia
    binary_io_t surge_protect;	//zadziałanie zabezpieczenia antyprzepięciowego: obecność, nr modułu IO + numer wejścia
    binary_io_t remote_ctrl;	//jak bedzie nieaktywne oznacza to, że jak nie ma lokalnego ani ręcznego, wtedy automatycznie robi się zdalnie
    binary_io_t local_ctrl;		//sterowanie lokalne (z panelu) - musi być
    binary_io_t hand_ctrl;		//sterowanie ręczne (przyciskami bez cpu) - musi być
    uint8_t res[20];
} io_cfg_t;

typedef struct __attribute((packed)){
    uint8_t active;			//aktywność kanału komunikacji
    uint8_t no;			//numer logiczny can
    uint8_t baud;			//prędkość can 25kbps lub 125kbps
    uint8_t accept_cmd;		//akceptowanie poleceń
    uint8_t send_reports;		//wysyłanie meldunków
    uint8_t res[7];
} can_cfg_t;

typedef struct __attribute((packed)){
    uint8_t active;			//aktywność kanału komunikacji
    uint8_t addr;		//adres modbus
    uint8_t transmission_medium;	//medium transmisyjne - rs lub ethernet
    uint16_t port_no;		//nr portu rs lub tcp na którym to będzie dzialać
    uint8_t accept_cmd;		//akceptowanie poleceń
    uint8_t res[10];
} modbus_slave_cfg_t;

typedef struct __attribute((packed)){
    uint8_t active;
    uint8_t transmission_medium;	//medium transmisyjne - rs lub ethernet
    uint8_t no;			//nr tgfm
    uint8_t accept_cmd;		//akceptowanie poleceń
    uint8_t send_reports;		//wysyłanie meldunków
    uint16_t port;			//nr portu rs dla rs lub numer portu tcp dla ethernetu
    uint8_t server_ip[4];		//ip serwera dla połączenia ethernet
    uint8_t res[9];
} tgfm_cfg_t;

typedef struct __attribute((packed)){
    uint8_t active;
    uint8_t type;			//wirtualny czy fizyczny
    uint32_t baud;			//oczywiście tylko dla fizycznego
    uint8_t stop_bits;
    uint8_t parity;
    uint16_t port;			//nr portu dla wirtualnego na moxie
    uint8_t server_ip[4];
    uint8_t protocol;		//np.: brak, konsola, modbus rtu master dla jsn-2, protokół do podłączenia licznika energii itp.
    uint8_t res[17];
} rs_cfg_t;

typedef struct __attribute((packed)){
    uint8_t ip[4];
    uint8_t mask[4];
    uint8_t gateway[4];
    uint8_t res[12];
} eth_cfg_t;

extern eorkonf_hdr_t               eorkonf_hdr;
extern general_cfg_t               general_cfg;
extern user_cfg_t                  user_cfg[];
extern io_module_cfg_t             io_module_cfg[];
extern jsn2_module_cfg_t           jsn2_module_cfg[];
extern meter_cfg_t                 meter_cfg[];
extern general_weather_measure_cfg_t   general_weather_measure_cfg;
extern weather_autom_cfg_t         weather_autom_cfg[];
extern temperatures_cfg_t          temperatures_cfg;
extern circuit_cfg_t               circuit_cfg[];
extern group_cfg_t                 group_cfg;
extern io_cfg_t                    io_cfg;
extern can_cfg_t                   can_cfg;
extern modbus_slave_cfg_t          modbus_slave_cfg;
extern tgfm_cfg_t                  tgfm_cfg;
extern rs_cfg_t                    rs_cfg[];
extern eth_cfg_t                   eth_cfg;

extern const int32_t eorkonf_data_size;

#endif // EOR_CFG_H
