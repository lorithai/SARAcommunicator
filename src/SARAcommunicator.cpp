#include "Arduino.h"
#include "SARAcommunicator.h"    

struct SARAResponses{
    const int NEGATIVE = 0;
    const int POSITIVE = 1;
    const int TIMEOUT = 2;
}SARAResponse;


SARAconnector::SARAconnector(bool debug) : // Constructor. Runs automatically
    SARA_resetpin(SARA_RESETN), // SARA_RESETN pin defined somewhere in Arduino core?
    SIM_APN(""),
    SIM_PIN(""),

    // MQTT
    mqtt_server_ip_address(""),
    mqtt_server_domain_name(""),
    mqtt_default_topic(""),
    mqtt_client_id(""),
    mqtt_port(1883),
    debug(debug),

    // response handlers
    rawResponseBuffer(),
    rawResponseBufferPosition(0),
    
    // modem status
    modem_communication_failure_counter(0),
    modem_hard_reset_counter(0),
    modemNetworkConnectionStatus(2),
    modemResponsiveStatus(2),
    modemPowerStatus(2),

    // constants
    MODEM_MIN_RESPONSE_TIME(20)
{
    digitalWrite(SARA_resetpin, LOW); // Hard reset is activated with a HIGH signal. Not reccommended to often, so turn to low by default

    // check current state
    // modem responsive?
    // Packet Switching Data set?
    // registered to network?
} 

void SARAconnector::begin(){ //TODO rename and place in constructor?
    SerialSARA.begin(115200);
}

void SARAconnector::setSimContext(char SIM_APN[],char SIM_PIN[]) {
    strcpy(this->SIM_APN,SIM_APN);
    strcpy(this->SIM_APN,SIM_APN);
}

void SARAconnector::setMqttContext(char mqtt_server_ip_address[],char mqtt_server_domain_name[],char mqtt_client_id[],char mqtt_user[],char mqtt_password[],int port,char mqtt_default_topic[]) {
    strcpy(this->mqtt_server_ip_address,mqtt_server_ip_address);
    strcpy(this->mqtt_server_domain_name,mqtt_server_domain_name);
    strcpy(this->mqtt_client_id,mqtt_client_id);
    this->mqtt_port = port;
    strcpy(this-> mqtt_default_topic, mqtt_default_topic);
    strcpy(this->mqtt_password,mqtt_password);
    strcpy(this->mqtt_user,mqtt_user);
}

bool SARAconnector::MqttLogin(){
    this->clearRawResponseBuffer();
    this->sendATCommand("AT+UMQTTC=1");
    int start_millis = millis();
    while (!strstr(this->rawResponseBuffer,"OK")) {
        if (SerialSARA.available()) {
        this->rawResponseBuffer[this->rawResponseBufferPosition++] = SerialSARA.read();
        }
        if ((start_millis+10000)<millis()) {
            Serial.println("Communication timed out");
            return false;
        }  
    }

    if (strstr(this->rawResponseBuffer,"1,1")){
        Serial.println("MQTT login success");
        return true;
    } else {
        return false;
    }
    return true;
}

bool SARAconnector::MqttLogout(){
    this->clearRawResponseBuffer();
    sendATCommand("AT+UMQTTC=0");
    int start_millis = millis();
    while (!strstr(this->rawResponseBuffer,"OK")) {
        if (SerialSARA.available()) {
            this->rawResponseBuffer[this->rawResponseBufferPosition++] = SerialSARA.read();
        }
        if ((start_millis+10000)<millis()) {
            Serial.println("Communication timed out");
            return false;
        }  
    }

    if (strstr(this->rawResponseBuffer,"0,1")){
        Serial.println("MQTT logout success");
        return true;
    } else {
        return false;
    }
}

bool SARAconnector::MqttPublish(char message[],char topic[]){
    char mqttMessageBuffer[512];
    int comresponse;
    sprintf(mqttMessageBuffer,"AT+UMQTTC=2,0,0,\"%s\",\"%s\"",topic,message);
    comresponse = this->SARAcommunicate(mqttMessageBuffer);
    if (comresponse != SARAResponse.POSITIVE) {
        modem_communication_failure_counter +=1;
        return false;
    } else {
        if (strstr(this->rawResponseBuffer,": 2,1")){
            return true
        }
        return true;
    }
    
}

