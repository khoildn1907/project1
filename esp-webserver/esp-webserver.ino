#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Web server variables
// Replace with your network credentials
//const char* ssid = "FreshAll";
//const char* password = "";
char* ssid = "khoildn";
char* password = "244466666";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
String outputState();
int8_t cleanSerialBuf();
String pHValue;
String tdsValue;
char serial_data[30];
unsigned long time1;

const char index_html[] PROGMEM = R"rawliteral(
<<!DOCTYPE html><html>
  <head>
    <meta charset="utf-8" />
    <title>Start Webserver</title>

    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <meta name="robots" content="all" />
    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
    <link
      href="https://fonts.googleapis.com/css2?family=Quicksand:wght@400;700&display=swap"
      rel="stylesheet"
    />
    <style>
    html {
      background: url(https://carrickswan.com/wp-content/uploads/2016/04/site-background.jpg);
    }
    body {
      font-family: "Quicksand";
      margin: 0;
    }
    h1,
    h2,
    h3,
    p {
      margin: 0;
    }
    #top {
      background-color: #95a5a6;
      text-align: center;
      padding: 10px;
      border: 1px solid blue;
    }
    #main img {
      display: block;
      margin: 20px auto;
      border: 2px solid #34495e;
      border-radius: 10px 10px 10px 10px;
    }
    #sidebar img {
      display: block;
      max-width:20px;
      max-height:20px;
      margin: 0px 250px;
    }
    #sidebar {
      background-color: #ecf0f1;
      padding: 10px;
    }
    #sidebar h3 {
      text-align: left;
      margin: 5px 0 10px 0;
    }
    #main h4 { 
      text-align: center;
    }
    #main form {
      text-align: center;
    }
    </style>
  </head>
  <body>
    <div class="container">
      <div id="top">
        <h1>Database</h1>
      </div>

      <div class="wrapper">
        <div id="main">
          <h1>Web giám sát</h1>
          <p>
            <img
            width="300"
            src="data:image/png;base64,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"
            />
          </p>

          <h2>Đánh giá chất lượng môi trường</h2>
            <h4>Nồng độ pH trong nước 
              <p id="pHvalue"></p>

              Nồng độ TDS trong nước (ppm)
              <p id="TDSvalue"></p>
            </h4>
        </div>

        <div id="sidebar">
          <h3>Giới thiệu</h3>
          <p>
            Ghé thăm <a href="https://www.epa.gov/waterqualitysurveillance/online-water-quality-monitoring-resources"> tại đây </a> để xem các repo mới và hot cập
            nhật mỗi ngày. <br />

            <img src="http://t2.gstatic.com/licensed-image?q=tbn:ANd9GcTVrJLHaEUyrYtJ_rQSG8S-HuKTHiOMRPZ29oVtB_aYtckXBplzXLXSXkg7vbCsxRl5z21ma-XTl22bC4XJIpA" />
          </p>
        </div>
      </div>
    </div>
  
    <script>
      setInterval(function() {
        // Call a function repetatively with 2 Second interval
        getpHData();
        getTDSData();
      }, 2000); //2000mSeconds update rate
      
      function getpHData() {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("pHvalue").innerHTML = this.responseText;
          }
        };
        xhr.open("GET", "/readpH", true);
        xhr.send();
      }
      function getTDSData() {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("TDSvalue").innerHTML = this.responseText;
          }
        };
        xhr.open("GET", "/readTDS", true);
        xhr.send();
      }
    </script>
  </body>
</html>
)rawliteral";


void setup()
{
  Serial.begin(9600);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
    // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  server.on("/readpH", HTTP_GET, [](AsyncWebServerRequest *request){
    String getpHValue = pHValue;
    //Serial.println(getpHValue);
    request->send(200, "text/plain", getpHValue);
  });

   server.on("/readTDS", HTTP_GET, [](AsyncWebServerRequest *request){
    String getTDSValue = tdsValue;
    //Serial.println(getTDSValue);
    request->send(200, "text/plain", getTDSValue);
  });
  
  // Start server
  server.begin();
}

void loop() {
  int i = 0;
  //char tdsValue[]="0.0";
  //char pHValue[]="0.0";
  memset(serial_data,0,sizeof(serial_data));
  time1 = millis();
  if (Serial.available() > 0)
  {
    while((millis() - time1 < 1000))
    {
      if (Serial.available() > 0)
      { 
        serial_data[i] = Serial.read();
        i++;
        if( strstr((char*)serial_data,"end")!=NULL){
          break;
        }

      } 
    }
    if( strstr((char*)serial_data,"end")!=NULL){
      char * token = strtok(serial_data,";");
      tdsValue = token;
      if( token != NULL ) {
         token = strtok(NULL, ";");
      }
      pHValue = token;
    }
  }
}
int8_t cleanSerialBuf()
{
  unsigned long time2 = millis();
  int Dummy_buf;
  int ret = -1;
  while((millis() - time2 < 2000))
  {
    if (Serial.available() > 0)
    { 
      Dummy_buf = Serial.read();
    } else { return 0; }
  }
  return ret;
}
