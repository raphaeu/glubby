#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <EEPROM.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define YOUR_AP_WIFI_SSID "glubby"//"casa_dos_rafas"
#define TZ  PSTR("<-03>3")

struct tm *tm_struct;


String msg;
//#define IN1 14
//#define IN2 12
//#define IN3 13
//#define IN4 15

#define GP_ACTIVE_BUZZER 12
#define GP_RELE_FOOD 13
#define GP_RELE_LUZ 15
String IP_local;
unsigned long previousMillis = 0; 
const long interval = 1000; 

struct StructConfig {
  char ssid[30];
  char password[30];
  int timer;
  
  int h1h;
  int h1m;

  int h2h;
  int h2m;

  int h3h;
  int h3m;

  int h4h;
  int h4m;

  int sluz;

};
StructConfig MyConfig;

const String headPage = "<html>\
  <head>\
    <title>Glubby - Alimentador automatico de pet</title>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.1/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-4bw+/aepP/YC94hEpVNVgiZdgIC5+VKNBQNGCHeKRQN+PtmoHDEXuppvnDJzQIu9\" crossorigin=\"anonymous\">\
    <style>\
      a,a:focus,a:hover,body{color:#fff}.btn-secondary,.btn-secondary:focus,.btn-secondary:hover{color:#333;text-shadow:none;background-color:#fff;border:.05rem solid #fff}.mastfoot,.nav-masthead .nav-link{color:rgba(255,255,255,.5)}body,html{height:100%;background-color:#333}body{display:-ms-flexbox;display:-webkit-box;display:flex;-ms-flex-pack:center;-webkit-box-pack:center;justify-content:center;text-shadow:0 .05rem .1rem rgba(0,0,0,.5);box-shadow:inset 0 0 5rem rgba(0,0,0,.5)}.cover-container{max-width:42em}.masthead{margin-bottom:2rem}.masthead-brand{margin-bottom:0}.nav-masthead .nav-link{padding:.25rem 0;font-weight:700;background-color:transparent;border-bottom:.25rem solid transparent}.nav-masthead .nav-link:focus,.nav-masthead .nav-link:hover{border-bottom-color:rgba(255,255,255,.25)}.nav-masthead .nav-link+.nav-link{margin-left:1rem}.nav-masthead .active{color:#fff;border-bottom-color:#fff}@media (min-width:48em){.masthead-brand{float:left}.nav-masthead{float:right}}.cover{padding:0 1.5rem}.cover .btn-lg{padding:.75rem 1.25rem;font-weight:700}\
    </style>\
  </head>\
  <body>\
  <div class=\"cover-container d-flex h-100 p-3 mx-auto flex-column\">\
  <header class=\"masthead mb-auto\">\
  <div class=\"inner\">\
    <h3 class=\"masthead-brand\">Glubby 1.0</h3>\
    <nav class=\"nav nav-masthead justify-content-center\">\
      <a class=\"nav-link\" id=\"div_home\" href=\"/\">Home</a>\
      <a class=\"nav-link\" id=\"div_config\" href=\"/config\">Configurar</a>\
    </nav>\
  </div>\
  </header>\
  <main role=\"main\" class=\"inner cover\">";
const String footPage = "\
  </main>\
  <footer class=\"mastfoot mt-auto\">\
    <div class=\"inner\">\
      <p>Desenvolvido por <a href=\"mailto:raphaeu.aguiar@gmail.com\">Rafael Aguiar</a></p>\
    </div>\
  </footer>\
  </body>\
  </html>";
 
//https://javl.github.io/image2cpp/
const unsigned char dog_bitmap [] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x01, 0xff, 0xff, 0x00, 
  0x07, 0xf0, 0x1f, 0xc0, 0x0e, 0xe0, 0x0e, 0xe0, 0x1c, 0xc0, 0x06, 0x70, 0x39, 0x80, 0x03, 0x38, 
  0x31, 0x80, 0x03, 0x18, 0x63, 0x00, 0x01, 0x8c, 0xe3, 0x00, 0x01, 0x8e, 0xc3, 0x00, 0x01, 0x86, 
  0xc6, 0x08, 0x20, 0xc6, 0xc6, 0x18, 0x70, 0xc6, 0x7c, 0x00, 0x20, 0x7c, 0x7c, 0x00, 0x00, 0x7c, 
  0x1c, 0x00, 0x00, 0x70, 0x0c, 0x07, 0xc0, 0x60, 0x18, 0x0f, 0xe0, 0x30, 0x18, 0x0c, 0x60, 0x30, 
  0x18, 0x0c, 0x60, 0x30, 0x18, 0x0f, 0xe0, 0x30, 0x0c, 0x07, 0xc0, 0x60, 0x0c, 0x03, 0x80, 0x60, 
  0x0e, 0x01, 0x00, 0xe0, 0x07, 0x01, 0x01, 0xc0, 0x03, 0xc1, 0x07, 0x80, 0x00, 0xff, 0xfe, 0x00, 
  0x00, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


