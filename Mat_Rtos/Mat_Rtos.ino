#include <Arduino_FreeRTOS.h>

//midi constants
  int noteon = 144;
  int noteoff = 128;
  int cc = 176;

//breath controller variables
  int brth = A0;
  int intensity = 0;
  int last = 0;
  int reading;
  int sensorIni;
  
  //Mux control pins for analog signal (SIG_pin) default
  const byte s0 = 1;
  const byte s1 = 0;
  const byte s2 = 9;
  const byte s3 = 8;
  
  //Mux control pins for Output signal (OUT_pin) default 
  const byte w0 = 7;
  const byte w1 = 6;
  const byte w2 = 5;
  const byte w3 = 4;
  
  //Mux in "SIG" pin default 
  const byte SIG_pin = A1;
  
  //Mux out "SIG" pin default
  const byte OUT_pin = 2;
  
  //Row and Column pins default
  const byte STATUS_pin = 13;
  
  //Array of values for selecting the disered channel of the multiplexers
  const boolean muxChannel[16][4] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    {0, 1, 1, 0}, //channel 6
    {1, 1, 1, 0}, //channel 7
    {0, 0, 0, 1}, //channel 8
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    {1, 1, 1, 1} //channel 15
  };
  
  const int numofsens = 15;                                         //Num of fisical sensors by column or row (squarematrix)
  const int numofbuttons = 3;                                       //Num of desire buttons
  const int numofsensbybut = numofsens / numofbuttons;              //Num of sensors by button
  int matrix[numofsens][numofsens];                                 //Matrix for storing the analog values
  int minimo;                                                       //Variable for storing the minimun in the array
  int maximo;                                                       //Variable for storing the maximun in the array
  int media = 0;                                                    //Variable for determine the threshold for makin on/of a button
  bool boton[numofbuttons][numofbuttons];                           //Actual state (on/off) of the desired buttons Matrix
  bool antboton[numofbuttons][numofbuttons];                        //Previous state (on/of) ot the desired buttons Matrix
  const byte miditones[numofbuttons][numofbuttons] = {    
                                                {57,41,49},
                                                {48,45,51},
                                                {36,38,46} };       //Desired midi tones per button matrix
  const int umbral = 30;                                            //Threshold percentage
  const int maximopeso = 350;                                       //Variable for detecting when no weight is on the mat
  

void TaskMatrice( void *pvParameters );
void TaskBreath( void *pvParameters );

void setup() {
  
  Serial.begin(9600);

  delay(200);
  sensorIni = analogRead(brth);
  
  while (!Serial) {
    ; 
  }

  xTaskCreate(
    TaskMatrice
    ,  "TaskMatrice"   
    ,  128  
    ,  NULL
    ,  2  
    ,  NULL );

  xTaskCreate(
    TaskBreath
    ,  "TaskBreath"
    ,  128  
    ,  NULL
    ,  2  
    ,  NULL );
}

void loop()
{
  //empty
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskMatrice(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  pinMode(w0, OUTPUT);
  pinMode(w1, OUTPUT);
  pinMode(w2, OUTPUT);
  pinMode(w3, OUTPUT);

  pinMode(OUT_pin, OUTPUT);

  pinMode(STATUS_pin, OUTPUT);

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  digitalWrite(w0, LOW);
  digitalWrite(w1, LOW);
  digitalWrite(w2, LOW);
  digitalWrite(w3, LOW);

  digitalWrite(OUT_pin, HIGH);
  digitalWrite(STATUS_pin, LOW);


  for (;;)
  {
      storeanalog();
    
    for (int j = 0; j < numofbuttons; j++) {
      for (int i = 0; i < numofbuttons; i++) {
        boton[j][i] = readboton(j, i);
        byte tono = miditones[j][i];
        if (boton[j][i] == 1 && antboton[j][i] == 0) {
          //usbMIDI.sendNoteOn(tono,127,1);
          //MIDI.sendNoteOn(tono, 127, 1);
          MIDImessage(noteon,tono,127);
          digitalWrite(STATUS_pin, !digitalRead(STATUS_pin));
  //        Serial.print("Boton ");
  //        Serial.print(tono);
  //        Serial.println(" presionado");
          antboton[j][i] = 1;
        }
        if (boton[j][i] == 0 && antboton[j][i] == 1) {
          //usbMIDI.sendNoteOff(tono,127,1);
          //MIDI.sendNoteOff(tono, 127, 1);
          MIDImessage(noteoff,tono,0);
          digitalWrite(STATUS_pin, !digitalRead(STATUS_pin));
  //        Serial.print("Boton ");
  //        Serial.print(tono);
  //        Serial.println(" no presionado");
          antboton[j][i] = 0;
        }
      }
    }
  }
}

void TaskBreath(void *pvParameters)  
{
  (void) pvParameters;
  
  
  
  

  

  for (;;)
  {
    reading = constrain(analogRead(brth)-sensorIni, 0, 1014-sensorIni);
        intensity = map(reading, 0, 1014-sensorIni, 0, 127);

        if(intensity > 0 && intensity != last){
          //MIDI.sendControlChange(11, intensity, 1);
          //usbMIDI.sendControlChange(11,intensity,1);
          //controlChange(1,11,intensity);
          MIDImessage(cc,11,intensity);
          last = intensity;
        }


        else{
          if(intensity == 0 && last > 0){
            //MIDI.sendControlChange(11, 0, 1);
            //controlChange(1,11,0);
            //usbMIDI.sendControlChange(11,0,1);
            MIDImessage(cc,11,0);
            last = 0;
          }
        }
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}


//Multiplexer read function
int readMux(byte channel){
  byte controlPin[] = {s0, s1, s2, s3};

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}

//Multiplexer write function
void writeMux(byte channel){
  byte controlPin[] = {w0, w1, w2, w3};

  //loop through the 4 sig
  for(byte i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
}

//Store all the values founded in all the columns and rows, calculate the maximun, minimum and the threshold value
void storeanalog() {
  minimo = 1024;
  maximo = 0;
  for (int j = 0; j < numofsens; j++) {
    writeMux(j);
    for (int i = 0; i < numofsens; i++) {
      matrix[j][i] = readMux(i);
      if (matrix[j][i] < minimo)
        minimo = matrix[j][i];
      if (matrix[j][i] > maximo)
        maximo = matrix[j][i];
    }
  }
  media = minimo + ((maximo - minimo) * umbral)/100;
}

//This function retuns for the disered number of buttons the on/off state
boolean readboton(int col, int ren) {
  int botonpres = 0;
  for (int j = col * numofsensbybut; j < col * numofsensbybut + numofsensbybut; j++) {
    for (int i = ren * numofsensbybut; i < ren * numofsensbybut + numofsensbybut; i++) {
      if (matrix[j][i] > media && matrix[j][i] >= maximopeso) {
        botonpres = botonpres + 1;
      }
    }
  }
  if (botonpres > 0) {
    return 1;
  }
  else {
    return 0;
  }
}

//send MIDI message
void MIDImessage(byte command, byte data1, byte data2) {
  Serial.write(command);
  Serial.write(data1);
  Serial.write(data2);
}
