#include <PS4Controller.h>
#include <ps4.h>
#include <ps4_int.h>


#define RIGHT 1
#define LEFT 0

const int ENA_RIGHT = 18;
const int ENA_LEFT = 25;

const int motor1pin1 = 19;  // IN1&2 for Motor RIGHT
const int motor1pin2 = 21;

const int motor2pin3 = 33;  // IN3&4 for Motor LEFT
const int motor2pin4 = 32;

//Set PWM config
const int PWM_CHANNEL_RIGHT = 0;     
const int PWM_CHANNEL_LEFT = 1;  
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

const int SENSVN = 34;

int throttle = 0;
int stickValue = 0;
int steeringValueLEFT = 0;
int steeringValueRIGHT = 0;

int i = 0;

#define Measurment 100
int voltArray[Measurment];
int voltIndex = 0;
bool lowBatteryWarned = false;
const int ADC_LOW  = n;
const int ADC_HIGH = n;


void setup() {

  //pinMode Motor RIGHT
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);

  //pinMode Motor LEFT
  pinMode(motor2pin3, OUTPUT);
  pinMode(motor2pin4, OUTPUT);

  pinMode(ENA_RIGHT, OUTPUT);
  pinMode(ENA_LEFT, OUTPUT);

  pinMode(SENSVN, INPUT);

  ledcAttachChannel(ENA_RIGHT, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL_RIGHT);
  ledcAttachChannel(ENA_LEFT, PWM_FREQ, PWM_RESOLUTION, PWM_CHANNEL_LEFT);

  setMotor(0, RIGHT);
  setMotor(0, LEFT);

  Serial.begin(115200); 
  //Serial.begin(9600);
  PS4.begin("e8:9e:b4:d9:df:48");
}

void loop() 
{
  getMeasurment();

  if(PS4.Cross())
  {
    stickValue = PS4.LStickX();
    if(stickValue > 10)
    {
      setMotor(map(stickValue, 10, 128, 0, 255), LEFT);
      setMotor(map(stickValue, 10, 128, 0, -255), RIGHT);
    }
    else if(stickValue < -10)
    {
      setMotor(map(stickValue, -128, -10, -255, 0), LEFT);
      setMotor(map(stickValue, -128, -10, 255, 0), RIGHT);
    }
    else
    {
      setMotor(0, LEFT),
      setMotor(0, RIGHT);
    }
  }
  else
  {
    getSteering();
    if(PS4.Circle())
    {
      steeringValueLEFT = map(steeringValueLEFT, -255, 255, -200, 200);
      steeringValueRIGHT = map(steeringValueRIGHT, -255, 255, -200, 200);
    }
    setMotor(steeringValueLEFT, LEFT);
    setMotor(steeringValueRIGHT, RIGHT);
  }
}


//trackSelect 0 = LEFT; 1 = RIGHT
void setMotor(int speedVal, bool trackSelect)
{
  speedVal = constrain(speedVal, -255, 255);
  if (speedVal > 0) 
  {
    if(trackSelect) //if right track
    {
      digitalWrite(motor1pin1, HIGH);
      digitalWrite(motor1pin2, LOW);
      ledcWriteChannel(PWM_CHANNEL_RIGHT, speedVal);
    }
    else //if left track
    {
      digitalWrite(motor2pin3, LOW);
      digitalWrite(motor2pin4, HIGH);
      ledcWriteChannel(PWM_CHANNEL_LEFT, speedVal);
    }


  } 
  else if (speedVal < 0) 
  {
    // Reverse rotation
    if(trackSelect) //if right track
    {
      digitalWrite(motor1pin1, LOW);
      digitalWrite(motor1pin2, HIGH);
      ledcWriteChannel(PWM_CHANNEL_RIGHT, abs(speedVal));
    }
    else //if left track
    {
      digitalWrite(motor2pin3, HIGH);
      digitalWrite(motor2pin4, LOW);
      ledcWriteChannel(PWM_CHANNEL_LEFT, abs(speedVal));
    }
  } 
  else 
  {
    // Stop: freewheel (both LOW). 
    // For stronger braking, set both HIGH instead.
    if(trackSelect) //if right track
    {
      digitalWrite(motor1pin1, LOW);
      digitalWrite(motor1pin2, LOW);
      ledcWriteChannel(PWM_CHANNEL_RIGHT, speedVal);
    }
    else //if left track
    {
      digitalWrite(motor2pin3, LOW);
      digitalWrite(motor2pin4, LOW);
      ledcWriteChannel(PWM_CHANNEL_LEFT, speedVal);
    }
  }
}
// Aggressiv mapping for R2 and L2 joysticks
int mapAntiDeadzone(int raw, int T0, float gamma, int minPWM, float gain) {
  if (raw <= T0) return 0;
  float x = (float)(raw - T0) / (255 - T0);
  float y = pow(x, 1.0 / gamma);
  int out = (int)(y * 255);
  if (out > 0 && out < minPWM) out = minPWM;
  out = (int)(out * gain);
  if (out > 255) out = 255;
  return out;
}

void getSteering()
{
  const int   T0    = 35;    // mindre dødgang
  const float gamma = 2.2f;  // tidlig respons
  const int   minPWM= 50;    // høyere kickstart
  const float gain  = 1.15f; // ekstra forsterkning

  if(PS4.R2())
  {
    int rawdata = PS4.R2Value();
    throttle = mapAntiDeadzone(rawdata, T0, gamma, minPWM, gain);
  }
  else if(PS4.L2())
  {
    int rawdata = PS4.L2Value();
    throttle = -mapAntiDeadzone(rawdata, T0, gamma, minPWM, gain);
  }
  else
  {
    throttle = 0;
  }

  stickValue = PS4.LStickX();
  //Serial.println(stickValue);
  if(stickValue > 10)
  {
    steeringValueRIGHT = throttle;
    steeringValueLEFT = steeringValueRIGHT - map(stickValue, 10, 128, 0, steeringValueRIGHT);
  }
  else if(stickValue < -10)
  {
    steeringValueLEFT = throttle;
    steeringValueRIGHT = steeringValueLEFT - map(stickValue, -128, -10, steeringValueLEFT, 0);
  }
  else
  {
    steeringValueRIGHT = throttle;
    steeringValueLEFT = throttle;
  }
}

void getMeasurment()
{
  int rawdata = analogRead(SENSVN);     // Readin raw adc value from 0 to 4095
  voltArray[voltIndex] = rawdata;       // save in the array
  voltIndex++;                      // point to next posisiton

  if (voltIndex >= Measurment)      // 100 measurment
  {
    voltIndex = 0;                  // Start ny measurment

    long sum = 0;                   // Sum all 100
    for (int j = 0; j < Measurment; j++)
    {
      sum += voltArray[j];
    }

    float avg = sum / (float)Measurment;  // Average of 100 measurments

    Serial.print("Avg VN: ");
    Serial.println(avg);

    // threshold for raw ADC
    if (avg < ADC_LOW && !lowBatteryWarned)
    {
      lowBatteryWarned = true;    
      PS4.setRumble(200, 255);      
    }
    else if (avg > ADC_HIGH)
    {
      lowBatteryWarned = false; 
    }
  }
}
