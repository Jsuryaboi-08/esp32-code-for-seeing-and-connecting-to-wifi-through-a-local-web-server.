#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "12345";
const char* password = "440surya";

WebServer server(80);

String wifiScanResults = ""; // To store WiFi scan results

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  Serial.println("Try Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", HTTP_GET, handle_root);
  server.on("/getWiFiScanResults", HTTP_GET, handle_getWiFiScanResults); // New endpoint for WiFi scan results
  server.on("/connectWiFi", HTTP_POST, handle_connectWiFi); // New endpoint for SSID and password submission
  
  server.begin();
  Serial.println("HTTP server started");
  delay(100);
}

void loop() {
  server.handleClient();
}

String HTML = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP32 WiFi Scan and Connect</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <button onclick="refreshWiFiScan()">Refresh WiFi Scan</button>
  <p>WiFi Scan Results:</p>
  <pre id="wifiScanResults">%WIFI_SCAN_RESULTS%</pre>
  <br>
  <form action="/connectWiFi" method="post">
    SSID: <input type="text" name="ssid"><br>
    Password: <input type="password" name="password"><br>
    <input type="submit" value="Connect to WiFi">
  </form>
  <script>
    function refreshWiFiScan() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("wifiScanResults").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/getWiFiScanResults", true);
      xhttp.send();
    }
  </script>
</body></html>)rawliteral";

void handle_root() {
  String htmlWithWiFiScan = HTML;
  htmlWithWiFiScan.replace("%WIFI_SCAN_RESULTS%", "");  // Initially, empty content
  
  server.send(200, "text/html", htmlWithWiFiScan);
}

void handle_getWiFiScanResults() {
  wifiScanResults = performWiFiScan(); // Refresh WiFi scan results
  server.send(200, "text/plain", wifiScanResults);
}

String performWiFiScan() {
  String result = "";
  
  Serial.println("WiFi Scan start");
  int n = WiFi.scanNetworks();
  Serial.println("WiFi Scan done");
  
  if (n == 0) {
    result = "No networks found";
  } else {
    result += n;
    result += " networks found\n";
    result += "Nr | SSID | RSSI | CH | Encryption\n";
    for (int i = 0; i < n; ++i) {
      result += String(i + 1);
      result += " | ";
      result += WiFi.SSID(i);
      result += " | ";
      result += WiFi.RSSI(i);
      result += " | ";
      result += WiFi.channel(i);
      result += " | ";
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:
          result += "open";
          break;
        case WIFI_AUTH_WEP:
          result += "WEP";
          break;
        case WIFI_AUTH_WPA_PSK:
          result += "WPA";
          break;
        case WIFI_AUTH_WPA2_PSK:
          result += "WPA2";
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          result += "WPA+WPA2";
          break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          result += "WPA2-EAP";
          break;
        case WIFI_AUTH_WPA3_PSK:
          result += "WPA3";
          break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
          result += "WPA2+WPA3";
          break;
        case WIFI_AUTH_WAPI_PSK:
          result += "WAPI";
          break;
        default:
          result += "unknown";
      }
      result += "\n";
    }
  }
  Serial.println("WiFi Scan results:");
  Serial.println(result);
  return result;
}

void handle_connectWiFi() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  
  server.send(200, "text/plain", "Connected to WiFi. IP: " + WiFi.localIP().toString());
}
