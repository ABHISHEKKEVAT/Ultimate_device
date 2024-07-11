#include "SIM800L.h"

SIM800L::SIM800L(int rxPin, int txPin) 
    : sim800(rxPin, txPin), serverName("http://defx-pos-dev.herokuapp.com/one_phones") {}

void SIM800L::begin(long baudRate) {
    sim800.begin(baudRate);
}

void SIM800L::initialize() {
    sendATCommand("AT");
    sendATCommand("AT+CMGF=1"); // Set SMS to text mode
    sendATCommand("AT+CNMI=3,2,0,1,0"); // Configure the module to notify new SMS via serial
}

void SIM800L::connectGPRS(const String& apn, const String& apnUser, const String& apnPass) {
    sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
    sendATCommand("AT+SAPBR=3,1,\"APN\",\"" + apn + "\"");
    if (apnUser != "" && apnPass != "") {
        sendATCommand("AT+SAPBR=3,1,\"USER\",\"" + apnUser + "\"");
        sendATCommand("AT+SAPBR=3,1,\"PWD\",\"" + apnPass + "\"");
    }
    sendATCommand("AT+SAPBR=1,1");
    sendATCommand("AT+SAPBR=2,1");
}

void SIM800L::handleIncomingMessages() {
    if (sim800.available()) {
        String smsContent = sim800.readString();
        Serial.println(smsContent);

        // Extract phone number and message content
        String phoneNumber = getPhoneNumber(smsContent);
        String messageContent = getMessageContent(smsContent);

        Serial.println("Extracted phone number: " + phoneNumber);
        Serial.println("Extracted message content: " + messageContent);

        // Send HTTP POST request
        sendPostRequest(phoneNumber, messageContent);
    }
}

void SIM800L::sendPostRequest(const String& phoneNumber, const String& messageContent) {
    DynamicJsonDocument jsonDoc(256);
    jsonDoc["phone_from"] = "8849384442";  // Replace with the actual sender phone number
    jsonDoc["phone_to"] = phoneNumber;
    jsonDoc["message"] = messageContent;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    sendATCommand("AT+HTTPINIT");
    sendATCommand("AT+HTTPPARA=\"CID\",1");
    sendATCommand("AT+HTTPPARA=\"URL\",\"" + String(serverName) + "\"");
    sendATCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    sendATCommand("AT+HTTPDATA=" + String(jsonString.length()) + ",20000");

    delay(4000);
    sim800.println(jsonString);
    Serial.println("HTTP request data" + jsonString);
    delay(7000); // wait for response

    sendATCommand("AT+HTTPACTION=1"); // Start HTTP POST
    delay(5000); // wait for response

    sendATCommand("AT+HTTPREAD"); // Read the data from the server
    delay(2000); // wait for response

    sendATCommand("AT+HTTPTERM");
}

void SIM800L::sendATCommand(const String& command) {
    sim800.println(command);
    delay(2000);
    while (sim800.available()) {
        Serial.println(sim800.readString());
    }
}

String SIM800L::getPhoneNumber(const String& sms) {
    int index1 = sms.indexOf("\"");
    int index2 = sms.indexOf("\"", index1 + 1);
    return sms.substring(index1 + 1, index2);
}

String SIM800L::getMessageContent(const String& sms) {
    int index1 = sms.indexOf("\r\n", sms.indexOf("\r\n") + 2);
    return sms.substring(index1 + 2);
}
