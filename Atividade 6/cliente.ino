// Ettore Biazon Baccan         16000465
// Mateus Henrique Zorzi        16100661
// Matheus Martins Pupo         16145559
// Murilo Martos Mendonçca      16063497
// Victor Hugo do Nascimento    16100588

#include <Ethernet.h>
#include <SPI.h>

#define sensorPin 2
#define ledPin 8
float celsius, kelvin;
int sensorValue, led_on_off;

byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x29, 0x02}; //mac do shield
byte server[] = {192, 168, 9, 39};                 // IP do servidor
int tcp_port = 5000;                               //porta do servidor

EthernetClient client;

void setup()
{
  Ethernet.begin(mac);
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  delay(1000);

  Serial.println("Connecting...");

  if (client.connect(server, tcp_port))
  { // Connection to server.js
    Serial.println("Connected to server");
  }
  else
  {
    Serial.println("connection failed");
  }
}

void loop()
{
  //GetTemp();

  delay(10000); //10segundos
  if (client.connected())
  {

    char send_buff[10];
    int analog_read = analogRead(sensorPin);
    Serial.print("AnalogRead: ");
    Serial.println(analog_read);
    celsius = (5.0 * analog_read * 100.0) / 1024.0;
    dtostrf(celsius,4,2,send_buff);
    //atof(celsius, send_buff, 10);
    client.write(send_buff);   // manda a msg pro servidor, tem que ser vetor de char
    Serial.print("Enviado: "); //confirma o que foi enviado
    Serial.println(send_buff); //confirma o que foi enviado

    while (!client.available()) //loop de espera ocupada para esperar o servidor enviar algo
    {
    }
    
      led_on_off = client.read();   //vamos receber apenas 1 char do servidor falando se o led tem que ser acesso (1) ou apagado (0)
      client.read();                //recebemos 2 chars, o primeiro com a info, e o segundo com o /0, por isso o descartamos
      led_on_off = led_on_off - 48; //faz a conta da tabela ascii, para converter para o numero que queremos
      Serial.print("Recebido: ");   //printa o que recebeu, para confirmação
      Serial.println(led_on_off);
      
    if (led_on_off) //se for pra acender a led
    {
      digitalWrite(ledPin, HIGH);   //manda um sinal de HIGH na porta da led
    }
    else    //se nao, apaga ela, caso esteja acessa, apaga, caso esteja apagada, continua
    {
      digitalWrite(ledPin, LOW);    //manda um sinal de LOW na porta da led
    }
  }

  if (!client.connected())    //se nao estiver mais conectado, encerra o programa, ficando em um loop infinito, uma vez que nao é possivel desligar o arduino por codigo (tipo um exit(0))
  {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    for (;;)
      ;
  }
}

