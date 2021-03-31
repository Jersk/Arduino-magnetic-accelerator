#include <Wire.h> //librerie display
#include <LiquidCrystal_I2C.h>

#define sensor1 A0  //pin analogici
#define sensor2 A1
#define sensor3 A2
#define sensor4 A3

#define mosfet1 3 //pin digitali
#define mosfet2 5
#define mosfet3 6
#define mosfet4 9

#define led1 10
#define led2 11
#define led3 12
#define led4 13

#define encoderPin1 2
#define encoderPin2 4

#define sogliaSensore 800
#define MaxAngle 13.0 //angolo massimo durata accensione bobina
#define MinAngle 1.0  //angolo minimo durata accensione bobina

float Angle = 4.0;  //angola fissato per la sola partenza
float velocity = 0;
int lastEncoded = 0;
int lastencoderValue = 0;
uint16_t count = 0;

const float diametro = 0.195; //diametro in metri
const float circonferenza = diametro * 3.1416;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  Serial.begin(115200); //configurazione display
  Wire.setClock(800000UL);
  lcd.begin(); // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Tilt to start");

  pinMode(sensor1, INPUT);  //configurazione pin d'ingresso e d'uscita
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
  pinMode(sensor4, INPUT);

  pinMode(mosfet1, OUTPUT);
  pinMode(mosfet2, OUTPUT);
  pinMode(mosfet3, OUTPUT);
  pinMode(mosfet4, OUTPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  TCNT1 = 0;
  TIMSK1 = 0;

  attachInterrupt(digitalPinToInterrupt(encoderPin1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), updateEncoder, CHANGE);
  sei();
  Serial.println("Hello! this project has been built from Ivan Scordato, Andrea Battello and some other Brian BVC, in Genuary 2021.");
  delay(2000);
}

void updateEncoder()
{
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit  //gestione encorder per regolazione dell'angolo --> velocità
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit
  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) Angle -= 0.1;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) Angle += 0.1;
  lastEncoded = encoded; //store this value for next time
  if (Angle > MaxAngle) Angle = MaxAngle;
  if (Angle < MinAngle) Angle = MinAngle;
  Serial.print("Angolo: ");
  Serial.println(Angle);
}

float CalcVelocity(float spazio, float tempo)
{
  return (float)(spazio / tempo);
}

void MagneticPulse(uint8_t mosfet)
{
  TCCR1B &= 0b11111000; //spegni il timer
  uint16_t TempoOn = 0;
  velocity = 0;

  if (TCNT1 > 0)
  {
    velocity = CalcVelocity(circonferenza / 4.0 , TCNT1 / 15625.0); //velocità media tra un sensore e il successivo
    TempoOn = 1000.0 * (Angle / 90.0) * (TCNT1 / 15625.0); //calcola il tempo di accensione di ogni mosfet
    TCNT1 = 0; //resetta il contatore del timer1
  }
  TCCR1B |= 0b00000101; //timer1 con prescaler 1024 -> 15625Hz

  if (TempoOn > 0)
  {
    switch (mosfet) //attivazione mosfet riferito al passaggio dal sensore che lo precede
    {
      case 1:
        digitalWrite(led1, HIGH);
        digitalWrite(mosfet1, HIGH);
        delay(TempoOn);
        digitalWrite(mosfet1, LOW);
        digitalWrite(led1, LOW);
        break;

      case 2:
        digitalWrite(led2, HIGH);
        digitalWrite(mosfet2, HIGH);
        delay(TempoOn);
        digitalWrite(mosfet2, LOW);
        digitalWrite(led2, LOW);
        break;

      case 3:
        digitalWrite(led3, HIGH);
        digitalWrite(mosfet3, HIGH);
        delay(TempoOn);
        digitalWrite(mosfet3, LOW);
        digitalWrite(led3, LOW);
        break;

      case 4:
        digitalWrite(led4, HIGH);
        digitalWrite(mosfet4, HIGH);
        delay(TempoOn);
        digitalWrite(mosfet4, LOW);
        digitalWrite(led4, LOW);
        break;
    }
  }
  Serial.print("Bobina no."); //parametri stampati sul monitor seriale
  Serial.println(mosfet);
  Serial.print("Velocità = ");
  Serial.print(velocity, 3);
  Serial.println(" m/s");
  Serial.print("Tempo On = ");
  Serial.print(TempoOn);
  Serial.println(" mS");
}

void loop()
{
  if (count >= 2000)
  {
    lcd.clear();  //parametri stampati sul display
    lcd.setCursor(0, 0);
    lcd.print("Angle = ");
    lcd.print(Angle, 1);
    lcd.setCursor(0, 1);
    lcd.print("Speed = ");
    lcd.print(velocity * 3.6, 1);
    lcd.print(" kph");
    count = 0;
  }
  if (analogRead(sensor1) > sogliaSensore) MagneticPulse(1);
  if (analogRead(sensor2) > sogliaSensore) MagneticPulse(2);
  if (analogRead(sensor3) > sogliaSensore) MagneticPulse(3);
  if (analogRead(sensor4) > sogliaSensore) MagneticPulse(4);
  count++;
}
