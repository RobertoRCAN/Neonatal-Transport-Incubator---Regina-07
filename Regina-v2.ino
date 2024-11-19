#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MLX90614.h>
#include <DHT.h>
#include <Tone.h>

double TNeo;
double TAmb;
double VoltBateria;
double HAmb;
double i;
boolean b_ModoAmb;
boolean b_DesconexionAC;
boolean b_NivelAgua;

// Inicialización de la pantalla LCD y botones
LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_MLX90614 mlx90614 = Adafruit_MLX90614();
DHT dht15(15, DHT22);

void ManejarAlarmas() {
  if (b_ModoAmb && TAmb >= 38) {
    TEMP__ALTA();
  } else if (!b_ModoAmb && TNeo >= 37) {
    TEMP__ALTA();
  } else if (VoltBateria >= 30) {
    BAT__BAJA();
  } else if (b_DesconexionAC) {
    FALLA_EN_CARGA();
  } else if (b_NivelAgua) {
    SIN_AGUA();
  }
}

void BAT__BAJA() {
  lcd.clear();
  while (digitalRead(27) == LOW) {  // Leer el botón de la batería baja
    lcd.setCursor(0, 1);
    lcd.print("BAT. BAJA");
    for (i = 1; i <= 2; i++) {
      tone(32, 1000, 500);  // Generar tono de advertencia
      delay(100);
    }
    delay(1000);
  }
  lcd.clear();
}

void LeerSensores() {
  TNeo = mlx90614.readObjectTempC();
  TAmb = dht15.readTemperature();
  HAmb = dht15.readHumidity();
  b_NivelAgua = digitalRead(16);  // Leer el estado del botón de nivel de agua
  b_DesconexionAC = digitalRead(19);  // Leer desconexión de AC
  VoltBateria = map(analogRead(2) * 5, 0, 13, 1, 100);  // Calcular el voltaje de la batería
}

void ControlarActuadores() {
  if (b_ModoAmb) {
    analogWrite(13, (uint16_t)(map(TAmb, 5, 40, 5, 204)));  // Controlar actuador en modo ambiente
  } else {
    analogWrite(13, (uint16_t)(map(TNeo, 5, 40, 5, 204)));  // Controlar actuador en modo neonato
  }
  analogWrite(5, (uint16_t)(map(HAmb, 0, 100, 100, 255)));  // Controlar actuador de humedad
  analogWrite(14, (uint16_t)(map(HAmb, 0, 100, 200, 255)));  // Controlar otro actuador de humedad
}

void IRAM_ATTR fnc_interruptHandler_21() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("FALLA_SENSOR_TH");
  while (true) {
    for (i = 1; i <= 3; i++) {
      tone(32, 1000, 1500);  // Generar tono de fallo
      delay(100);
    }
    delay(1000);
  }
}

void TEMP__ALTA() {
  lcd.clear();
  while (digitalRead(27) == LOW) {  // Leer botón para detener alarma de temperatura
    lcd.setCursor(0, 1);
    lcd.print("TEMP. ALTA");
    for (i = 1; i <= 3; i++) {
      tone(32, 1000, 500);  // Generar tono de advertencia de temperatura alta
      delay(100);
    }
    delay(1000);
  }
  lcd.clear();
}

void ControlarLCD() {
  if (b_ModoAmb) {
    lcd.setCursor(0, 0);
    lcd.print("Temp. Ambiente: " + String(TAmb) + "°C");
    lcd.setCursor(0, 2);
    lcd.print("Modo: T. Ambiente");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp. Paciente: " + String(TNeo) + "°C");
    lcd.setCursor(0, 2);
    lcd.print("Modo: T. Neonato");
  }

  lcd.setCursor(0, 1);
  lcd.print("Hum. Ambiente: " + String(HAmb) + "%");
  lcd.setCursor(0, 3);
  lcd.print(" Pulse para config. ");
}

void IRAM_ATTR fnc_interruptHandler_15() {
  lcd.clear();
  while (digitalRead(27) == LOW) {  // Leer botón para detener alarma de sensor TC
    lcd.setCursor(0, 1);
    lcd.print("FALLA_SENSOR_TC");
    for (i = 1; i <= 2; i++) {
      tone(32, 1000, 1500);  // Generar tono de fallo
      delay(100);
    }
    delay(1000);
  }
  lcd.clear();
}

void SIN_AGUA() {
  lcd.clear();
  while (digitalRead(27) == LOW) {  // Leer botón para detener alarma de agua
    lcd.setCursor(0, 1);
    lcd.print("SIN AGUA");
    for (i = 1; i <= 4; i++) {
      tone(32, 1000, 500);  // Generar tono de advertencia de falta de agua
      delay(100);
    }
    delay(1000);
  }
  lcd.clear();
}

void FALLA_EN_CARGA() {
  lcd.clear();
  while (digitalRead(27) == LOW) {  // Leer botón para detener alarma de carga
    lcd.setCursor(0, 1);
    lcd.print("FALLA EN CARGA");
    tone(32, 1000, 500);
    delay(100);
    tone(32, 1000, 1500);
    delay(1000);
  }
  lcd.clear();
}

void setup() {
  pinMode(27, INPUT);  // Configurar botón de entrada
  pinMode(32, OUTPUT);  // Configurar pin de salida para tono
  pinMode(15, INPUT);
  pinMode(16, INPUT);
  pinMode(19, INPUT);
  pinMode(13, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(21, INPUT);

  mlx90614.begin();
  dht15.begin();
  pinMode(2, INPUT);
  
  // Configurar interrupciones
  attachInterrupt(digitalPinToInterrupt(21), fnc_interruptHandler_21, CHANGE);
  attachInterrupt(digitalPinToInterrupt(15), fnc_interruptHandler_15, CHANGE);

  b_ModoAmb = true;
  lcd.begin();
  lcd.noCursor();
  lcd.backlight();
  lcd.setCursor(6, 0);
  lcd.print("Iniciando...");
  delay(2000);
  lcd.clear();
}

void loop() {
  LeerSensores();
  ControlarActuadores();
  ControlarLCD();
  ManejarAlarmas();
}

