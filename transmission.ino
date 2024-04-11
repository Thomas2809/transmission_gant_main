//***************************************************************************
// envoie_donne_m5paper.ino
// Main_Robotique
// Giarrizzo Thomas
// 6 ème électronique Inraci
// Hardwarde: Arduino_feather_ESP32, M5Stack_Paper, Adc1115(I2C), Servomoteurs, capteur_de_fexion_ZD10-100,
// 18 mars 2024


//*********************************Librairie*********************************

#include "BluetoothSerial.h"
#include <Wire.h>
#include "transmission.h"

//********************************Constante**********************************
#define NB_data 7
int point[2][2];
//#define debug_ADC

//********************************Variable***********************************
char sendtab[NB_data];
int16_t adc0, adc1, adc2;
int16_t adc6, adc7;  //   _t:

//*******************************Objet****************************************
//BluetoothSerial SerialBT;
esp_timer_handle_t timer;
//******************************
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif
//***************************************Interuption***************************
// cette fonction est appelé à chaque déclanchement du timer
void IRAM_ATTR onTimer(void *param) {

  static unsigned int cpt = 0;
  //C'est ici que tu écris ce que tu veux qui arrive toutes les 50ms
  Serial.printf("%d-> ", cpt++);  // incrémante le compteurs

  if (ads2.checkADS1115()) {  // gère les données du deuxième module

    adc7 = ads2.readVoltage(3);
    Serial.printf("A03:%5d ", adc7 / 25);

    adc6 = ads2.readVoltage(2);
    Serial.printf("A02:%5d ", adc6 / 25);
  } else {
    Serial.println("ADS1115-gauche Disconnected!");
  }

  if (ads1.checkADS1115()) {  // gère les données du premier module

    adc2 = ads1.readVoltage(2);
    Serial.printf("A12:%5d ", adc2 / 25);

    adc1 = ads1.readVoltage(1);
    Serial.printf("A11:%5d ", adc1 / 25);

    adc0 = ads1.readVoltage(0);
    Serial.printf("A10:%5d \n", 255 - adc0 / 25);
  } else {
    Serial.println("ADS1115-droite Disconnected!");
  }

  sendtab[0] = '#';
  sendtab[1] = char(255 - adc0 / 25);  //les données doivent être placé ici ...
  sendtab[2] = char(adc1 / 25);        // /25 car pas besoin de 6123 positions.
  sendtab[3] = char(adc2 / 25);
  sendtab[4] = char(adc6 / 25);
  sendtab[5] = char(adc7 / 25);
  sendtab[6] = sendtab[1] ^ sendtab[2] ^ sendtab[3] ^ sendtab[4] ^ sendtab[5];  //byte de controle d'intégrité de donnée


  for (char cptSend = 0; cptSend < NB_data; cptSend++) {
    SerialBT.print(sendtab[cptSend]);
#ifdef debug_ADC
    Serial.println(sendtab[cptSend]);
#endif
  }
}
//*******************************Initialisation**************************************
void setup() {

  init_bluetooth();

  delay(100);

  M5.begin();
  secondWire.begin(I2C_SDA, I2C_SCL, (uint32_t)400000U);  // Initialisation du deuxième bus I2C avec les broches SDA et SCL définies
  init_screen(90, 90, 540, 960, 5); // rotion x, rotation y, Longueur, largeur, taille
  init_ads();
  //************* Configuration de l'interruption du timer ************
  esp_timer_create_args_t timerArgs;       // crée une structure pour configurer le timer
  timerArgs.callback = &onTimer;           // Définition de la fonction de rappel
  timerArgs.arg = NULL;                    // rien de plus n'est passé à la fonction de rappel
  esp_timer_create(&timerArgs, &timer);    //Création du timer
  esp_timer_start_periodic(timer, 20000);  // Déclencher toutes les 20 ms
}
void loop() {

  static int cpt = 0;
  /*
  canvas.drawString("ADS1", 10, 0);
  canvas.drawString("A0: " + String(255 - adc0 / 25) + "mV", 10, 60);
  canvas.drawString("A1: " + String(adc1 / 25) + "mV", 10, 110);
  canvas.drawString("A2: " + String(adc2 / 25) + "mV", 10, 150);
  canvas.drawString("ADS2", 10, 210);
  canvas.drawString("A2: " + String(adc6 / 25) + "mV", 10, 260);
  canvas.drawString("A3: " + String(adc7 / 25) + "mV", 10, 300);

  canvas.drawString("Life CPT: " + String(cpt++), 10, 360);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  delay(20);  // Ajout d'un délai
  */
  // SerialBT.println('@');
  //Serial.println('@');

  if (M5.TP.available()) {
    if (!M5.TP.isFingerUp()) {
      M5.TP.update();
      canvas.fillCanvas(0);
      bool is_update = false;
      for (int i = 0; i < 2; i++) {
        tp_finger_t FingerItem = M5.TP.readFinger(i);
        if ((point[i][0] != FingerItem.x) || (point[i][1] != FingerItem.y)) {
          is_update = true;
          point[i][0] = FingerItem.x;
          point[i][1] = FingerItem.y;
          canvas.fillRect(FingerItem.x - 50, FingerItem.y - 50, 100,
                          100, 15);
          Serial.printf("Finger ID:%d-->X: %d*C  Y: %d  Size: %d\r\n", FingerItem.id, FingerItem.x, FingerItem.y, FingerItem.size);
          canvas.drawString("X: " + String(FingerItem.x) + " "
                                                           "Y:"
                              + String(FingerItem.y),
                            10, 60);
        }
      }
      if (is_update) {
        canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
      }
    }
  }
}