char * SARAconnector::createFormatString(const char *format, ...) {
    // Combines the format string with any other parameters passed after it. Wonky way of concatinating strings. The three ... are called elipsis for some reason 
    va_list variable_argument_list; // handles taking the variables passed through the elipsis (...)
    va_start(variable_argument_list,format); // start the process, using the format char as base
    vsnprintf(this->formatStringBuffer,sizeof(this->formatStringBuffer),format,variable_argument_list); // one by one, replace the %d or %s or whatever with the ... arguments and put them into the commandBuffer[100]
    va_end(variable_argument_list); // indicate that the process is finished
    return this->formatStringBuffer;
}


bool SARAconnector::initializeSimContext(){
    if (this->modemIsResponsive()){
    this->SARAcommunicate("AT+CFUN=0"); // Turn off radio
    this->SARAcommunicate(this->createFormatString("AT+CGDCONT=1,\"IP\",\"%s\"",this->SIM_APN)); // SET context IP version and APN
    if (this->SARAcommunicate("AT+CPIN?",10000,"READY") == 0){
        Serial.println("Need to set pin");
    } // set pin (if needed) TODO
    this->SARAcommunicate("AT+UAUTHREQ=1,0"); // SET context IP version and APN
    this->SARAcommunicate("AT+CFUN=1"); // Turn on radio
    delay(1000);
    this->SARAcommunicate("AT+CEREG?");
    return true;
    }
    return false;
}

bool SARAconnector::initializeMqttContext(){
    this->SARAcommunicate("AT+CFUN=0"); // Turn off radio
    this->SARAcommunicate(this->createFormatString("AT+UMQTT=0,\"%s\"", this->mqtt_client_id));
    this->SARAcommunicate(this->createFormatString("AT+UMQTT=1,%i", this->mqtt_port)); 
    this->SARAcommunicate(this->createFormatString("AT+UMQTT=2,\"%s\"", this->mqtt_server_domain_name)); 
    this->SARAcommunicate(this->createFormatString("AT+UMQTT=3,\"%s\"", this->mqtt_server_ip_address)); 
    this->SARAcommunicate(this->createFormatString("AT+UMQTT=4,\"%s\",\"%s\"", this->mqtt_user,this->mqtt_password)); // set MQTT user and pw
    this->SARAcommunicate("AT+CFUN=1"); // Turn off radio
    return true;
}

void SARAconnector::displayObjectContext(){
    Serial.print("SIM_APN: ");
    Serial.println(this->SIM_APN);
    Serial.print("SIM_PIN: ");
    Serial.println(this->SIM_PIN);
    Serial.print("Mqtt_server_ip_address: ");
    Serial.println(this->mqtt_server_ip_address);
    Serial.print("Mqtt_server_domain_name: ");
    Serial.println(this->mqtt_server_domain_name);
    Serial.print("default_mqtt_topic: ");
    Serial.println(this->mqtt_default_topic);
    Serial.print("Mqtt_user: ");
    Serial.println(this->mqtt_user);
    Serial.print("Mqtt_password: ");
    Serial.println(this->mqtt_password);
}

bool SARAconnector::modemHardwarePowerOn(){
    int start_millis = millis();
    if (!this->modemIsResponsive()) {
        if (this->debug == true) {
            Serial.println("Powering modem ON");
        }
        digitalWrite(SARA_PWR_ON, HIGH);  // Datasheet LOW = ARDUINO HIGH
        delay(200); // Datasheet says power-on pulse should be >=150ms, <=3200ms
        digitalWrite(SARA_PWR_ON, LOW); // Datasheet HIGH = ARDUINO LOW
        delay(1000); // leave some time for the modem to boot up
        while (!this-modemIsResponsive()) {
            delay(1000);
            if (start_millis+10000>=millis()) {
                return false;
            }
        }
        return true;
    } else {return true;}

}
bool SARAconnector::modemHardwarePowerOff(){

    // Gracefully power off
    if (this->modemIsResponsive()) { // first checks if modem responds.
        if (this->debug == true) {
            Serial.println("Powering modem OFF");
        }
        digitalWrite(SARA_PWR_ON, HIGH);  // Datasheet LOW = ARDUINO HIGH
        delay(1600); // Datasheet says power-off pulse should be >=1500ms
        digitalWrite(SARA_PWR_ON, LOW); // Datasheet HIGH = ARDUINO LOW
        delay(500); // leave some time for the modem to power down
        if (!this->modemIsResponsive()){
            return true; 
        } else {return false;}
    } else {return true;}

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
    this->SARAcommunicate("AT+CFUN=0");
    return true;
}
bool SARAconnector::enableRadio(){
    Serial.println("Radio on");
    return true;
}