ESP8266WebServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(GP_ACTIVE_BUZZER, OUTPUT);
  pinMode(GP_RELE_FOOD, OUTPUT);
  digitalWrite(GP_RELE_FOOD, LOW);
  pinMode(GP_RELE_LUZ, OUTPUT);
  digitalWrite(GP_RELE_LUZ, LOW);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  displayInfo("iniciando", "Carregando config");

  EEPROM.get( 0, MyConfig );

  Serial.println( "Carregado dados..." );
  Serial.print( "SSID:");
  Serial.println( MyConfig.ssid );
  Serial.print( "Password:");
  Serial.println( MyConfig.password );

  
  Serial.println( "Alimentacao:");
  Serial.print( "Horario 1:");
  Serial.print( MyConfig.h1h);
  Serial.print( ":");
  Serial.println( MyConfig.h1m);
  
  Serial.print( "Horario 2:");
  Serial.print( MyConfig.h2h);
  Serial.print( ":");
  Serial.println( MyConfig.h2m);
    
  Serial.print( "Horario 3:");
  Serial.print( MyConfig.h3h);
  Serial.print( ":");
  Serial.println( MyConfig.h3m);
  
  Serial.print( "Horario 4:");
  Serial.print( MyConfig.h4h);
  Serial.print( ":");
  Serial.println( MyConfig.h4m);

  Serial.print( "Tempo de alimentacao:");
  Serial.println( MyConfig.timer);
  
  displayInfo("iniciando", "Conectando Wifi");
  int count =0 ;
  WiFi.mode(WIFI_STA);
  WiFi.begin(MyConfig.ssid, MyConfig.password);

  Serial.print("Conectando no Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    if (count++ >= 10) break;    
  }
  
  if (WiFi.status() == WL_CONNECTED)
  {
    displayInfo("iniciando", "Sincronizando horario");

    //Configurando relogio
    Serial.println("Carregando data e hora.");
    configTime(TZ, "pool.ntp.org");

    delay(500);
 
    
    Serial.println("ok");
    digitalWrite(LED_BUILTIN, LOW);

 
  
    Serial.println("Iniciando servidor web.");
    //Configurando endpints
    server.on("/", HTTP_GET, handleStatus);
    server.on("/alimentar", HTTP_GET, handleAlimentar);
    server.on("/config", HTTP_GET, handleConfig);
    server.on("/save", HTTP_POST, handleConfigSave);
    server.onNotFound(handleNotFound);
    server.begin();

    time_t now = time(NULL);
    tm_struct = localtime(&now);


  
    Serial.print("Hora:");
    Serial.print(tm_struct->tm_hour);
    Serial.print(" Minuto:");
    Serial.print(tm_struct->tm_min);
    Serial.print(" Segundo:");
    Serial.println(tm_struct->tm_sec);
    Serial.print("Conectado em: ");
    Serial.println(MyConfig.ssid);
    Serial.print("IP: ");
    Serial.println(IP_local = WiFi.localIP().toString());
  }else{
    displayInfo("noticia", "Wifi Falhou");
    delay(1000);
    Serial.println("Falhou.");
    WiFi.disconnect();
    displayInfo("modo config", "Iniciando AP Wifi");
    delay(1000);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(YOUR_AP_WIFI_SSID);

    server.on("/", HTTP_GET, handleConfig);
    server.on("/save", HTTP_POST, handleConfigSave);
    server.onNotFound(handleNotFound);
    server.begin();

    Serial.print("SSID: ");
    Serial.println(YOUR_AP_WIFI_SSID);

    Serial.print("AP IP: ");
    Serial.println(IP_local = WiFi.softAPIP().toString());
    displayInfo("modo config", IP_local);

  }
}
void(* resetFunc) (void) = 0;


