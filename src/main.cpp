/* 
* I, Justin Borzi, 000798465 certify that this material is my original
* work. No other person's work has been used without due 
* acknowledgement. (Replace with your own name and student number)
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
// 1-Wire sensor communication libary
#include <OneWire.h>
// DS18B20 sensor library
#include <DallasTemperature.h>

// access credentials for WiFi network.
const char *ssid = "Mohawk-IoT";
const char *password = "IoT@MohawK1";

// Pin that the  DS18B20 is connected to
const int oneWireBus = D3;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature DS18B20(&oneWire);

// setup a device address.
DeviceAddress sensor;

// WiFI server.  Listen on port 80, which is the default TCP port for HTTP
WiFiServer server(80);

String address;

void setup()
{

  Serial.begin(115200);
  Serial.println("\nWeb Server Demo");

  Serial.printf("\nConnecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n",
                WiFi.localIP().toString().c_str());

  // Check if device is installed.
  if (!DS18B20.getAddress(sensor, 0))
  {
    Serial.println("No DS18B20 temperature sensors are installed!");
  }
  else
  {
    Serial.println("Found DS18B20 sensor with the address: ");
    for (uint8_t i = 0; i < 8; i++)
    {
      if (sensor[i] < 16)
        Serial.print("0");
      Serial.print(sensor[i], HEX);
    }
    Serial.println();
  }
}

void loop()
{
  WiFiClient client = server.available();

  unsigned long allSeconds = millis() / 1000;
  int secsRemaining = allSeconds % 3600;
  int hours = allSeconds / 3600;
  int minutes = secsRemaining / 60;
  int seconds = secsRemaining % 60;

  // wait for a client (web browser) to connect
  if (client)
  {
    Serial.println("\n>> Client connected");
    Serial.println(">> Client request:");
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        // print information about the request (browser, OS, etc)
        String line = client.readStringUntil('\r');
        Serial.print(line);
        // wait for end of client's request
        if (line.length() == 1 && line[0] == '\n')
        {
          // Initialize a float value.
          float fTemp;

          // Request DS18B20 (devices) for the current temperature(s)
          DS18B20.requestTemperatures();

          // fetch the temperature.  We only have 1 sensor, so the index is 0.
          fTemp = DS18B20.getTempCByIndex(0);

          // Determine the device is connected as to not throw an error.
          if (DS18B20.getAddress(sensor, 0) || fTemp != DEVICE_DISCONNECTED_C)
          {
            // Initialize the string value.
            String tempatureString;

            // Set the variable to the correct output based on tempature.
            if (fTemp <= 10)
            {
              tempatureString = "Cold!";
            }
            else if (fTemp < 15)
            {
              tempatureString = "Cool";
            }
            else if (fTemp < 25)
            {
              tempatureString = "Perfect";
            }
            else if (fTemp < 30)
            {
              tempatureString = "Warm";
            }
            else if (fTemp < 35)
            {
              tempatureString = "Hot";
            }
            else if (fTemp >= 35)
            {
              tempatureString = "Too Hot!";
            }

            // wait 5s (5000ms) before doing this again
            // send some data back to the browser.  Note:  this is not
            // proper HTML!
            char buf[15];
            sprintf(buf, "%02d:%02d:%02d", hours, minutes, seconds);
            client.println("<!DOCTYPE html>");
            client.println("<html>");
            client.println("<head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>");
            client.println("<body>");
            client.println("<h1>Web Server Lab</h1>");
            client.println("<p>Temperature: " + String(fTemp) + "°C or " + tempatureString + "</p>");
            client.println("<p>Device: ");
            for (uint8_t i = 0; i < 8; i++)
            {
              if (sensor[i] < 16)
                client.print("0");
              client.print(sensor[i], HEX);
            }
            client.println("</p>");
            client.println("<p>IP: " + String(WiFi.localIP().toString().c_str()) + "</p>");
            client.println("<p>Time Since Started: " + String(buf) + "</p>");
            client.println("<p>Justin Borzi, 000798465</p>");
            client.println("</body>");
            client.println("</html>");
            Serial.println(">> Response sent to client");
          }
          break;
        }
      }
    }

    // keep read client request data (if any).  After that, we can terminate
    // our client connection
    while (client.available())
    {
      client.read();
    }

    // close the connection:
    client.stop();
    Serial.println(">> Client disconnected");
  }
}