// Helpers
bool SARAconnector::stringStartsWith(const char* main_string, const char* substring){
    if (strlen(main_string)>=strlen(substring)){
        if (strncmp(main_string,substring,strlen(substring) == 0)) {
            return true;
        }
    } else { return false; }
}
bool SARAconnector::stringEndsWith(const char* main_string, const char* substring){
    if (strlen(main_string)>=strlen(substring)){
        char main_string_end[10];
        for (int i = strlen(main_string)-strlen(substring); i <= strlen(main_string); i++){
            main_string_end[i-(strlen(main_string)-strlen(substring))] = main_string[i];
        }
        if (strncmp(main_string_end,substring,strlen(substring) == 0)) {
            return true;
        }
    } else { return false; }
}


bool SARAconnector::sendATCommand(char command[]){
    if (this->debug == true) {
        Serial.println("Command:");
        Serial.println(command);
    }
    SerialSARA.println(command);
    delay(MODEM_MIN_RESPONSE_TIME);
    return true;
}

int SARAconnector::SARAcommunicate(char command[],int timeout, char positive_response[], char negative_response[]){
    this->clearRawResponseBuffer();
    this->sendATCommand(command);
    delay(100);
    int start_millis = millis();
    while (!SerialSARA.available()){ // initial wait for response
        delay(100);
        if ((start_millis+timeout)<millis()) {
            return SARAResponse.TIMEOUT;
        }
    }
    delay(100);
    // removes the echo. There might be a command to disable this, but to lazy to look for it.
    while (SerialSARA.available()) {
        this->rawResponseBuffer[this->rawResponseBufferPosition++] = SerialSARA.read();
        if (this->stringStartsWith(this->rawResponseBuffer,"AT") && this->stringStartsWith(this->rawResponseBuffer,"\r\n")) {
            this->clearRawResponseBuffer();
            break;
        }
    }
    // TODO wait for end of response, or that sara is no longer available?
    while (!strstr(this->rawResponseBuffer,positive_response)) {
        if (SerialSARA.available()) {
            this->rawResponseBuffer[this->rawResponseBufferPosition++] = SerialSARA.read();
        }
        if ((start_millis+timeout)<millis()) {
            Serial.println("AT reading timed out timed out");
            return SARAResponse.TIMEOUT;
        }
    }
    delay(10); 
    // Read remaining serial from the modem just in case
    while (SerialSARA.available()) {
        this->rawResponseBuffer[this->rawResponseBufferPosition++] = SerialSARA.read();
        delay(1);
  }

  // Parse and check for responses. use strstr to find. 
  // if positive response, true (1), else, false (0 or 2)
  Serial.println("response");
  Serial.println(this->rawResponseBuffer);

  return SARAResponse.POSITIVE;
}


int SARAconnector::waitForResponse(int timeout){
    Serial.println("waiting");
    return true;
}

bool SARAconnector::clearRawResponseBuffer(){
    this->rawResponseBufferPosition = 0;
    memset(this->rawResponseBuffer,0,sizeof(this->rawResponseBuffer));
}

// status
bool SARAconnector::modemIsResponsive(){
    this->clearRawResponseBuffer();
    this->sendATCommand("AT");
    int start_millis = millis();
    while (!SerialSARA.available()){ // check if SARA responds or not
    delay(100);
    if ((start_millis+100)<millis()) {
      return false;
    }
    }
    while (!strstr(this->rawResponseBuffer,"OK")) { // check if response contain "OK"
        if (SerialSARA.available()) {
        this->rawResponseBuffer[this->rawResponseBufferPosition++] = SerialSARA.read();
        }
        if ((start_millis+1000)<millis()) {
        //Serial.println("AT reading timed out timed out");
        return false;
        }
    }
    while (SerialSARA.available()) {
        this->rawResponseBuffer[this->rawResponseBufferPosition++] = SerialSARA.read();
    }
    if (debug == true) {
        Serial.println("Response:");
        Serial.println(rawResponseBuffer);
    }
    return true;
}
bool SARAconnector::modemIsRegisteredToNetwork(){
    Serial.println("Modem is connected");
    return true;
}

bool SARAconnector::modemCanPing(char ip_address[64]) {
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
        this->SARAcommunicate("AT+CPIN?"); // is pin ok?
        this->SARAcommunicate("AT+CGDCONT?"); // responds conted_id, ipversion, APN_name, ip_address, ++
        this->SARAcommunicate("AT+UAUTHREQ=0"); // is there any authentication needed? (checks if using none is OK)
    }
    // check modem network connection status
    if (this->modemResponsiveStatus == 1 && this->modemNetworkConnectionStatus == 1) {
        // check cereg response
        this->SARAcommunicate("AT+CEREG?");
    }

    Serial.println("status");
}

// info
bool SARAconnector::getModemInfo(){
    Serial.println("Hardware On");
    return true;
}


 