//-----------------------LIBRERIAS--------------------------
#include <ESP8266WiFi.h> //librerias para la conexion WIFI
#include <PubSubClient.h> //libreria conexion mqtt
#include <DHT.h>//libreria sensor temperatura humedad

//-------------------VARIABLES GLOBALES-------------------------- 

//variables conexion y WIFI
WiFiClient esp8266Client;
PubSubClient client(esp8266Client);
const char *ssid = "FAMILIA"; //nombre de usuario WIFI
const char *password = "obandogil1234"; //y su contraseña.
int contconexion = 0; //cuenta el numero de conexiones cada que reinicia
unsigned long previousMillis = 0; //segundos de espera para actualizar y enviar informacion a la pagina web.

//variables MQTT 
char   SERVER[30]   = "m15.cloudmqtt.com";//"34.238.150.50"; //"m15.cloudmqtt.com" //servidor broker, direccion url o Ip. En la pc ir a buscar/cmd/ping m15.cloudmqtt.com
int    SERVERPORT   = 19262; //El numero del puerto del servidor lo asigna CloudMQTT.
String USERNAME = "projectXesp8266";   //Nombre de usuario registrado en cloudMQTT
char   PASSWORD[20] = "19951998";//y su contraseña.

//variables pin topic
char username[50];//username de ClodMQTT convertido de string a cadena
char valueStrHum[15];
char valueStrTemp[15];
char valueStrHumAmb[15];
String StrHumedad = "";
String StrTemperatura  = "";;
String StrHumedadAmb = "";
char HUMEDAD[50];
//char PULSADOR[50];
char REGAR[50];
char SALIDAANALOGICA[50];
char TEMPERATURA[50];
char HUMEDADAMBIENTE[50];

//variables componentes
double h ;// humedad ambiente Dht11
double t;//temperatura ambiente 
double porcentajeHumedad; 
double rangoHumedad;
int humedadMax = 75;
int humedadMin = 25;
int ledRojo = 12;
int ledVerde = 13;
int ledAzul = 14;
int ventilador = 5;
int regar = 15;//rele//bomba de agua
//int tiempoDeRiego = 20000; 
//int tiempoDeEspera = 100000;
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
#define DHTTYPE DHT11// Dependiendo del tipo de sensor
DHT dht(DHTPIN, DHTTYPE);// Inicializamos el sensor DHT11

//------------------------SETUP-----------------------------
void setup(){
    //prepara GPI13 y 12 como salidas 
    pinMode(16, OUTPUT); // D0 salida analógica
    pinMode(ledRojo,OUTPUT); //se declara como salida los leds: rojo  
    pinMode(ledVerde,OUTPUT); //verde
    pinMode(ledAzul,OUTPUT); //azul  
    pinMode(regar,OUTPUT);//bomba de agua
    digitalWrite(regar,LOW);    
    digitalWrite(ventilador,LOW);    
    
    // Entradas

    // Inicia conexion monitor Serial
    Serial.begin(115200);
    Serial.println("");
    // Comenzamos el sensor DHT
    dht.begin();
    // Conexión WIFI
    WiFi.begin(ssid, password);
    //para  usar ip dinamica       
    while(WiFi.status() != WL_CONNECTED) {//estado de conexion de nuestro modulo a wifi// WL IDLE STATUS // CAMBIA DE UN ETSADO A OTRO
        delay(500);                        //retorna varios codigos                   //WL NO SSID AVAIL // nOMBRE iNCORRECTO
        Serial.print(".");                                                            //WL CONNECT FAILED//cONTRASEÑA iNCORRECTA
                                                                                      //disconneted no configurado modo estacion
    }
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println(WiFi.localIP());
    Serial.print(" = Direccion ip");

    client.setServer(SERVER, SERVERPORT);//inicia server
    client.setCallback(callback); //callback recibe los mensajes
    
    // se etiquetan y declaran los topics que se ejecutaran en el websocket ui
    String humedadTopic = "/" + USERNAME + "/" + "humedadDelSuelo"; //etiqueta para Rules en cloud MQTT
    humedadTopic.toCharArray(HUMEDAD, 50); //convertido a un array de 50
    String salidaDigitalTopic = "/" + USERNAME + "/" + "riegoManual"; 
    salidaDigitalTopic.toCharArray(REGAR, 50);
    String temperaturaTopic = "/" + USERNAME + "/" + "temperaturaAmbiente"; 
    temperaturaTopic.toCharArray(TEMPERATURA, 50); 
    String humedadAmbTopic = "/" + USERNAME + "/" + "humedadAmbiente"; 
    humedadAmbTopic.toCharArray(HUMEDADAMBIENTE, 50); 
    String salidaAnalogicaTopic = "/" + USERNAME + "/" + "salidaAnalogica"; 
    salidaAnalogicaTopic.toCharArray(SALIDAANALOGICA, 50);  
}
//--------------------------LOOP--------------------------------
void loop()
{
    if(!client.connected()) { //si el cliente no conecta llamamos la funcion reconectar
        reconnect();
    } 
    client.loop();
    bucle();
    delay(500);
}
//------------------------FUNCIONES CONEXION WIFI Y MQTT-----------------------------

