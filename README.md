# Estacionamento inteligente com validação de vaga PCD via sensor RFID

<img width="1323" height="627" alt="image" src="https://github.com/user-attachments/assets/f2317f9c-1b1d-447b-87bb-2fb72536703d" />

## 🛠️​**Ferramentas utilizadas**
* 1x Esp32
* 2x Sensor de proximidade infravermelho
* 1x Servomotor
* 2x Leds Verdes
* 2x Leds Vermelhos
* 1x Buzzer
* 1x Módulo RFID
* 5x Resistores 220Ω

## ⚡​**Esquema elétrico**
<img width="2779" height="1990" alt="ProjetoTT" src="https://github.com/user-attachments/assets/57407f1d-0795-45e1-a3cc-5029eca52fcb" />

**Módulo RFID:**
* Vcc <-> 3V3 
* RST (Reset) <-> D21
* GND <-> GND
* MISO (Master Input Slave Output) <-> 19
* MOSI (Master Output Slave Input) <-> 23
* SCK (Serial Clock) <-> 18
* SS/SDA (Slave select) <-> 5

**Sensores Infravermelhos:**
* Vcc <-> 3V3 
* GND <-> GND
* D26 (Entrada) D15 (Vaga)

**Servo motor:**
* Vcc <-> 3V3 
* GND <-> GND
* D32

**Leds:**
* Vcc <-> 3V3 
* GND <-> GND
* D4 (Alarme) D2 (Verificação feita)
* D13 (Vaga Ocupada) D2 (Vaga Livre)

**Buzzer:**
* Vcc <-> 3V3 
* GND <-> GND
* D22

## ⚙️**Funcionamento**
O sistema conta com um sensor de proximidade para liberação da catraca de entrada (servo motor). Ao encontrar uma vaga livre (led verde), para ocupá-la o usuário deve validar sua TAG PCD no sensor RFID, caso contrário um alarme soará (led vermelho), enquanto a validação não for feita. Caso a TAG incorreta seja mostrada, o mesmo alarme soará até que uma TAG correta seja apresentada. Quando uma TAG correta é mostrada, o usuário ocupa a vaga (led vermelho) e o alarme ficará desativado (led verde).
A aplicação também explora os conceitos de IoT, enviado os dados de status da vaga (livre/ocupada) e do alarme (ativado/desativado) para a nuvem, para um possível processamento e uso remoto dessa informação
