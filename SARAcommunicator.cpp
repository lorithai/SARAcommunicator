#include "Arduino.h"
#include "SARAcommunicator.h"    

# define MODEM_MIN_RESPONSE_TIME = 20; // modem needs some time to respond

SARAconnector::SARAconnector(bool debug) : // Constructor. Run automatically
    SARA_resetpin(SARA_RESETN), // pin defined somewhere in Arduino core?
    SIM_APN(""),
    SIM_PIN(""),
    mqtt_server_ip_address(""),
    mqtt_server_domain_name(""),
    mqtt_default_topic(""),
    debug(debug),
    // internal data
    rawResponseBuffer(""),
    
    // modem status
    modem_communication_failure_counter(0),
    modem_hard_reset_counter(0),
    modemNetworkConnectionStatus(2),
    modemResponsiveStatus(2),
    modemPowerStatus(2)

{
    digitalWrite(SARA_resetpin, LOW); // Hard reset is activated with a HIGH signal. Not reccommended to often, so turn to low by default

    // check current state
    // modem responsive?
    // Packet Switching Data set?
    // registered to network?
}   

bool SARAconnector::setSimContext(String SIM_PIN,String SIM_APN){
    this->SIM_APN = SIM_APN;
    this->SIM_PIN = SIM_PIN;
    return true;
}

bool SARAconnector::setMqttContext(String ip_address,String domain_name,String default_topic){
    this->mqtt_server_ip_address = ip_address;
    this->mqtt_server_domain_name = domain_name;
    this->mqtt_default_topic = default_topic;
    return true;
}

void SARAconnector::displayConfig(){
    Serial.print("SIM_APN: ");
    Serial.println(SIM_APN);
    Serial.print("SIM_PIN: ");
    Serial.println(SIM_PIN);
    Serial.print("Mqtt_server_ip_address: ");
    Serial.println(mqtt_server_ip_address);
    Serial.print("Mqtt_server_domain_name: ");
    Serial.println(this->mqtt_server_domain_name);
    Serial.print("default_mqtt_topic: ");
    Serial.println(mqtt_default_topic);
}

void SARAconnector::modemHardwarePowerOn(){
    if (this->debug == true) {
        Serial.println("Powering modem on");
    }
    digitalWrite(SARA_PWR_ON, HIGH);  // Datasheet LOW = ARDUINO HIGH
    delay(200); // Datasheet says power-on pulse should be >=150ms, <=3200ms
    digitalWrite(SARA_PWR_ON, LOW); // Datasheet HIGH = ARDUINO LOW
    delay(500); // leave some time for the modem to boot up
}
void SARAconnector::modemHardwarePowerOff(){
    if (this->debug == true) {
        Serial.println("Powering modem on");
    }
    Serial.println("Powering modem OFF");
    // Gracefully power off
    digitalWrite(SARA_PWR_ON, HIGH);  // Datasheet LOW = ARDUINO HIGH
    delay(1600); // Datasheet says power-off pulse should be >=1500ms
    digitalWrite(SARA_PWR_ON, LOW); // Datasheet HIGH = ARDUINO LOW
    delay(500); // leave some time for the modem to power down
    Serial.println("Hardware Off");

}
bool SARAconnector::modemATPowerOff(){
    Serial.println("Hardware AT off");
    return true;
}
bool SARAconnector::modemHardwareHardReset(){ // can brick your modem, but sometimes you just gotta
    Serial.println("Hardware Hard Reset");
    return true;
} 
bool SARAconnector::disableRadio(){
    Serial.println("Radio off");
    return true;
}
bool SARAconnector::enableRadio(){
    Serial.println("Radio on");
    return true;
}

// Helpers
bool SARAconnector::sendATCommand(){
    Serial.println("AT command send");
    return true;
}
bool SARAconnector::communicate(String command,int timeout, String positive_response, String negative_response){
    Serial.println("communicate");
    return true;
}
int SARAconnector::waitForResponse(int timeout){
    Serial.println("waiting");
    return true;
}

// status
bool SARAconnector::modemIsResponsive(){
    Serial.println("Modem is responsive");
    return true;
}
bool SARAconnector::modemIsRegisteredToNetwork(){
    Serial.println("Modem is connected");
    return true;
}

bool SARAconnector::modemCanPing(String ip_address) {
    Serial.println("ping");
}

bool SARAconnector::modemStatus() {
    // Power is on? Can't really test this as the Vint_power pin on the SARA module isn't connected to the Arduino
    // Modem is responsive to AT commands
    if (this->modemIsResponsive()) {
        this->modemPowerStatus = 1; // if modem is responding, modem must have power
        this->modemResponsiveStatus = 1; 
    } else {
        return false;
        this->modemResponsiveStatus = 0;
    }
    // check modem context. 
    if (this->modemResponsiveStatus == 1){ // If modem is responsive, it is assumed to have power.
        this->communicate("AT+CPIN?"); // is pin ok?
        this->communicate("AT+CGDCONT?"); // responds conted_id, ipversion, APN_name, ip_address, ++
        this->communicate("AT+UAUTHREQ=0"); // is there any authentication needed? (checks if using none is OK)
    }
    // check modem network connection status
    if (this->modemResponsiveStatus == 1 && this->modemNetworkConnectionStatus == 1) {
        // check cereg response
        this->communicate("AT+CEREG?");
    }

    Serial.println("status");
}

// info
bool SARAconnector::getModemInfo(){
    Serial.println("Hardware On");
    return true;
}


 