//CICLO Funciones conexion wifi de sensores
void bucle(){
    unsigned long currentMillis = millis(); //"millis" cuenta los miliseg desde que arranca el microprocesador   
    if(currentMillis - previousMillis >= 3000) { //envia la temperatura cada 5 segundos
        previousMillis = currentMillis;  //entre tiempo actual y el tiempo previo
        int lectura = analogRead(A0);//lectura analogRead del sensor de humedad            
    porcentajeHumedad = map(lectura, 1024, 0, 0, 230);//conversion de analogRead a porcentaje de humedad
    //Serial.print("La lectura es: ");                //3.3 v = 668 x el 100% de humedad
    //Serial.println(lectura);
    h = dht.readHumidity();// Leemos la humedad relativa
    h = h - 10.0;
    t = dht.readTemperature();    // Leemos la temperatura en grados centígrados (por defecto)
    lecTempHumAmbiente(h,t); 
    LecturaHumedad(porcentajeHumedad);//se hace lectura de humedad del suelo para saber su estado
    humedadMin = rangoHumedad;
    if(porcentajeHumedad < humedadMin && porcentajeHumedad > 0){
      if(h < 80 && t > 25){
        Riego(porcentajeHumedad); //riega
      }
      else{
        Serial.println("No es necesario regar"); 
      }
    }
          while(t > 25){
          digitalWrite(ventilador,HIGH);
          Serial.print("La tempeatura del ambiente esta por encima de los 30°C");
      }
      digitalWrite(ventilador,LOW);
        //int analog = analogRead(17); //gpio entrada analogica lectura para un sensor ml35
        //float temp = lectura*0.322265625;  // 3.3v / 1024 (conversor analogico 2^10) * 100 mV = 0,322. el sensor manda 10mV por cada grado centigrado
        StrHumedad = String(porcentajeHumedad, 1); //1 decimal
        StrHumedad.toCharArray(valueStrHum, 15);
        StrTemperatura = String(t, 1); //1 decimal
        StrTemperatura.toCharArray(valueStrTemp, 15);
        StrHumedadAmb = String(h, 1); //1 decimal
        StrHumedadAmb.toCharArray(valueStrHumAmb, 15);               
        Serial.println("Enviando: [" +  String(HUMEDAD) + "] " + StrHumedad);
        client.publish(HUMEDAD, valueStrHum); //publica el valor de la humedad del suelo
        Serial.println("Enviando: [" +  String(TEMPERATURA) + "] " + StrTemperatura);
        client.publish(TEMPERATURA, valueStrTemp); //publica el valor de la temperatura        
        Serial.println("Enviando: [" +  String(HUMEDADAMBIENTE) + "] " + StrHumedadAmb);
        client.publish(HUMEDADAMBIENTE, valueStrHumAmb); //publica el valor de la temperatura
    }
}

//CALLBACK Funciones conexion wifi RiegoManual y analogica
void callback(char* topic, byte* payload, unsigned int length)//comunicacion por mensajes
{      //payload=dato o mensaje, topic=etiqueta de comando, tamaño 
    char PAYLOAD[5] = "    ";//vacio para salida analogica 1023
    Serial.print("Mensaje Recibido: [");//recibe mensaje, lee el topic que llega
    Serial.print(topic);
    Serial.print("] ");
    for(int i = 0; i < length; i++) { //convierte el mensaje que llega en bits a caracteres
        PAYLOAD[i] = (char)payload[i];
    }
    Serial.println(PAYLOAD);

    if(payload[0] != 'O'){
        long humMinConfig = 0;
        humMinConfig = (String(PAYLOAD).toInt());
        rangoHumedad = humMinConfig;
        //rangoHumedad = map(humMin, 1024, 0, 100, 0);              
        Serial.println(humMinConfig);
    }
    if(String(topic) ==  String(REGAR)) { //compara el topic que llega con el declarado en el codigo
        if(payload[1] == 'N') { //y solo tiene encuenta el segundo caracter
            digitalWrite(15, HIGH);
        }
        if(payload[1] == 'F') {
            digitalWrite(15, LOW);
        }
    }
    if(String(topic) ==  String(SALIDAANALOGICA)) {//ahora compara el topic de la salida analogica
        analogWrite(16, String(PAYLOAD).toInt()); //pin 13 payload convertido a string y luego a entero
    }
}

