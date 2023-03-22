#ifndef SARAcommunicator_h
#define SARAcommunicator_h

#include <Arduino.h>
#include <stdarg.h> // contains va_start stuff

class SARAconnector {
    public:
        SARAconnector(bool debug = false); // constructor
        void begin();
        bool modemHardwarePowerOn();
        bool modemHardwarePowerOff();
        bool modemATPowerOff();
        bool modemHardwareHardReset(); // can brick your modem, but sometimes you just gotta
        bool disableRadio();
        bool enableRadio();

        // variable manipulators
        void setSimContext(char SIM_APN[64],char SIM_PIN[4]);
        void setMqttContext(char mqtt_server_ip_address[64],char mqtt_server_domain_name[64],char mqtt_client_id[64],char mqtt_user[64],char mqtt_password[64],int port=1883,char mqtt_default_topic[128]="");
        bool initializeSimContext();
        bool initializeMqttContext();
        void displayObjectContext();

        // Helpers
        bool sendATCommand(char command[512]);
        bool stringStartsWith(const char* main_string, const char* substring);
        bool stringEndsWith(const char* main_string, const char* substring);
        int SARAcommunicate(char command[512],int timeout = 1000, char positive_response[16] = "OK", char negative_response[16] = "ERROR");
        int waitForResponse(int timeout=1000);
        bool clearRawResponseBuffer();
        char * createFormatString(const char *format, ...);

        // status
        bool modemIsResponsive();
        bool modemIsRegisteredToNetwork();
        bool modemCanPing(char ip_address[64]);
        bool modemStatus();

        // info
        bool getModemInfo();

        // MQTT
        bool MqttLogin();
        bool MqttLogout();
        bool MqttPublish(char message[], char topic[]);

        // data
        int mqtt_port;
        char mqtt_client_id[64];
        char mqtt_server_ip_address[64];
        char mqtt_server_domain_name[64];
        char mqtt_default_topic[128];
        char mqtt_password[64];
        char mqtt_user[64];

        char SIM_PIN[4];
        char SIM_APN[64];

        // internal data
        int rawResponseBufferPosition;
        char rawResponseBuffer[512]; // used to temporarily store modem responses
        char formatStringBuffer[512]; // used for storing the formatted strings created for AT commands
        int modem_communication_failure_counter;
        int modem_hard_reset_counter;
        int SARA_resetpin;
        int modemNetworkConnectionStatus; // 0 - disconnected, 1 - connected, 2> - unknown
        int modemResponsiveStatus; // 0 - unresponsive, 1 - responsive, 2> - unknown 
        int modemPowerStatus; // 0 - no power, 1 - has power, 2> - unknown 
        bool debug; // lots of printing
        int MODEM_MIN_RESPONSE_TIME; 

 


};


#endif

