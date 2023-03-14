#ifndef SARAcommunicator_h
#define SARAcommunicator_h

#include <Arduino.h>

class SARAconnector {
    public:
        SARAconnector(bool debug = false); // constructor
        void modemHardwarePowerOn();
        void modemHardwarePowerOff();
        bool modemATPowerOff();
        bool modemHardwareHardReset(); // can brick your modem, but sometimes you just gotta
        bool disableRadio();
        bool enableRadio();

        // variable manipulators
        bool setSimContext(String SIM_PIN,String SIM_APN);
        bool setMqttContext(String ip_address,String domain_name,String default_topic = "");
        void displayConfig();

        // Helpers
        bool sendATCommand();
        bool communicate(String command,int timeout = 1000, String positive_response = "OK", String negative_response = "ERROR");
        int waitForResponse(int timeout=1000);

        // status
        bool modemIsResponsive();
        bool modemIsRegisteredToNetwork();
        bool modemCanPing(String ip_address);
        bool modemStatus();

        // info
        bool getModemInfo();

        // data
        String mqtt_server_ip_address;
        String mqtt_server_domain_name;
        String mqtt_default_topic;

        String SIM_PIN;
        String SIM_APN;

        // internal data
        String rawResponseBuffer; // used to temporarily store modem responses
        int modem_communication_failure_counter;
        int modem_hard_reset_counter;
        int SARA_resetpin;
        int modemNetworkConnectionStatus; // 0 - disconnected, 1 - connected, 2> - unknown
        int modemResponsiveStatus; // 0 - unresponsive, 1 - responsive, 2> - unknown 
        int modemPowerStatus; // 0 - no power, 1 - has power, 2> - unknown 
        bool debug; // lots of printing

 


};


#endif

