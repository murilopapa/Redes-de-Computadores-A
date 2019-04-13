#define sensorPIn 0
int led = 8;


float Celsius,Fahrenheit,Kelvin;
int sensorValue,resposta = 1;

void setup() {
  Serial.begin(9600);
  pinMode(led,OUTPUT);
  Serial.println("Iniciando...");
  

}

void loop() {
 GetTemp();

 //valor da temperatua esta em Celsius. Enviar esse valor para o servidor
 
 
 //Receber 0 ou 1 do servidor. Se , ligar led, se 0, desligar led

  if(resposta == 1)
  {
    digitalWrite(led,HIGH);
  }
  
  else
  {
    digitalWrite(led,LOW);
  }

 delay(2000);//mudar pra 10s
 
}

void GetTemp(){
sensorValue  = analogRead(sensorValue);
Kelvin = (((float(sensorValue) / 1023) * 5) * 100);
Celsius = Kelvin - 273.15;
  
}