//RECONNECT
void reconnect()
{
    uint8_t retries = 3;  //numero de intentos de conexion
    // Loop hasta que estamos conectados
    while(!client.connected()) {
        Serial.print("Intentando conexion MQTT...");
        // Crea un ID de cliente aleatorio 
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX); 
        // Attempt to connect
        USERNAME.toCharArray(username, 50); //username lo convertimos a un string de 50
        if(client.connect("", username, PASSWORD)) {
            Serial.println("conectado");
            client.subscribe(REGAR); //subscribir los topcis para publicar algo sobre ellos
            client.subscribe(SALIDAANALOGICA);
        }
        else {
            Serial.print("fallo, rc=");
            Serial.print(client.state());
            Serial.println(" intenta nuevamente en 5 segundos");
            // espera 5 segundos antes de reintentar, lo hace 3 veces
            delay(5000);
        }
        retries--;
        if(retries == 0) {
            // esperar a que el WatchDogTimer lo reinicie
            while (1);
        }
    }
}
//--------------------FUNCIONES SENSORES, LEDS Y RELE-----------------------------
void Riego(int )
{
    LecturaHumedad (porcentajeHumedad);  // Realializo una lectura de confirmacion
    if (porcentajeHumedad < humedadMin) {
        digitalWrite (regar, HIGH); //prende bomba de agua
        Serial.println("Regando");
        delay (5000); //tiempo que va estar regando
        digitalWrite (regar, LOW); //apaga bomba de agua
    }
    else{
        Serial.println("No es necesario regar"); //no necesita regar
    }
}
void LecturaHumedad(int porcentajeHumedad)
{
    if(porcentajeHumedad == 0){
        Serial.print("La humedad es de: " );
        Serial.print(porcentajeHumedad);
        Serial.println("% ,El sensor esta desconectado");
        analogWrite(ledRojo, 0);
        analogWrite(ledVerde, 0);
        analogWrite(ledAzul, 0); 
    }
    else if(porcentajeHumedad <= humedadMin){
        Serial.print("La humedad es de: " );
        Serial.print(porcentajeHumedad);
        Serial.println("% ,El suelo esta seco");
        analogWrite(ledRojo, 255);
        analogWrite(ledVerde, 0);
        analogWrite(ledAzul, 0);
    }
    else if(porcentajeHumedad >= humedadMax){
        Serial.print("La humedad es de: " );
        Serial.print(porcentajeHumedad);
        Serial.println("% ,El suelo esta muy humedo");
        analogWrite(ledRojo, 0);
        analogWrite(ledVerde, 0);
        analogWrite(ledAzul, 255);
    }
    else if(porcentajeHumedad > humedadMin && porcentajeHumedad <humedadMax){
        Serial.print("La humedad es de: " );
        Serial.print(porcentajeHumedad);
        Serial.println("% ,El suelo esta humedo");
        analogWrite(ledRojo, 0);
        analogWrite(ledVerde, 255);
        analogWrite(ledAzul, 0);
    }
}

void lecTempHumAmbiente(float h,float t)
{
  // Leemos la temperatura en grados Fahreheit
  //float f = dht.readTemperature(true); 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
  // Calcular el índice de calor en Fahreheit
  //float hif = dht.computeHeatIndex(f, h);
  // Calcular el índice de calor en grados centígrados
  //float hic = dht.computeHeatIndex(t, h, false);
  Serial.print("Humedad ambiente: ");
  Serial.print(h);
  Serial.print("%");  
  Serial.print("\nTemperatura ambiente: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("\n");  
  //Serial.print(f);
  //Serial.print(" *F\t");
  //Serial.print("Índice de calor: ");
  //Serial.print(hic);
  //Serial.print(" *C ");
  //Serial.print(hif);
  //Serial.println(" *F");
}
