

// #include<Arduino.h>
// #include<WiFi.h>
// #include<WebServer.h>
// #include<ESPmDNS.h>
// #include<LittleFS.h>
// #include<WebSocketsServer.h>


// #define USERNAME "admin"
// #define PASSWORD "0000"


// uint8_t auth_flag = 0;


// const char *ssid = "YieldEx_E3D8";
// const char *password = "25422019yEX";


// const char *mDNS_name = "myESP";


// const char* NTPServerName = "time.nist.gov";
// const int NTP_PACKET_SIZE = 48;
// byte NTPBuffer[NTP_PACKET_SIZE];
// uint32_t timeUNIX = 0;
// unsigned long prevActualTime = 0;
// uint16_t intervalNTP = 


// WebServer server(80);
// WebSocketsServer webSocket(81);
// WiFiUDP UDP;
// IPAddress ip;
// IPAddress timeServerIP;


// String date = "";
// String time = "";
// String location = "Bayero University Kano - Rimin Gata";
// String ambientTemp = "";
// String ambientHum = "";
// String innerTemp = "";
// String innerHum = "";


// void setup(){

//   startLittleFS();

//   WiFi.begin(ssid, password);
//   while(WiFi.status() != WL_CONNECTED){
//     delay(500);
//   }

//   startUDP();
//   startWebServer();
//   startWebSocket();

//   WiFi.hostByName(NTPServerName, timeServerIP);

//   while(!MDNS.begin(mDNS_name)){}
// }

// void loop(){

//   server.handleClient();
//   webSocket.loop();
//   MDNS.update();

//   if(updateTime){
//     sendNTPpacket(timeServerIP);
//   }

//   uint32_t timeL = getTime();
//   if(timeL){
//     timeUNIX = timeL;
//   }

//   uint32_t actualTime = timeUNIX;
//   if (actualTime != prevActualTime && timeUNIX != 0) {
//     prevActualTime = actualTime;
//     time =  String(getHours(actualTime)) + ":" + String(getMinutes(actualTime));
//   }  
// }


// void startWebServer(){

//   server.on("/", HTTP_GET, []{
//     if(!streamFile("/index.html")){
//       streamFile("/notFound.html");
//     }
//   });

//   server.on("/$login", HTTP_POST, loginAuth);

//   server.on("/HomePage", HTTP_GET, []{
//     if(!streamFile("/homepage.html")){
//       streamFile("/notFound.html");
//     }
//   });

//   server.onNotFound(notFound);

//   server.begin();
// }


// void startWebSocket(){

//   webSocket.begin();
//   webSocket.onEvent(webSocketEvent);
// }


// void startLittleFS(){

//   LittleFS.begin();
// }


// bool streamFile(String filename){
  
//   if(filename == "/index.html"){
//     auth_flag = 1;
//   }

//   if(auth_flag){

//     String path = filename;

//     String contentType = "";

//     if(filename.endsWith(".html") || filename.endsWith(".hml")){ contentType = "text/html";}
//     else if(filename.endsWith(".css")){ contentType = "text/css";}
//     else if(filename.endsWith(".js")){ contentType = "application/javascript";}
//     else{ contentType = "text/plain";}

//     File file = LittleFS.open(path, "r");

//     if(!file){
//       return 0;
//     }

//     size_t sent = server.streamFile(file, contentType);
//     file.close();

//      if(filename == "/index.html"){
//       auth_flag = 0;
//     }

//     return 1;
//   }
//   else{
//     server.sendHeader("Location", "/");
//     server.send(303, "text/plain", "Unauthorized Access");

//     auth_flag = 0;
//   }

//   return 0;
// }


// void notFound() {
  
//   if(!auth_flag){
    
//     auth_flag = 1;
//   }

//   streamFile("/notFound.html");

//   auth_flag = 0;
// }


// void loginAuth(){

//   if(!server.hasArg("username") || !server.hasArg("password") || server.arg("username") == NULL || server.arg("password") == NULL){ 
//     server.send(400, "text/plain", "Unauthorized");

//     auth_flag = 0;
    
//     return;
//   }

//   else if(server.arg("username") == USERNAME && server.arg("password") == PASSWORD){
//     server.sendHeader("Location", "/HomePage");
//     server.send(302, "text/plain", "Login Successful. Redirecting...");

//     auth_flag = 1;

//     return;
//   }

//   else{
//     server.send(400, "text/plain", "Unauthorized");

//     auth_flag = 0;

//     return;
//   }

// }


// void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

//   switch (type) {
//     case WStype_DISCONNECTED:             
//       break;
//     case WStype_CONNECTED: {              
//         ip = webSocket.remoteIP(num);
//       }
//       break;
//     case WStype_TEXT:{
//       String text = "";
//       char *data = (char*)payload;
      
//       if(strcmp(data, "Refresh") == 0){

//         text = "Station Info:" + date + "," + time + "," + location + "," + ambientTemp + "," + ambientHum + "," + String(UNIVERSAL_LOCK) + "," + String(W_Threshold) + "," + String(weight) + "," + innerTemp + "," + innerHum + "," + String(fan_Flag);
//         webSocket.sendTXT(num, text.c_str());
//       }
//     }                     
//   }
// }


// void startUDP() {
//   UDP.begin(123);
// }


// uint32_t getTime() {
//   if (UDP.parsePacket() == 0) {
//     return 0;
//   }
//   UDP.read(NTPBuffer, NTP_PACKET_SIZE);
//   uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
 
//   const uint32_t seventyYears = 2208988800UL;

//   uint32_t UNIXTime = NTPTime - seventyYears;
//   return UNIXTime;
// }


// void sendNTPpacket(IPAddress& address) {
//   memset(NTPBuffer, 0, NTP_PACKET_SIZE);  

//   NTPBuffer[0] = 0b11100011;

//   UDP.beginPacket(address, 123);
//   UDP.write(NTPBuffer, NTP_PACKET_SIZE);
//   UDP.endPacket();
// }


// inline int getMinutes(uint32_t UNIXTime) {
//   return UNIXTime / 60 % 60;
// }


// inline int getHours(uint32_t UNIXTime) {
//   return (UNIXTime / 3600 % 24) + 1;
// }