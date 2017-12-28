const int INTERRUPT_PIN = 2;
int TRANS_PIN[3] = {11,12,13};
int const FREQ_TIMER1=1;
int const REGISTER_FREQ_TIMER1=65536-16000000/256/FREQ_TIMER1; // preload timer 65536-16MHz/256/2Hz

enum Direction
{
    forward = 1,
    backward = -1
};
class SpeedCounter{
  public:
    int theorVelocity;
    int encoderPerSec;
    int speed;
    int prevEncoderState;
    void incrementEncoder();
    bool isEncoderInpulseValid();
    void updateVelocity(int argPeriod);
    SpeedCounter();
};
SpeedCounter::SpeedCounter(){
  theorVelocity=0;
  encoderPerSec=0;
  speed=0;
  prevEncoderState=0;
};
void SpeedCounter::incrementEncoder(){
  encoderPerSec++;
};

bool SpeedCounter::isEncoderInpulseValid(){
  int temp=digitalRead(INTERRUPT_PIN);
  delay(1);
  if(temp==digitalRead(INTERRUPT_PIN) && temp!=prevEncoderState){
    prevEncoderState=temp;
    return true;
  }
  return false;
};

void SpeedCounter::updateVelocity(int argPeriod){
  theorVelocity=360000/argPeriod;
  speed=(encoderPerSec*360*FREQ_TIMER1)/24;
  // encoderPerSec=0;
};

class CoilsControler{
  public:
    int period;
    int currentPhase;
    int analogInput;
    Direction direction;
    SpeedCounter speedCounter;
    CoilsControler();
    void stepHalfStep();
    void stepFullStep();
    void stepDoubleStep();
    void setPeriod();
};
CoilsControler::CoilsControler(){
  period=1000;
  currentPhase=0;
  direction=forward;
  analogInput=0;
};
void CoilsControler::stepHalfStep(){
  switch(currentPhase){
    case 0:
      digitalWrite(TRANS_PIN[0], HIGH);
      digitalWrite(TRANS_PIN[1], LOW);
      digitalWrite(TRANS_PIN[2], LOW);
      break;
    case 1:
      digitalWrite(TRANS_PIN[0], HIGH);
      digitalWrite(TRANS_PIN[1], HIGH);
      digitalWrite(TRANS_PIN[2], LOW);
      break;
    case 2:
      digitalWrite(TRANS_PIN[0], LOW);
      digitalWrite(TRANS_PIN[1], HIGH);
      digitalWrite(TRANS_PIN[2], LOW);
      break;
    case 3:
      digitalWrite(TRANS_PIN[0], LOW);
      digitalWrite(TRANS_PIN[1], HIGH);
      digitalWrite(TRANS_PIN[2], HIGH);
      break;
    case 4:
      digitalWrite(TRANS_PIN[0], LOW);
      digitalWrite(TRANS_PIN[1], LOW);
      digitalWrite(TRANS_PIN[2], HIGH);
      break;
    default:
      digitalWrite(TRANS_PIN[0], HIGH);
      digitalWrite(TRANS_PIN[1], LOW);
      digitalWrite(TRANS_PIN[2], HIGH);
      break;
  }
  currentPhase=(currentPhase+direction)%6;
  // Serial.println(currentPhase,DEC);
  delay(period/18);
};

void CoilsControler::stepFullStep(){
  switch(currentPhase){
    case 0:
      digitalWrite(TRANS_PIN[0], HIGH);
      digitalWrite(TRANS_PIN[1], LOW);
      digitalWrite(TRANS_PIN[2], LOW);
      break;
    case 1:
      digitalWrite(TRANS_PIN[0], LOW);
      digitalWrite(TRANS_PIN[1], HIGH);
      digitalWrite(TRANS_PIN[2], LOW);
      break;
    default:
      digitalWrite(TRANS_PIN[0], LOW);
      digitalWrite(TRANS_PIN[1], LOW);
      digitalWrite(TRANS_PIN[2], HIGH);
      break;
  }
  currentPhase=(currentPhase+direction)%3;
  // Serial.println(currentPhase,DEC);
  delay(period/9);
};

void CoilsControler::stepDoubleStep(){
  switch(currentPhase){
    case 0:
      digitalWrite(TRANS_PIN[0], HIGH);
      digitalWrite(TRANS_PIN[1], LOW);
      digitalWrite(TRANS_PIN[2], HIGH);
      break;
    case 1:
      digitalWrite(TRANS_PIN[0], HIGH);
      digitalWrite(TRANS_PIN[1], HIGH);
      digitalWrite(TRANS_PIN[2], LOW);
      break;
    default:
      digitalWrite(TRANS_PIN[0], LOW);
      digitalWrite(TRANS_PIN[1], HIGH);
      digitalWrite(TRANS_PIN[2], HIGH);
      break;
  }
  currentPhase=(currentPhase+direction)%3;
  // Serial.println(currentPhase,DEC);
  delay(period/9);
};

void CoilsControler::setPeriod(){
  int analog = analogRead(A0);
  if(abs(analogInput-analog)>5){
    period = map(analog,0,1023,150,3000);
    analogInput=analog;
  }
};

CoilsControler coilsControler;

