#include <SoftwareSerial.h>

SoftwareSerial sim800l(D2, D3); // RX, TX

const char* serverName = "https://defx-pos-dev.herokuapp.com/one_phones";
String apn = "airtelgprs.com"; // Replace with your network's APN
String apnUser = ""; // If your network requires a username
String apnPass = ""; // If your network requires a password

void setup() {
  Serial.begin(115200);
  sim800l.begin(9600);

  // Initialize the SIM800L
  sendATCommand("AT");
  sendATCommand("AT+CMGF=1"); // Set SMS to text mode
  sendATCommand("AT+CNMI=2,2,0,0,0"); // Configure the module to notify new SMS via serial

  // Connect to GPRS
  connectGPRS();
}

void loop() {
  if(sim800l.available()) {
    String smsContent = sim800l.readString();
    Serial.println(smsContent);

    // Extract phone number and message content
    String phoneNumber = getPhoneNumber(smsContent);
    String messageContent = getMessageContent(smsContent);

    // Send HTTP POST request
    sendPostRequest(phoneNumber, messageContent);
  }
}

void sendATCommand(String command) {
  sim800l.println(command);
  delay(2000);
  while(sim800l.available()) {
    Serial.println(sim800l.readString());
  }
}

void connectGPRS() {
  sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  sendATCommand("AT+SAPBR=3,1,\"APN\",\"" + apn + "\"");

  if(apnUser != "" && apnPass != "") {
    sendATCommand("AT+SAPBR=3,1,\"USER\",\"" + apnUser + "\"");
    sendATCommand("AT+SAPBR=3,1,\"PWD\",\"" + apnPass + "\"");
  }

  sendATCommand("AT+SAPBR=1,1");
  sendATCommand("AT+SAPBR=2,1");
}

String getPhoneNumber(String sms) {
  // Implement your logic to extract the phone number from the SMS content
  // This is just an example assuming a specific SMS format
  int index1 = sms.indexOf("\"");
  int index2 = sms.indexOf("\"", index1 + 1);
  return sms.substring(index1 + 1, index2);
}

String getMessageContent(String sms) {
  // Implement your logic to extract the message content from the SMS
  int index1 = sms.indexOf("\r\n", sms.indexOf("\r\n") + 2);
  return sms.substring(index1 + 2);
}

void sendPostRequest(String phoneNumber, String messageContent) {
  String httpRequestData = "phone_number=" + phoneNumber + "&message_content=" + messageContent;

  sendATCommand("AT+HTTPINIT");
  sendATCommand("AT+HTTPPARA=\"CID\",1");
  sendATCommand("AT+HTTPPARA=\"URL\",\"" + String(serverName) + "\"");
  sendATCommand("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
  sendATCommand("AT+HTTPDATA" + String(httpRequestData.length()) + ",10000");
  sendATCommand("AT+HTTPACTION=1");

  delay(2000);
  sim800l.print(httpRequestData);
  delay(10000); // wait for response

  sendATCommand("AT+HTTPACTION=1"); // Start HTTP POST
  delay(5000); // wait for response

  sendATCommand("AT+HTTPREAD=0,1000"); // Read the data from the server
  delay(1000); // wait for response

  sendATCommand("AT+HTTPTERM");
}