void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (WiFi.status() == WL_CONNECTED){
      showDisplay();
      if (tm_struct->tm_hour == MyConfig.h1h && tm_struct->tm_min == MyConfig.h1m && tm_struct->tm_sec == 0) functionPutFood();
      if (tm_struct->tm_hour == MyConfig.h2h && tm_struct->tm_min == MyConfig.h2m && tm_struct->tm_sec == 0) functionPutFood();
      if (tm_struct->tm_hour == MyConfig.h3h && tm_struct->tm_min == MyConfig.h3m && tm_struct->tm_sec == 0) functionPutFood();
      if (tm_struct->tm_hour == MyConfig.h4h && tm_struct->tm_min == MyConfig.h4m && tm_struct->tm_sec == 0) functionPutFood();
      
    }
  }
}

void showDisplay(){

  String hour;
  String mim;
  String sec;

  time_t now = time(NULL);
  tm_struct = localtime(&now);


  
  hour = String(tm_struct->tm_hour);
  if (tm_struct->tm_hour <= 9)
    hour = "0"+hour;

  mim = String(tm_struct->tm_min);
  if (tm_struct->tm_min <= 9)
    mim = "0"+mim;

  sec = String(tm_struct->tm_sec);
  if (tm_struct->tm_sec <= 9)
    sec = "0"+sec;

  
  display.clearDisplay();
  display.drawBitmap(0, 0, dog_bitmap, 31, 31, WHITE);
 
  display.setTextColor(SSD1306_WHITE);      
  
  display.setTextSize(2);           
  
  display.setCursor(40,5); 
  display.println("GlubbY");
  
  display.setTextSize(1);           

  display.setTextColor(BLACK, WHITE);  
  display.setCursor(90,20); 
  display.println("V1.0");
 
  display.setTextColor(SSD1306_WHITE);      
  display.setTextSize(2);           
  
  display.setCursor(10,33); 
  display.println(hour);

  display.setCursor(35,33); 
  display.println(":");

  display.setCursor(50,33); 
  display.println(mim);

  display.setCursor(75,33); 
  display.println(":");

  display.setCursor(90,33); 
  display.println(sec);


  display.setTextSize(1);           

  display.setCursor(18,56); 
  display.println(IP_local);

  display.setCursor(103,56); 
  display.println("T:");

  display.setCursor(115,56); 
  display.println(MyConfig.timer);


  display.drawLine(0,52,130,52,WHITE);

  display.display();

/*
  Serial.print("Hora:");
  Serial.print(hour);
  Serial.print(" Minuto:");
  Serial.print(mim);
  Serial.print(" Segundo:");
  Serial.println(sec);
*/
}

void displayInfo(String title, String msg){



  display.clearDisplay();
  display.drawBitmap(50, 15, dog_bitmap, 31, 31, WHITE); 
  
  display.setTextColor(SSD1306_WHITE); 
  display.setTextSize(1);           

  int pos = 128/2 - (msg.length() / 2 * 6) ;
  display.setCursor(pos,56); 
  display.print(msg);
 
  pos = 128/2 - (title.length() / 2 * 6) ;
  display.setTextColor(BLACK, WHITE);    
  display.setCursor(0,0);
  display.print("                     ");
  display.setCursor(pos,0);
  display.print(title);


  display.display();

}

// functions to be called when an alarm triggers:
void functionPutFood() {
  Serial.println("Alimentando Au Au...");
  tone(GP_ACTIVE_BUZZER, 660, 250); 
  delay(300);
  tone(GP_ACTIVE_BUZZER, 660, 250); 
  delay(300);
  tone(GP_ACTIVE_BUZZER, 660, 250); 
  delay(600);
  tone(GP_ACTIVE_BUZZER, 660, 600); 
  delay(700);

  displayInfo("INFORMACAO", "Alimentando gluby");
  digitalWrite(GP_RELE_FOOD, HIGH);
  digitalWrite(GP_RELE_LUZ, HIGH);

  if (MyConfig.timer > 0)
  {
    for (int x = 1 ; x <= MyConfig.timer; x++){
      delay(1000);
      Serial.println(MyConfig.timer - x);
      
    }
  }

  digitalWrite(GP_RELE_FOOD, LOW);
  digitalWrite(GP_RELE_LUZ, LOW);
  

  tone(GP_ACTIVE_BUZZER, 660, 600); 
  delay(700);
  tone(GP_ACTIVE_BUZZER, 660, 250); 
  delay(600);
  tone(GP_ACTIVE_BUZZER, 660, 250); 
  delay(600);
  tone(GP_ACTIVE_BUZZER, 660, 250); 
  delay(600);
  Serial.println("Terminou...");
}


