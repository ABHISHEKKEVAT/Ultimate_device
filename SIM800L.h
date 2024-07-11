#ifndef SIM800L_H
#define SIM800L_H

#include <SoftwareSerial.h>
#include <ArduinoJson.h>

class SIM800L {
public:
    SIM800L(int rxPin, int txPin);
    void begin(long baudRate);
    void initialize();
    void connectGPRS(const String& apn, const String& apnUser = "", const String& apnPass = "");
    void handleIncomingMessages();
    void sendPostRequest(const String& phoneNumber, const String& messageContent);

private:
    SoftwareSerial sim800;
    const char* serverName;
    void sendATCommand(const String& command);
    String getPhoneNumber(const String& sms);
    String getMessageContent(const String& sms);
};

#endif
