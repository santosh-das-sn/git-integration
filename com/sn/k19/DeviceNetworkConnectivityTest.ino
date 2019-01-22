

/*
    HTTP over TLS (HTTPS) example sketch

    This example demonstrates how to use
    WiFiClientSecure class to access HTTPS API.
    We fetch and display the status of
    esp8266/Arduino project continuous integration
    build.

    Limitations:
      only RSA certificates
      no support of Perfect Forward Secrecy (PFS)
      TLSv1.2 is supported since version 2.4.0-rc1

    Created by Ivan Grokhotkov, 2015.
    This example is in public domain.
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJWT.h>
#include <sha256.h>

const char* ssid = "Santosh";
const char* password = "adidas12GE@";

const char* host = "hackathon021.service-now.com";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "45 8B 95 CD 4B 67 60 83 4E F7 B9 5E 65 BA D8 4F C8 27 32 CC";

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  ArduinoJWT jwtBuilder("s1a2n3t4o5s6h7");
  //jwtBuilder.setPSK("s1a2n3t4o5s6h7");
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url = "/api/now/table/u_machine_surveillance";

  String body = "{\"short_description\":\"Major Incident Occurred\",\"active\":\"true\",\"description\":\"Major Machine failure\"}";
  Serial.println("Device security  enforced.Start jwt generation");
  String encodedJWT = jwtBuilder.encodeJWT(body);
  Serial.println("Generated JWT for device authentication is " +encodedJWT);
  
  Serial.print("requesting URL: ");
  Serial.println(url);

 /* client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");*/
    String resp = httpsPost(url,body,client,encodedJWT);

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
   Serial.println("Response returned from ServiceNow is "+resp);
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}

String httpsPost(String url, String data , WiFiClientSecure client,String JWT) {
  if (client.connect(host, 443)) {
    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: " + (String)host);
    client.println("User-Agent: ESP8266/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/json;");
    client.println("Authorization: Basic YWRtaW46QWxleGFAMzIx;");
    client.println("X-ServiceNow-Device-Auth: Bearer "+JWT+";");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);
    delay(10);
    String response = client.readString();
    int bodypos =  response.indexOf("\r\n\r\n") + 4;
    return response.substring(bodypos);
  }
  else {
    return "ERROR";
  }
}

void loop() {
}