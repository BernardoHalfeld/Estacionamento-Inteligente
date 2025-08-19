#include <ESP32Servo.h> //import da bilbioteca para utilização do servo motor
#include <WiFi.h> //import da bilbioteca para utilização do módulo wifi do esp
#include <ThingSpeak.h> //import da bilbioteca para utilização da plataforma em nuvem "ThingSpeak"
#include <SPI.h> //import da bilbioteca para utilização do módulo RFID RC522
#include <MFRC522.h> //import da bilbioteca para configuração e utilização da comunicação SPI

Servo servoMotor; //criação de um "objeto" servo para referências futuras

const int pinoSensorObs1 = 26; //Declaração do Pino utilizado pelo sensor de obstáculos da entrada
const int pinoSensorObs2 = 15; //Declaração do Pino utilizado pelo sensor de obstáculos da vaga

const int LED_OCUPADO = 13;//Declaração do Pino utilizado pelo Led vermelho (ocupado)
const int LED_LIVRE = 12;//Declaração do Pino utilizado pelo Led Verde (livre)

#define RST_PIN   21    // Pino de reset do RFID
#define SS_PIN    5    // Pino de seleção SPI

//Declarações utilizadas no alarme
#define BUZZER_PIN 22    // Pino do buzzer
#define LED_PIN   4     // Pino do LED(alarme)
#define LEDV_PIN   2    // Pino do LED(verificação feita com sucesso)

int status;//variavel para indicar o status da vaga (0-ocupado 1-livre)
int statusCatraca;//variavel para indicar o status da catraca (0-fechado 1-aberto)

// Configuração da rede WiFi com as credenciais corretas:
const char* WIFI_NAME = "Sua rede";
const char* WIFI_PASSWORD = "Sua senha";

// Parâmetros da nuvem ThingSpeak
const int myChannelNumber = //Numero do canal;
const char* myApiKey = "Sua chave";
const char* server = "api.thingspeak.com";

WiFiClient client;//criação de um "objeto" client para referências futuras

MFRC522 mfrc522(SS_PIN, RST_PIN); // Inicializa o leitor RFID utilizando os pinos SPI

//Criação de objetos que serão utilizados utilizadas no alarme
const int buzzer = BUZZER_PIN;
const int led = LED_PIN;
const int ledV = LEDV_PIN;

bool tagLida = false; //varivael para verificação de leitura de tag
int parametroTempo = 0; //varivael para medir o intervalo de tempo sem tags apresentadas

// Define os valores da tag correta
byte expectedUID[] = {0xEB, 0xBC, 0x93, 0x1C};

// Rotina de setup do ESP
void setup() {
  Serial.begin(115200);//Define o Baud Rate da transmissão serial
  
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD, 6);// Inicia o processo de conexão Wi-Fi com as credenciais fornecidas
  while (WiFi.status() != WL_CONNECTED) {// Aguarda até que a conexão Wi-Fi seja estabelecida
    delay(100);
    Serial.print(".");
  }
  // Quando a conexão Wi-Fi é estabelecida com sucesso, exibe mensagens informativas
  Serial.println("Wifi conectado!");
  Serial.println("Local IP: " + String(WiFi.localIP()));
  WiFi.mode(WIFI_STA);// Configura o modo do ESP32 para operar como estação (client) na rede Wi-Fi
  ThingSpeak.begin(client);// Inicia a comunicação com a plataforma ThingSpeak usando o cliente WiFi fornecido


  pinMode(LED_OCUPADO, OUTPUT);//Define o led vermelho como saída (emissão)
  pinMode(LED_LIVRE, OUTPUT);//Define o led verded como saída (emissão)

  pinMode(pinoSensorObs1, INPUT); //Define o pino do sensor de obstáculos como entrada (recepção)
  pinMode(pinoSensorObs2, INPUT); //Define o pino do sensor de obstáculos como entrada (recepção)

  servoMotor.attach(32);//Declaração do Pino utilizado pelo servo

  SPI.begin();// Inicializa o barramento SPI
  mfrc522.PCD_Init();// Inicializa o leitor RFID MFRC522

  //Define os objetos utilizados para alarme (buzzer e leds) como saída (emissão)
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(ledV, OUTPUT);
}

