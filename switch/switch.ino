#include <ESP8266WiFi.h>

// wifi, GPIO settings
const char *ssid = "ATT9eg4Iys";
const char *password = "a5dvbgn3db5n";
int relayInput1 = 2;
int relayInput2 = 0;
bool operating = false;

// open port 80
WiFiServer server(80);

String header;

//set timeout time
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;


// connect wifi, initialization
void setup() {
	Serial.begin(115200);
	pinMode(relayInput1, OUTPUT);
	pinMode(relayInput2, OUTPUT);

	digitalWrite(relayInput1, HIGH);
	digitalWrite(relayInput2, HIGH);

	Serial.print("Connecting...");
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected.");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	server.begin();
}


// web
void loop(){
	WiFiClient client = server.available();
	if (client) {
		Serial.print("New Client Connecting...");
		String currentLine = "";
		currentTime = millis();
		previousTime = currentTime;
		while (client.connected() && currentTime - previousTime <= timeoutTime) {
		currentTime = millis();         
		if (client.available()) {
			char c = client.read();
			header += c;
			if (c == '\n') {
			if (currentLine.length() == 0) {
				client.println("HTTP/1.1 200 OK");
				client.println("Content-type:text/html");
				client.println("Connection: close");
				client.println();

				if (header.indexOf("GET /power") >= 0) {
					if (operating == false) {
						Serial.print("Power");
						operating = true;
						digitalWrite(relayInput1, LOW);
						delay(1000);
						digitalWrite(relayInput1, HIGH);
						operating = false;
					}
				} else if (header.indexOf("GET /forceshutdown") >= 0) {
					if (operating == false) {
						Serial.print("Shutdown");
          				operating = true;
						digitalWrite(relayInput1, LOW);
						delay(1000);
						delay(1000);
						delay(1000);
						delay(1000);
						delay(1000);
						digitalWrite(relayInput1, HIGH);
          				operating = false;
					}
				} else if (header.indexOf("GET /forcerestart") >= 0) {
					if (operating == false) {
						Serial.print("Restart");
						operating = true;
						digitalWrite(relayInput2, LOW);
						delay(1000);
						digitalWrite(relayInput2, HIGH);
						operating = false;
					}
				}
				client.println("<!DOCTYPE html><html>");
				client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
				client.println("<link rel=\"icon\" href=\"data:,\">");
				client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
				client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
				client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
				client.println("</style></head>");
				client.println("<body><h1>Wireless PC Power Control</h1>");
				client.println("<p> PC Power </p>"); 
				client.println("<p><a href=\"/power\"><button class=\"button\">Power</button></a></p>");
				client.println("<p><a href=\"/forceshutdown\"><button class=\"button\">Force Shutdown</button></a></p>");
				client.println("<p><a href=\"/forcerestart\"><button class=\"button\">Restart</button></a></p>");
				client.println("</body></html>");
				client.println();
				break;
			} else {
				currentLine = "";
			}
			} else if (c != '\r') {
			currentLine += c;
			}
		}
		}
		header = "";
		client.stop();
		Serial.println("Client disconnected.");
		Serial.println("");
	}
}