void setup() {
  Serial.begin(9600);
  pinMode(TRANS_PIN[0], OUTPUT);
  pinMode(TRANS_PIN[1], OUTPUT);
  pinMode(TRANS_PIN[2], OUTPUT);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), pin_ISR, CHANGE);
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = REGISTER_FREQ_TIMER1;            
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_OVF_vect)        // interrupt service routine that wraps a user defined function supplied by attachInterrupt
{
  TCNT1 = REGISTER_FREQ_TIMER1;            // preload timer
  coilsControler.speedCounter.updateVelocity(coilsControler.period);
  Serial.print("Coils period = ");
  Serial.print(coilsControler.period,DEC);
  Serial.println("[ms]");
  Serial.print("Theoretical velocity = ");
  Serial.print(coilsControler.speedCounter.theorVelocity,DEC);
  Serial.println("[o/s]");
  Serial.print("Velocity = ");
  Serial.print(coilsControler.speedCounter.speed,DEC);
  Serial.println("[o/s]");
  Serial.print("Encoder inpulse per s = ");
  Serial.print(coilsControler.speedCounter.encoderPerSec,DEC);
  Serial.println("[inpulse/s]\n\n");
  coilsControler.speedCounter.encoderPerSec=0;
}

void pin_ISR(){
  if(coilsControler.speedCounter.isEncoderInpulseValid()){
    coilsControler.speedCounter.incrementEncoder();
  }
}

void loop(){
  coilsControler.setPeriod();
  coilsControler.stepDoubleStep();
}


///////////////////////////////////////////////////////////


// int TRANS_PIN[3] = {11,12,13}; // select the pin transitor coil
// int encoder=0;
// int lastvalue=0;
// int period=0;
// int analog=0;
// int k=0;
// int velocity=0;
// double suma=0;
// int const FREQ_TIMER1=1;
// int const FREQ_TIMER2=8000;

// int const REGISTER_FREQ_TIMER1=65536-16000000/256/FREQ_TIMER1; // preload timer 65536-16MHz/256/2Hz
// int const REGISTER_FREQ_TIMER2=65536-16000000/256/FREQ_TIMER2;
// int const SIZE_SHIFT_REG=16;
// int SHIFT_REG_ENC[SIZE_SHIFT_REG];
// const byte INTERRUPT_PIN = 2;
  
// void setup() {
//   Serial.begin(9600);
//   // declare the ledPin as an OUTPUT:
//   pinMode(TRANS_PIN[0], OUTPUT);
//   pinMode(TRANS_PIN[1], OUTPUT);
//   pinMode(TRANS_PIN[2], OUTPUT);
//   pinMode(INTERRUPT_PIN, INPUT_PULLUP);
//   attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), pin_ISR, CHANGE);
//   noInterrupts();           // disable all interrupts
//   TCCR1A = 0;
//   TCCR1B = 0;
//   TCCR2A = 0;
//   TCCR2B = 0;

//   TCNT1 = REGISTER_FREQ_TIMER1;            
//   TCCR1B |= (1 << CS12);    // 256 prescaler 
//   TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
//   interrupts();             // enable all interrupts
// }




// void loop() {
// // if (Serial.available() > 0) {
// // // read the incoming byte:
// // period = Serial.parseInt();
// // Serial.print("I received: ");
// // Serial.println(period, DEC);
// // }
//   if(abs(analogRead(A0)-analog)>5)
//     {
//       analog = analogRead(A0);
//       period = map(analog,0,1023,150,3000);
//     }
//   switch(k){
//       case 0:
//         digitalWrite(TRANS_PIN[0],1);
//         digitalWrite(TRANS_PIN[1],0);
//         digitalWrite(TRANS_PIN[2],0);
//         break;
//        case 1:
//         digitalWrite(TRANS_PIN[0],1);
//         digitalWrite(TRANS_PIN[1],1);
//         digitalWrite(TRANS_PIN[2],0);
//         break;
        
//        case 2:
//         digitalWrite(TRANS_PIN[0],0);
//         digitalWrite(TRANS_PIN[1],1);
//         digitalWrite(TRANS_PIN[2],0);
//         break;
        
//        case 3:
//         digitalWrite(TRANS_PIN[0],0);
//         digitalWrite(TRANS_PIN[1],1);
//         digitalWrite(TRANS_PIN[2],1);
//         break;

        
//        case 4:
//         digitalWrite(TRANS_PIN[0],0);
//         digitalWrite(TRANS_PIN[1],0);
//         digitalWrite(TRANS_PIN[2],1);
//         break;

        
//        case 5:
//         digitalWrite(TRANS_PIN[0],1);
//         digitalWrite(TRANS_PIN[1],0);
//         digitalWrite(TRANS_PIN[2],1);
//         break;
        
//        default:
//         digitalWrite(TRANS_PIN[0],0);
//         digitalWrite(TRANS_PIN[1],0);
//         digitalWrite(TRANS_PIN[2],0);
//         break;
    
//     }
//   k=k+1;
//   k=k%6;
//   delay((period/6.0)/3.0);
// }

// ISR(TIMER1_OVF_vect)        // interrupt service routine that wraps a user defined function supplied by attachInterrupt
// {
//   TCNT1 = REGISTER_FREQ_TIMER1;            // preload timer
//   velocity=(encoder*360*FREQ_TIMER1)/24;
//   Serial.print("Coils period = ");
//   Serial.print(period,DEC);
//   Serial.println("[ms]");
//   float theorVelocity = 360000.0/period;
//   Serial.print("Theoretical velocity = ");
//   Serial.print(theorVelocity,DEC);
//   Serial.println("[o/s]");
//   Serial.print("Velocity = ");
//   Serial.print(velocity,DEC);
//   Serial.println("[o/s]\n\n");
//   encoder=0;
// }

// int prev=0;
// void pin_ISR(){
//   int temp=0;
//   temp=digitalRead(INTERRUPT_PIN);
//   delay(1);
//   if(temp==digitalRead(INTERRUPT_PIN) && temp!=prev){
//     encoder++;
//     prev=temp;
//   }
// }