void handleAlimentar() {
  msg = "Alimentando seu animalzinho.";
  handleStatus();
  functionPutFood();
}
  

void handleConfig() {


  
  String statusConfig = headPage + "\
    <form method=\"post\" action=\"/save\">\
      <h1>Dados do Wifi:</h1>\
      <div class=\"form-group\">\
        <label>SSID</label>\
        <input type=\"text\"  class=\"form-control\" placeholder=\"Digite o SSID\" name=\"ssid\" value=\"" + MyConfig.ssid + "\">\
      </div>\
      <div class=\"form-group\">\
        <label>SSID</label>\
        <input type=\"password\"  class=\"form-control\" placeholder=\"Digite a senha\" name=\"password\" value=\"" + MyConfig.password + "\">\
      </div>\
      <br><h1>Horarios:</h1>";

      statusConfig += "\
      <div class=\"form-group\">\
        <label>Horario 1</label>\
        <table width=\"100%\"><tr><td>";
        statusConfig += "<select name=\"h1h\"  class=\"form-control\" >";
        statusConfig += "<option value=\"\">Hora</option>";
        for(int h = 1; h <= 23; h++)statusConfig += "<option value=\"" + String(h) + "\" "+(h==MyConfig.h1h?"selected":"")+">" + String(h) + "</option>";
        statusConfig += "</select></td><td>";
        statusConfig += "<select name=\"h1m\"  class=\"form-control\" >";
        statusConfig += "<option value=\"\">Minutos</option>";
        for(int m = 0; m <= 55; m+=5)statusConfig += "<option value=\"" + String(m) + "\" "+(m==MyConfig.h1m?"selected":"")+">" + String(m) + "</option>";
        statusConfig += "</select></td></tr></table>\
      </div>";

      statusConfig += "\
      <div class=\"form-group\">\
        <label>Horario 2</label>\
        <table width=\"100%\"><tr><td>";
        statusConfig += "<select name=\"h2h\"  class=\"form-control\" >";
        statusConfig += "<option value=\"\">Hora</option>";
        for(int h = 1; h <= 23; h++)statusConfig += "<option value=\"" + String(h) + "\" "+(h==MyConfig.h2h?"selected":"")+">" + String(h) + "</option>";
        statusConfig += "</select></td><td>";
        statusConfig += "<select name=\"h2m\"  class=\"form-control\" >";
        statusConfig += "<option value=\"\">Munutos</option>";
        for(int m = 0; m <= 55; m+=5)statusConfig += "<option value=\"" + String(m) + "\" "+(m==MyConfig.h2m?"selected":"")+">" + String(m) + "</option>";
        statusConfig += "</select></td></tr></table>\
      </div>";
      
      statusConfig += "\
      <div class=\"form-group\">\
        <label>Horario 3</label>\
        <table width=\"100%\"><tr><td>";
        statusConfig += "<select name=\"h3h\"  class=\"form-control\" >";
        statusConfig += "<option value=\"\">Hora</option>";
        for(int h = 1; h <= 23; h++)statusConfig += "<option value=\"" + String(h) + "\" "+(h==MyConfig.h3h?"selected":"")+">" + String(h) + "</option>";
        statusConfig += "</select></td><td>";
        statusConfig += "<select name=\"h3m\"  class=\"form-control\" >";
        statusConfig += "<option value=\"\">Munutos</option>";
        for(int m = 0; m <= 55; m+=5)statusConfig += "<option value=\"" + String(m) + "\" "+(m==MyConfig.h3m?"selected":"")+">" + String(m) + "</option>";
        statusConfig += "</select></td></tr></table>\
      </div>";

      statusConfig += "\
      <div class=\"form-group\">\
        <label>Horario 4</label>\
        <table width=\"100%\"><tr><td>";
        statusConfig += "<select name=\"h4h\"  class=\"form-control\" >";
        statusConfig += "<option value=\"\">Hora</option>";
        for(int h = 1; h <= 23; h++)statusConfig += "<option value=\"" + String(h) + "\" "+(h==MyConfig.h4h?"selected":"")+">" + String(h) + "</option>";
        statusConfig += "</select></td><td>";
        statusConfig += "<select name=\"h4m\"  class=\"form-control\" >";
        statusConfig += "<option value=\"\">Munutos</option>";
        for(int m = 0; m <= 55; m+=5)statusConfig += "<option value=\"" + String(m) + "\" "+(m==MyConfig.h4m?"selected":"")+">" + String(m) + "</option>";
        statusConfig += "</select></td></tr></table>\
      </div>";

      
      statusConfig += "Tempo Alimentador:<br><input type=\"text\" class=\"form-control\"  name=\"timer\" value=\"" + String(MyConfig.timer) + "\"><br>\
      <input type=\"checkbox\" name=\"sluz\" "+(MyConfig.sluz==1?"checked":"")+" />Sinal luminoso<br>\
      <input type=\"submit\" style=\"width:100%\" class=\"btn btn-primary\" value=\"Salvar\">\
    </form>\
  <script>\
    document.getElementById(\"div_config\").classList.add(\"active\");\
  </script>\
  "+footPage;
  server.send(200, "text/html", statusConfig);
}

