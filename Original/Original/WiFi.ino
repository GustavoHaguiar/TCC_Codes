//void WiFi_client(char *ssid, char *password){
//  AsyncWebServer server(80);
//
//  Serial.begin(115200);  
//  
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//      delay(1000);
//  }
//  
//  Serial.println("Conectado ao WiFi");
//  Serial.println(WiFi.localIP());
//
//  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//        request->send(SPIFFS, "/index.html", "text/html");
//        Serial.println("Recebeu uma solicitação http");
//  });
//
//  server.on("/WiFi", HTTP_POST, [](AsyncWebServerRequest *request){
//    Serial.println("Recebeu uma solicitação POST");
//
//    if (request->hasParam("ssid", true) && request->hasParam("senha", true)) {
//      String valorRecebido1 = request->getParam("ssid", true)->value();
//      Serial.println("Valor recebido1: " + valorRecebido1);
//      String valorRecebido2 = request->getParam("senha", true)->value();
//      Serial.println("Valor recebido2: " + valorRecebido2);  
//        
//        
//      request->send(200);
//    }
//    else {
//      Serial.println("Valores não presentes na solicitação POST");
//      request->send(400);
//    }
//
//    // Adicionando cabeçalhos CORS
//    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "Recebido");
//    response->addHeader("Access-Control-Allow-Origin", "*");
//  });
//
//   server.begin();
//}
