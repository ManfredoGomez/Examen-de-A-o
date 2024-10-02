#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Adafruit_NeoPixel.h>  // Biblioteca para NeoPixel

// Configuración para la tira de NeoPixel
#define PIN 12        // Pin donde está conectada la tira de LEDs
#define NUMPIXELS 30 // Número de LEDs en la tira
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Inicializa el acelerómetro
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Inicializa el DFPlayer Mini
SoftwareSerial mySerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

float lastX, lastY, lastZ; // Últimos valores de las lecturas del acelerómetro
#define THRESHOLD 1.50  // Umbral para detectar movimiento
#define DEBOUNCE_DELAY 2000 // Retardo de 2 segundos para evitar múltiples detecciones

int currentSound = 1;  // Contador para cambiar el sonido
bool firstMovement = false;  // Bandera para evitar reproducción en el inicio
bool isMoving = false;  // Bandera para determinar si hay movimiento
unsigned long lastMoveTime = 0; // Tiempo del último movimiento

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  // Inicializa el DFPlayer Mini
  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("Error al iniciar el DFPlayer Mini.");
    while (true);
  }

  myDFPlayer.volume(20); // Ajusta el volumen (0-30)

  // Inicializa el acelerómetro
  if (!accel.begin()) {
    Serial.println("Error al inicializar el ADXL345.");
    while (true);
  }

  // Establece el rango del acelerómetro a 2G
  accel.setRange(ADXL345_RANGE_2_G);

  // Inicializa las primeras lecturas del acelerómetro para evitar falsos movimientos al iniciar
  sensors_event_t event;
  accel.getEvent(&event);
  lastX = event.acceleration.x;
  lastY = event.acceleration.y;
  lastZ = event.acceleration.z;

  // Inicializa la tira de NeoPixel
  strip.begin();
  strip.show();  // Asegura que todos los LEDs estén apagados al inicio
  Serial.println("Sistema listo. Esperando movimiento...");

  // Enciende la tira de NeoPixel con efecto sable de luz blanco
  lightsaberEffect();
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  // Lectura actual del acelerómetro
  float currentX = event.acceleration.x;
  float currentY = event.acceleration.y;
  float currentZ = event.acceleration.z;

  // Detecta si hay un cambio significativo (movimiento)
  if (abs(currentX - lastX) > THRESHOLD || abs(currentY - lastY) > THRESHOLD || abs(currentZ - lastZ) > THRESHOLD) {
    unsigned long currentTime = millis();

    // Aplica debounce para evitar múltiples detecciones del mismo movimiento
    if (currentTime - lastMoveTime > DEBOUNCE_DELAY) {
      if (!firstMovement) {
        firstMovement = true;  // Marca que ya ha habido un movimiento
        Serial.println("Primer movimiento detectado.");
      } else {
        // Si ya ha habido movimiento, reproduce el sonido correspondiente
        Serial.print("Movimiento detectado. Reproduciendo sonido ");
        Serial.println(currentSound);

        // Reproduce el sonido basado en el contador
        myDFPlayer.play(currentSound);

        // Cambia la secuencia de luces dependiendo del sonido
        switch (currentSound) {
          case 1:
            sequence1();  // Ejecuta la secuencia de colores para el sonido 1
            delay(3000);  // Duración del sonido 1: 3 segundos
            break;
          case 2:
            sequence2();  // Ejecuta la secuencia de colores para el sonido 2
            delay(9000);  // Duración del sonido 2: 9 segundos
            break;
          case 3:
            sequence3();  // Ejecuta la secuencia de colores para el sonido 3
            delay(3000);  // Duración del sonido 3: 3 segundos
            break;
        }

        // Cambia el sonido para la próxima detección de movimiento
        currentSound++;
        if (currentSound > 3) {
          currentSound = 1;  // Regresa al primer sonido después de reproducir el tercero
        }
      }

      isMoving = true;  // Marca que se detectó movimiento
      lastMoveTime = currentTime;  // Actualiza el tiempo del último movimiento
    }
  } else {
    if (isMoving) {
      // Solo imprime "Sin movimiento" una vez después de que ha habido movimiento
      Serial.println("Sin movimiento.");
      strip.fill(strip.Color(255, 255, 255));  // Enciende la tira de color blanco
      strip.show();
      isMoving = false;  // Resetea la bandera cuando no hay movimiento
    }
  }

  // Actualiza los valores anteriores con las lecturas actuales
  lastX = currentX;
  lastY = currentY;
  lastZ = currentZ;

  // Pequeño retardo para evitar saturar el puerto serie
  delay(200);
}

// Efecto tipo sable de luz
void lightsaberEffect() {
  // Encender cada LED uno por uno
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));  // Blanco
    strip.show();
    delay(50);  // Ajusta el retardo para controlar la velocidad del encendido
  }

  // Apagar cada LED uno por uno
  for (int i = NUMPIXELS - 1; i >= 0; i--) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));  // Apagar
    strip.show();
    delay(50);  // Ajusta el retardo para controlar la velocidad del apagado
  }
}

// Secuencia de colores para el sonido 1
void sequence1() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));  // Rojo
    strip.show();
    delay(50);
  }
  strip.clear();
}

// Secuencia de colores para el sonido 2
void sequence2() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0));  // Verde
    strip.show();
    delay(50);
  }
  strip.clear();
}

// Secuencia de colores para el sonido 3
void sequence3() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 255));  // Azul
    strip.show();
    delay(50);
  }
  strip.clear();
}