void handleConfigSave() {

  String ssid = server.arg("ssid");
  String password = server.arg("password");

  ssid.toCharArray(MyConfig.ssid, ssid.length() +1);
  password.toCharArray(MyConfig.password, password.length() +1);
  MyConfig.timer =  server.arg("timer").toInt();

  MyConfig.h1h =  server.arg("h1h").toInt();
  MyConfig.h1m =  server.arg("h1m").toInt();

  MyConfig.h2h =  server.arg("h2h").toInt();
  MyConfig.h2m =  server.arg("h2m").toInt();

  MyConfig.h3h =  server.arg("h3h").toInt();
  MyConfig.h3m =  server.arg("h3m").toInt();

  MyConfig.h4h =  server.arg("h4h").toInt();
  MyConfig.h4m =  server.arg("h4m").toInt();

  if (server.hasArg("sluz"))
    MyConfig.sluz =  1;
  else
    MyConfig.sluz =  0;
  
  EEPROM.put(0, MyConfig);
  EEPROM.commit();
  Serial.println("Dados de login salvo com sucesso.");
  Serial.print("SSID:");
  Serial.println(MyConfig.ssid);
  Serial.print("Password:");
  Serial.println(MyConfig.password);
 
  Serial.println( "Alimentacao:");
  Serial.print( "Horario 1:");
  Serial.print( MyConfig.h1h);
  Serial.print( ":");
  Serial.println( MyConfig.h1m);
  
  Serial.print( "Horario 2:");
  Serial.print( MyConfig.h2h);
  Serial.print( ":");
  Serial.println( MyConfig.h2m);
    
  Serial.print( "Horario 3:");
  Serial.print( MyConfig.h3h);
  Serial.print( ":");
  Serial.println( MyConfig.h3m);
  
  Serial.print( "Horario 4:");
  Serial.print( MyConfig.h4h);
  Serial.print( ":");
  Serial.println( MyConfig.h4m);

  Serial.print( "Tempo de alimentacao:");
  Serial.println( MyConfig.timer);
  
  msg = "Configuracao salva com sucesso.";
  handleStatus(); 
  
  delay(1000);

  resetFunc(); 
}

void handleStatus() {

  String statusPage = headPage;
  if (msg!=""){
    statusPage += "<div class=\"alert alert-success\" role=\"alert\">"+msg+"</div>";
    msg="";
  }
  
  
  statusPage +=  "\
  <h1 class=\"cover-heading\">Alimentador Automatico.</h1>\
  <p class=\"lead\">Com esse dispositivo voce pode configurar os horarios que deseja alimentar o seu pet de forma automatica.</p>\
  <p class=\"lead\">\
    <a href=\"/alimentar\" class=\"btn btn-lg btn-secondary\">Alimentar</a>\
  </p>\
  <script>\
    document.getElementById(\"div_home\").classList.add(\"active\");\
  </script>\
  "+footPage;
  server.send(200, "text/html", statusPage);
}

void handleNotFound() {
 Serial.print("Webserver - 404 - Pagina nao encontrada. URI:");
 Serial.println(server.uri());
 server.send(404, "text/plain", "Pagina nao encontrada.");
}