// Rotina de execução contínua
void loop() {

  if(digitalRead(pinoSensorObs1) == LOW){//Se o sensor de obstáculos registrar presença (low)
      statusCatraca = 1;
      servoMotor.write(25); //aberto (servo 90°)
  }else{ //SENÃO, FAZ
      statusCatraca = 0;
      servoMotor.write(115); //fechado  (servo 0°)
  }
  
  //Vaga ocupada
  if(digitalRead(pinoSensorObs2) == LOW){
    digitalWrite(LED_OCUPADO, HIGH);//ativa o led vermelho
    digitalWrite(LED_LIVRE, LOW);//apaga o led verde

    Serial.println(parametroTempo);//mostra o parâmetro de tempo atual

    //Ativa o alarme se tags não foram detectadas e o tempo foi excedido
    if((parametroTempo > 2) && (!tagLida)){
      Serial.println("tempo excedido");
      soundAndBlink();
    }
    else{//se não, incrementa o parâmetro de tempo
      parametroTempo += 1;
    }

    Serial.println(tagLida);//mostra o parâmetro tag lida

    if(!tagLida){//Caso nenhuma tag tenha sido lida, procura por tags
      // Procura por cartões
      while (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        Serial.println("Cartão detectado!");

        // Mostra a UID do cartão
        Serial.print("UID da Tag:");
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
        }
        Serial.println();
      
        // Verifica se a UID do cartão coincide com a UID esperada
        //Se sim, define tag lida para 1, ativa a led de verificação e ocupa a vaga
        //Se não, ativa o alarme
        if (compareUID(mfrc522.uid.uidByte, expectedUID, mfrc522.uid.size)) {
          Serial.println("Tag RFID válida!");
          tagLida = true;
          digitalWrite(ledV, HIGH);
          status = 0;//define status ocupado
          break;
        } else if(!(compareUID(mfrc522.uid.uidByte, expectedUID, mfrc522.uid.size))) {
          Serial.println("Tag RFID inválida!");
          soundAndBlink();//ativa a função de alarme
        }
        mfrc522.PICC_HaltA();// Desativa a comunicação com o cartão RFID atualmente detectado
      }
    }

  }
  else{ //Livre
    digitalWrite(LED_OCUPADO, LOW);//apaga o led vermelho
    digitalWrite(LED_LIVRE, HIGH);//ativa o led verde
    status = 1;//define status livre
    tagLida = false;//libera a variavel tag lida para proximas verificações
    parametroTempo = 0;//zera o parametro de tempo
    digitalWrite(ledV, LOW);//desliga a led de verificação caso esteja ligada
  }
  
  // Agora preparamos o envio dos dados para nuvem ThingSpeak
  //Defimos o campo determinado e os valores a serem enviados
  ThingSpeak.setField(2, statusCatraca);
  ThingSpeak.setField(3, status);
  ThingSpeak.setField(4, status);

  // Envia os dados para a plataforma ThingSpeak usando o canal e a chave API fornecidos
  int x = ThingSpeak.writeFields(myChannelNumber, myApiKey);
  // Verifica se o envio foi bem-sucedido com base no código de resposta HTTP
  if(x == 200) {
    Serial.println("Dados enviados com sucesso!");
  }else{
    Serial.println("Erro ao comunicar com servidor..." + String(x));
  }
  Serial.println("---");

  delay(5000);//delay de 1 segundo para voltar a função de loop
  
}

// Função para comparar UIDs
bool compareUID(byte* uid1, byte* uid2, byte size) {
  for (byte i = 0; i < size; i++) {
    if (uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}

// Função para soar o alarme no buzzer e piscar o LED
void soundAndBlink() {
  servoMotor.detach();
  while (true) {
    tone(buzzer, 1000);  // Frequência do som do alarme
    digitalWrite(led, HIGH);  // Acende o LED
    delay(500);          // Duração do alarme e do LED aceso
    noTone(buzzer);      // Desliga o buzzer
    digitalWrite(led, LOW);   // Apaga o LED
    delay(500);          // Pausa entre ciclos
    
    // Verifica se uma tag correta foi detectada enquanto o alarme estava soando e vaga ainda está ocupada
    if ((mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) || !(digitalRead(pinoSensorObs2) == LOW)) {
      //Se sim, ativa a tag lida, zera o parametro de tempo e ativa a led de verificação, se não mantem a condição de alarme até desocupar a vaga ou detectar uma tag correta
      if ((compareUID(mfrc522.uid.uidByte, expectedUID, mfrc522.uid.size)) || !(digitalRead(pinoSensorObs2) == LOW)){
        if(compareUID(mfrc522.uid.uidByte, expectedUID, mfrc522.uid.size)){
          status = 0;//define status ocupado
        }
        //Serial.println("Tag RFID válida!");
        tagLida = true;
        parametroTempo = 0;
        digitalWrite(ledV, HIGH);
        servoMotor.attach(32);
        break;  // Sai do loop se uma tag correta for detectada
      } else{
        Serial.println("Tag RFID inválida!");
      }
    }
  }
}
