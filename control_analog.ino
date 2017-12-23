// int ANALOG_INPUT_PIN = 0;     // potentiometer wiper (middle terminal) connected to analog pin 3
// int period = 0;           // variable to store the value read

// class Coils{
//   public:
//     int period;
//     int currentPhase;
//     int direction;
// };

// void stepCoil(Coils *coils){
//   switch(coils -> currentPhase){
//     case 0:
//       digitalWrite(11, HIGH);
//       digitalWrite(12, LOW);
//       digitalWrite(13, LOW);
//       break;
//     case 1:
//       digitalWrite(11, HIGH);
//       digitalWrite(12, HIGH);
//       digitalWrite(13, LOW);
//       break;
//     case 2:
//       digitalWrite(11, LOW);
//       digitalWrite(12, HIGH);
//       digitalWrite(13, LOW);
//       break;
//     case 3:
//       digitalWrite(11, LOW);
//       digitalWrite(12, HIGH);
//       digitalWrite(13, HIGH);
//       break;
//     case 4:
//       digitalWrite(11, LOW);
//       digitalWrite(12, LOW);
//       digitalWrite(13, HIGH);
//       break;
//     default:
//       digitalWrite(11, HIGH);
//       digitalWrite(12, LOW);
//       digitalWrite(13, HIGH);
//       break;
//   }
//   coils->currentPhase=coils->currentPhase+coils->direction;
//   coils->currentPhase=coils->currentPhase%6;
// };

// Coils coils;
// void setup() {
//   pinMode(11, OUTPUT);//Konfiguracja pinu jako wyjścia
//   pinMode(12, OUTPUT);//Konfiguracja pinu jako wyjścia
//   pinMode(13, OUTPUT);//Konfiguracja pinu jako wyjścia
//   Serial.begin(9600);          //  setup serial
//   coils.direction= 1;
//  coils.currentPhase=0;
// }



// void loop()
// {
 
 
// stepCoil(&coils);
// delay(2);
// Serial.println("DUPA"+coils.currentPhase);
  
// }
int transPin[3] = {11,12,13}; // select the pin transitor coil
int enkoder=0;
int lastvalue=0;
int period=0;
int analog=0;
int k=0;
float velocity=0;
double suma=0;
int const FREQ_TIMER1=1;
int const FREQ_TIMER2=1000;

int const REGISTER_FREQ_TIMER1=65536-16000000/256/FREQ_TIMER1; // preload timer 65536-16MHz/256/2Hz
int const REGISTER_FREQ_TIMER2=65536-16000000/256/FREQ_TIMER2;
int const SIZE_BUFER=8;
int bufer[SIZE_BUFER];

void setup() {
  Serial.begin(9600);
  // declare the ledPin as an OUTPUT:
  const byte interruptPin = 2;
  pinMode(transPin[0], OUTPUT);
  pinMode(transPin[1], OUTPUT);
  pinMode(transPin[2], OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), pin_ISR, CHANGE);
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR2A = 0;
  TCCR2B = 0;

  TCNT1 = REGISTER_FREQ_TIMER1;            
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  

  TCNT2 = REGISTER_FREQ_TIMER2;            // preload timer 65536-16MHz/256/f
  TCCR2B |= (1 << CS22);    // 256 prescaler 
  TIMSK2 |= (1 << TOIE2);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}




void loop() {
// if (Serial.available() > 0) {
// // read the incoming byte:
// period = Serial.parseInt();
// Serial.print("I received: ");
// Serial.println(period, DEC);
// }
  if(abs(analogRead(A0)-analog)>5)
    {
      analog = analogRead(A0);
      period = map(analog,0,1023,150,3000);
    }
  switch(k){
      case 0:
        digitalWrite(transPin[0],1);
        digitalWrite(transPin[1],0);
        digitalWrite(transPin[2],0);
        break;
       case 1:
        digitalWrite(transPin[0],1);
        digitalWrite(transPin[1],1);
        digitalWrite(transPin[2],0);
        break;
        
       case 2:
        digitalWrite(transPin[0],0);
        digitalWrite(transPin[1],1);
        digitalWrite(transPin[2],0);
        break;
        
       case 3:
        digitalWrite(transPin[0],0);
        digitalWrite(transPin[1],1);
        digitalWrite(transPin[2],1);
        break;

        
       case 4:
        digitalWrite(transPin[0],0);
        digitalWrite(transPin[1],0);
        digitalWrite(transPin[2],1);
        break;

        
       case 5:
        digitalWrite(transPin[0],1);
        digitalWrite(transPin[1],0);
        digitalWrite(transPin[2],1);
        break;
        
       default:
        digitalWrite(transPin[0],0);
        digitalWrite(transPin[1],0);
        digitalWrite(transPin[2],0);
        break;
    
    }
  
  k=k+1;
  k=k%6;
  delay((period/6.0)/3.0); 
}
bool flag=false;
ISR(TIMER2_OVF_vect){
 // if(flag==true){
    TCNT2 = REGISTER_FREQ_TIMER2;            // preload timer
    for(int i=0; i<SIZE_BUFER-1; i++ ){
      bufer[i+1]=bufer[i];
    }
    bufer[0]=digitalRead(3);
  //}
}

ISR(TIMER1_OVF_vect)        // interrupt service routine that wraps a user defined function supplied by attachInterrupt
{
  TCNT1 = REGISTER_FREQ_TIMER1;            // preload timer
  velocity=(float)(enkoder*360*FREQ_TIMER1)/24.0;
  Serial.print("Coils period = ");
  Serial.print(period,DEC);
  Serial.println("[ms]");
  float theorVelocity = 360000.0/period;
  Serial.print("Theoretical velocity = ");
  Serial.print(theorVelocity,DEC);
  Serial.println("[o/s]");
  Serial.print("Velocity = ");
  Serial.print(velocity,DEC);
  Serial.println("[o/s]\n\n");
  enkoder=0;
}
int prevBufor=0;
void pin_ISR(){
  int temp=0;
  //flag=true;
  for(int i=0;i<SIZE_BUFER;i++){
    temp=temp+bufer[i];
  }
  //Serial.println(temp);
  if((temp==0 || temp==SIZE_BUFER) && temp!=prevBufor){
    //Serial.println(temp);
    enkoder++;
    prevBufor=temp;
  }
    
}
