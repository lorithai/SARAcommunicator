#include <SARAcommunicator.h>
#include <ArduinoJson.h>

SARAconnector saracom(true);

#include "arduino_secrets.h"; // PUT YOUR SECRET STUFF IN THIS FILE
char SIM_APN[] = SECRET_SIM_APN;
char SIM_PIN[] = SECRET_SIM_PIN;
char mqtt_server_ip_address[] = SECRET_MQTT_SERVER_IP_ADDRESS;
char mqtt_server_domain_name[] = SECRET_MQTT_SERVER_DOMAIN_NAME;
char mqtt_client_id[] = SECRET_MQTT_CLIENT_ID;
char mqtt_default_topic[] = SECRET_MQTT_DEFAULT_TOPIC;
int mqtt_port = SECRET_MQTT_PORT;
char mqtt_user[] = SECRET_MQTT_USER;
char mqtt_password[] = SECRET_MQTT_PASSWORD;
int loop_counter = 0;

void setup() {
  Serial.begin(9600);
  saracom.begin();
  saracom.setSimContext(SIM_APN,SIM_PIN);
  saracom.setMqttContext(mqtt_server_ip_address,mqtt_server_domain_name,mqtt_client_id,mqtt_user,mqtt_password,mqtt_port,mqtt_default_topic);
  delay(1000);
}

void loop() {
  loop_counter += 1;
  saracom.displayObjectContext();
  Serial.println(loop_counter);
  delay(6000);
  saracom.modemHardwarePowerOn();
  delay(5000);
  saracom.initializeSimContext();
  if (saracom.modemIsResponsive()) {
    Serial.println("Modem is responsive");
    saracom.initializeSimContext();
    saracom.initializeMqttContext();
    delay(2000);
    saracom.MqttLogin();
    delay(10000);
    saracom.MqttPublish("{\"test\":11}",mqtt_default_topic);
    delay(5000);
    saracom.MqttLogout();
    delay(10000);
  } else {
    Serial.println("Modem is not responsive");
  }
  delay(5000);
  saracom.modemHardwarePowerOff();
  delay(5000);
  if (saracom.modemIsResponsive()) {
    Serial.println("Modem is responsive");
  } else {
    Serial.println("Modem is not responsive");
  }
  delay(5000);
}
