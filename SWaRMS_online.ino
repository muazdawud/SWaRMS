

// #include<Arduino.h>
// #include<WiFi.h>
// #include<WebServer.h>
// #include<ESPmDNS.h>
// #include<LittleFS.h>
// #include<WebSocketsServer.h>


// #define TIMEZONE  
// #define USERNAME "admin"
// #define PASSWORD "0000"


// uint8_t auth_flag = 0;


// const char *ssid = "YieldEx_E3D8";
// const char *password = "25422019yEX";

// const char *mDNS_name = "myESP";


// WebServer server(80);
// WebSocketsServer webSocket(81);
// IPAddress ip;


// void setup(){

//   Serial.begin(115200);

//   Serial.println();
//   Serial.println("******************************************************");
//   Serial.print("Connecting to ");
//   Serial.println(ssid);

//   WiFi.begin(ssid, password);

//   while(WiFi.status() != WL_CONNECTED){
//     delay(500);
//     Serial.print(".");
//   }

//   Serial.println("");
//   Serial.println("WiFi connected");
//   Serial.println("IP address: ");
//   Serial.println(WiFi.localIP());

//   Serial.println("");
//   Serial.println("Starting mDNS Server");

//   while(!MDNS.begin(mDNS_name)){
//     Serial.print(".");
//   }

//   Serial.println("");
//   Serial.println("Started mDNS Server");


//   MDNS.addService("http", "tcp", 80);
// }

// void loop(){


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

//   server.onNotFound(notFound());

//   server.begin();
// }


// // void startWebSocket(){

// //   webSocket.begin();
// //   webSocket.onEvent(webSocketEvent);
// // }


// void startLittleFS(){
//   Serial.printf("\r\n");
//   LittleFS.begin();
//   Serial.print("LittleFS Started.\nContents: \r\n");
//   {
//     Dir dir = LittleFS.openDir("/");
//     while(dir.next()){
//       String filename = dir.fileName();
//       size_t filesize = dir.fileSize();

//       if(filename == "secure"){
//         Serial.printf("\t   FS File: %s, size: %s\r\n", filename.c_str(), formatBytes(filesize).c_str());
//         Dir root = LittleFS.openDir("/secure");
//         while(root.next()){
//           filename = root.fileName();
//           filesize = root.fileSize();

//           Serial.printf("\t\t   FS File: %s, size: %s\r\n", filename.c_str(), formatBytes(filesize).c_str());
//         }
//       }else{
//         Serial.printf("\t   FS File: %s, size: %s\r\n", filename.c_str(), formatBytes(filesize).c_str());
//       }
//     }
//   }
// }


// void streamFile(String filename){
  
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
//     size_t sent = server.streamFile(file, contentType);
//     file.close();
//   }
//   else{
//     server.sendHeader("Location", "/");
//     server.send(303, "text/plain", "Unauthorized Access");

//     auth_flag = 0;
//   }
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
//     server.send(401, "text/plain", "Unauthorized");

//     auth_flag = 0;
    
//     return;
//   }

//   else if(server.arg("username") == USERNAME && server.arg("password") == PASSWORD){
//     server.sendHeader("Location", "/homepage");
//     server.send(200, "text/plain", "Login Successful. Redirecting...");

//     auth_flag = 1;

//     return;
//   }

//   else{
//     server.send(401, "text/plain", "Unauthorized");

//     auth_flag = 0;

//     return;
//   }

// }


// // void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
// //   Serial.printf("\r\n");
// //   switch (type) {
// //     case WStype_DISCONNECTED:             
// //       Serial.printf("[%u] Disconnected!\n", num);
// //       break;
// //     case WStype_CONNECTED: {              
// //         ip = webSocket.remoteIP(num);
// //         Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
// //       }
// //       break;
// //     case WStype_TEXT:{

// //     }                     
      
// //   }
// // }