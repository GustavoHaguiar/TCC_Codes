/*
 * Código utilizado para todos os testes com esp32, já foi compilado e não apresentou bugs
 * 
 * Para a versão final, algumas características devem ser alteradas:
 *  - Enviar potência para o app ao finalizar o banho
 *  
 *  Última revisão feita em 06-11-24 por Gustavo Aguiar
 */

// Libraries
#include <WiFiManager.h>
#include <WebServer.h>
#include <HTTPClient.h>

// Instância WebServer
WebServer server(80);
// Instância WiFiManager
WiFiManager wifiManager;

#define pinRST 26
#define BUZ 13 // Buzzer
#define V_REF 34 // Tensão na carga
#define ITR 12 // Interrupção

// define variables
unsigned long startTime = 0,        // initial time
              interval = 0,         // time for shower
              intervalAlert = 0,    // time for alert
              ctrStart;             // Tempo em milissegundos do início do banho
              
int valRST,                         // value of pin RESET
    ctrVref,                        // Tensão mínima para considerar o chuveiro como ligado
    dtaBathTime = 30,               // Tempo de banho, por enquanto mantido estático, será atualizado via app
    dtaPwr = 5500,                  // Configuráveis via app
    cnt = 0;
    
float ctrMidPower,                  // Potência média em que o chuveiro operou em determinado banho 
      pwr;                          // Potência

bool ctrOn,                         // Indica se o chuveiro já se encontra ligado
        dtaActive,                  // Indica se o buzzer será utilizado, atualizado via app
        timerSet = false,           // time defined by user
        dtaAutoOff,                 // Indica se o final do banho será forçado, via app
      active = true;                // variable as verify if has energy on shower (on/off)

String idShower2,                   // variable as armazened datas: id and temporizer of shower
       urlIp = "192.168.43.144:3001";// url do backend

// function to create a main screen on route GET of the aplication
void handleRoot() {
  server.send(200, "text/plain", "Servidor ESP32 em funcionamento");
  Serial.println("Recebeu uma solicitação HTTP GET");
}

// function to create a post route of the aplication
void handlePost() {
  if (server.hasArg("idShower")) {              // verify as the argument has the parameter "idShower"
    String idShower = server.arg("idShower"); 
    idShower2 = idShower;
    Serial.println(idShower);
    
    if (server.hasArg("alert")) {               // verify as the argument has the parameter "alert"
      String alert = server.arg("alert");
      dtaActive = alert;
      Serial.println(alert);
      // alertTemp = alert;
    }

    if (server.hasArg("pwrShower")) {                // verify as the argument has the parameter "power"
      dtaPwr = server.arg("pwrShower").toInt();
      Serial.println(dtaPwr);
    }
    
    if (server.hasArg("temp")) {                // verify as the argument has the parameter "temp"
      String temp = server.arg("temp");
      Serial.println(temp);
      dtaAutoOff = temp;
    }
    
    if (server.hasArg("time")) {                // verify as the argument has the parameter "time"
      String times = server.arg("time");
      dtaBathTime = (times.toInt())*60;

      
      interval = times.toInt() * 60 * 1000;             // Get the time in milisseconds to shower
      intervalAlert = (times.toInt() - 1) * 60 * 1000;  // Get the time in milisseconds to alert
      Serial.print("Tempo para emitir alerta: ");
      Serial.println(intervalAlert);
      Serial.print("Tempo para fim do banho: ");
      Serial.println(interval);
      startTime = millis();                             // Start the time to counter
      timerSet = true;
    }

    server.send(200, "text/plain", "Dados recebidos com sucesso");          // send 200 code if everything ok
  } else {
    server.send(400, "text/plain", "Parâmetro 'idShower' não encontrado");  // send 400 code if everything isn't ok
  }
}

