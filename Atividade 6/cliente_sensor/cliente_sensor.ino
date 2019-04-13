#include <Ethernet.h>
#include <SPI.h>

#define sensorPIn 0
int led = 8;
float Celsius,Kelvin;
int sensorValue;


byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x29, 0x02 };    //mac do shield
byte server[] = { 192, 168, 9, 30 }; // IP do servidor
int tcp_port = 5000;  //porta do servidor

EthernetClient client;

void setup()
{
  Ethernet.begin(mac);
  Serial.begin(9600);
  pinMode(led,OUTPUT);
  
  delay(1000);

  Serial.println("Connecting...");

  if (client.connect(server, tcp_port)) { // Connection to server.js
    Serial.println("Connected to server");
  } else {
    Serial.println("connection failed");
  }
}

void loop()
{
 GetTemp();
 
  delay(5000);         //5segundos
  if (client.connected()) {
      
      char send_buff[10];
      itoa(Celsius, send_buff, 10);
      client.write(send_buff); // Send what is read on serial monitor | tem que ser em string
      Serial.print("Enviado: ");
      Serial.println(send_buff);
      String rec_buff = String(100);
      while(client.available()){
        //ele recebe o caractere em int da tabela ascii
        //converter para char e colocar TUDO em rec_buff
        //codigo abaixo esta errado, rs
        int a=client.read();
        char b[3];
        String str;
        str=String(a);

        str.toCharArray(b,3); 
        
        
        rec_buff += str;
      }
      
        Serial.print("Recebido: ");
        Serial.println(rec_buff);

  if(rec_buff.toInt() == 1)
  {
    digitalWrite(led,HIGH);
  }
  
  else
  {
    digitalWrite(led,LOW);
  }


      
  }

  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    for(;;)
      ;
  }
}


void GetTemp(){
sensorValue  = analogRead(sensorValue);
Kelvin = (((float(sensorValue) / 1023) * 5) * 100);
Celsius = Kelvin - 273.15;
  
}