// function as send Ip to backend app
void sendIPAddress() {
  if (WiFi.status() == WL_CONNECTED) {                        // verify if WiFi was conected
    HTTPClient http;                                          // Start a intance of HTTPClient class
    String localIp = WiFi.localIP().toString();               // Get the shower ip and save in a string
    String url = "http://" + urlIp + "/esp32/sendIp";           // save the url of backend app
    String payload = "{\"ip\":\"" + localIp + "\"}";          // save the payload of post

    http.begin(url);                                          // start the conection with backend
    http.addHeader("Content-Type", "application/json");       // add cors
    http.setTimeout(10000);                                   // set 10 seconds to end of post verify

    Serial.print("Enviando IP para: ");
    Serial.println(url);
    Serial.print("Payload: ");
    Serial.println(payload);

    int httpResponseCode = http.POST(payload);                // send post

    if (httpResponseCode > 0) {                               // verify if has response code of the post
      String response = http.getString();                     // get the response code and save in a string
      Serial.print("Código de resposta: ");
      Serial.println(httpResponseCode);
      Serial.print("Resposta: ");
      Serial.println(response);
    } else {
      Serial.print("Erro ao registrar IP: ");
      Serial.println(httpResponseCode);
      Serial.print("Erro HTTP: ");
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end();                                             // end http conection
  } else {
    Serial.println("WiFi não está conectado");
  }
}

// function to send logs of bath
void sendLogs(String idShowerLog, int timeLog, int powerLog) {
  if (WiFi.status() == WL_CONNECTED) {                            // verify if WiFi was conected
    HTTPClient http;                                              // Start a intance of HTTPClient class
    String url = "http://" + urlIp + "/postLog";                    // save the url to post log (save the route)
    String payload = "{\"idShower\":\"" + idShowerLog + "\", \"time\":\"" + timeLog + "\", \"power\":\"" + powerLog + "\"}";

    http.begin(url);                                              // start the conection with backend
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);                                       // set 10 seconds to end of post verify

    Serial.print("Enviando Log para: ");
    Serial.println(url);
    Serial.print("Payload: ");
    Serial.println(payload);

    int httpResponseCode = http.POST(payload);                    // send post

    if (httpResponseCode > 0) {                                   // verify if has response code of the post
      String response = http.getString();                         // get the response code and save in a string
      Serial.print("Código de resposta: ");
      Serial.println(httpResponseCode);
      Serial.print("Resposta: ");
      Serial.println(response);
    } else {
      Serial.print("Erro ao enviar Log: ");
      Serial.println(httpResponseCode);
      Serial.print("Erro HTTP: ");
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end();                                                   // end http conection
  } else {
    Serial.println("WiFi não está conectado");
  }
}


void setup() {

  // Definições de entrada e saída de cada pino
  pinMode(BUZ, OUTPUT);
  pinMode(V_REF, INPUT);
  pinMode(ITR,OUTPUT);
  pinMode(pinRST, INPUT);

  // deixa interrupt desligado
  digitalWrite(ITR, HIGH);

  // Inicializa comunicação serial, para fins de debug
  Serial.begin(115200);

  // Próximas linhas são instruções de teste e devem ser retiradas na versão final
  dtaActive = true;
  dtaAutoOff = true;
  ctrVref = 744;

  
  // Start the WiFi AP and start a configuration portal
  if (!wifiManager.autoConnect("ESP32ConfigAP", "")) {
    Serial.println("Falha ao conectar e timeout atingido");
    // Reinicia o ESP
    ESP.restart();
  }

  // WiFi conected
  Serial.println("Conectado à rede WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Send ip to backend
  sendIPAddress();

  // Configure the web server
  server.on("/", HTTP_GET, handleRoot);             // root server (get)
  server.on("/update", HTTP_POST, handlePost);      // server to recive the configuration of bath

  server.begin();                                   // start server
}

/*
 *  Verifica pino de entrada do sinal V_REF (ver esquemático), retorna se o valor de tensão 
 *  é maior que o valor mínimo estipulado por ctrVref.
 */
bool checkBath(){

  cnt = 0;

  for(int i = 0; i < 60; i++){
    cnt += digitalRead(V_REF);
    delay(1);
  }
  Serial.println(cnt);

  updateData();
  
  return cnt > 0; // Se tensão de pico for maior que o parâmetro mínimo, retorna true
}

/*
 * Ativa buzzer se as condições predeterminadas forem atingidas
 */
void buzzer(){
  Serial.println("buzzer");
    unsigned long _now = millis(); // Momento atual em milissegundos

    // Verifica se o tempo de banho foi excedido e se o buzzer está ativo 
    if(_now - ctrStart >= (dtaBathTime - 60)* 1000 && dtaActive) {

      while(millis() - _now < 700) {Serial.println("Buzzer is on!"); digitalWrite(BUZ, HIGH);}  // Liga buzzer por 0,7 segundos

      digitalWrite(BUZ, LOW); // Desliga buzzer
    
      dtaActive = false;  // Desativa sinal do buzzer para evitar loops infinitos
    }
    
}

/*
 * Define as etapas para encerramento do banho, caso o tempo seja maior que o permitido
 */
void endBath() {
  Serial.println("Ending..");
  
  if(millis() - ctrStart < dtaBathTime * 1000) return; // Sai da função se o tempo estiver no limite
      Serial.println("OK");
      
  do {
    digitalWrite(ITR, LOW); // Ativa interrupção do Attiny85
    Serial.println("Waiting..");
    delay(1000); // Aguarda um segundo
    digitalWrite(ITR, HIGH); // Desativa interrupção do Attiny85
  }
  while(checkBath()); // Repete enquanto o chuveiro não for desligado

  updateData(); // Atualiza consumo
  
  Serial.println("DONE");
}

/*
 * Atualiza dados de consumo de energia do chuveiro a cada banho tomado
 * 
 * Posteriormente, esses dados são enviados para o app
 */
void updateData() {
  pwr = (cnt / 60.0) * dtaPwr;
  ctrVref = dtaPwr*0.08;
}

/*
 * Loop infinito de execução
 */
void loop() {
  // Verifica se há um banho ativo
  if(checkBath()) {  //////// aqui foi atualizado para testes
    
    // inicia o servidor WiFi somente quando o banho estiver ligado
    server.handleClient();
    
    // Caso ctrOn esteja em zero..
    if(!ctrOn) {
      ctrStart = millis(); // Atualiza valor de ctrStart
      ctrOn = true; // Define ctrOn como verdadeiro
    }
    buzzer(); // Liga buzzer, se necessário
    if(dtaAutoOff) endBath(); // Desativa chuveiro se as condições para isto forem atendidas
  }
    
  else {

    valRST = digitalRead(pinRST);                   // read the state of reset pin
  
    // Reinicia o WiFi do ESP32
    if (!valRST) {                                  // control the button reset
      Serial.print("valRST: ");
      Serial.println(valRST);
      wifiManager.resetSettings();
      ESP.restart();
    }
    
   if (timerSet && (millis() - startTime >= interval)) {                       // verify if the bath is ended
      Serial.println("Tempo passou. Enviando sinal...");
      Serial.println(idShower2);
      Serial.println(dtaActive);
      Serial.println(dtaAutoOff);
      Serial.println(pwr);
      sendLogs(idShower2, (millis() - ctrStart)/1000, pwr); // send the logs of bath
        
      // Reset the temporizer
      startTime = millis();
    }
    
    if(!dtaAutoOff && ctrOn) updateData(); // Atualiza consumo se usuário desligar o chuveiro dentro do prazo
    
    ctrOn = false; // Define ctrOn como falso
    dtaActive = true; // Volta a permitir acionamento do buzzer

  }
  digitalWrite(ITR, HIGH); // Como padrão, mantém sinal de interrupção em zero
  delay(100); // Aguarda 100ms e repete o processo, poderá ser retirado na versão final
}
