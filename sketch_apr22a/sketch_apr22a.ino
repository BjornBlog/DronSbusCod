#include <sbus.h> //https://github.com/bolderflight/sbus/tree/main
#include <ibus.h> //https://thenerdyengineer.com/ibus-and-arduino/#Using_iBUS_with_an_Arduino

/*
ibus.channels[i] - value for channel i
IBUS ibus(&Serial1, 115200); - starts serial rx on port 1 at 115200 baud, DONT USE OTHER IBUS IS AT THAT SPEED
ibus.update(); - reads declared serial input
*/

/* SBUS object, writing SBUS */
bfs::SbusTx sbus_tx(&Serial1);

IBUS ibus(&Serial1, 115200);
/* SBUS data */
bfs::SbusData data;
// int channelValues[] = {1500, 1500, 1500, 885, 1500, 1500, 1500};
int pitch; // pitch is broken?
int roll;
int yaw;
int throttle;
int arming;
bool enabled = false;
long stateChangeT0 = 0;
long blinkTime = 0;
int state = -1;  
int killswitch = 1000;
int failsafe = 1000;
int LEDpin = 13;
int compare = 1000;
bool lightOn = false;
int rampUpTime = 0;
/*
-1: Boot State
0: Arming state
1: Control State
2: Test State
*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {}
  stateChangeT0 = millis();  
  blinkTime = millis();
  /* Begin the SBUS communication */
  sbus_tx.Begin();
  roll = 1500;
  pitch = 1500;
  yaw = 1500;
  throttle = 880;
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  ibus.update();
  if (state == -1){
  compare = 1000;       
  throttle = 885;
  killswitch = 1000;
  Arm(false);     
  if(millis() - stateChangeT0 > 10000) //sending init data for 10 seconds
  {
    stateChangeT0 = millis();
    state = 0; //it's time to arm
  }
  }
  else if (state == 0){
    compare = 100;
    //set arming signals
    Arm(true);
    if (millis() - stateChangeT0 > 10000){
      rampUpTime = millis();
      throttle = 900;
      state = 2; // 1 is intstant, 2 is slow incraments of 50
      stateChangeT0 = millis();
    }
  }
  else if (state == 1){
    //Roll Ch 0, Pitch Ch 1, Yaw Ch 3, Throttle Ch 2, Arm Ch 4
    if(millis() - stateChangeT0 < 3000)
    {
      roll = 1500;
      pitch = 1500;
      yaw = 1500;
      throttle = 1250;
      killswitch = 1000;
    }  
    else if(millis() - stateChangeT0 < 6000)
    {    
      roll = 1500;
      pitch = 1500;
      yaw = 1500;
      throttle = 1350;
      killswitch = 1000;
    }
    else if(millis() - stateChangeT0 < 9000){
      roll = 1500;
      pitch = 1500;
      yaw = 1500;
      throttle = 1250;        
      killswitch = 1000;
    }
    else{
      roll = 1500;
      pitch = 1500;
      yaw = 1500;
      throttle = 885;        
      killswitch = 1700;
    }
  } // end state 1
  if(state == 2)
  {
    if((millis() - rampUpTime >= 1000) && (killswitch < 1500) && (throttle < 1150))
    {
      throttle += 50;
      rampUpTime = millis();
    }
    if((throttle >= 1150) && (millis() - rampUpTime >= 2000))
    {
      killswitch = 1700;
      failsafe = 2000;
    }
  }//end state 2
  if(state == -1 || state == 0){ //Light Blinker
    if(millis()- blinkTime >= compare){
      LightSRLatch();
      blinkTime = millis();
    }
  }
  else if(killswitch == 1700){
    digitalWrite(LEDpin,LOW);
  }
  else{
    digitalWrite(LEDpin,HIGH);
  }
  delay(10);
  DataSetSend();
  PrintData();
}//end Loop

void PrintData()
{
  Serial.println("Output"); 
  for (uint8_t i = 0; i < data.NUM_CH; i++){ //print data we're sending.
    Serial.print(i);
    Serial.print(": ");
    if(data.ch[i] != 0)
    {
      Serial.println((.6251*(data.ch[i]))+879.7);
    }
    else
    {
      Serial.println(data.ch[i]);
    }
  } // close printing for
  Serial.println("-----");
  Serial.print("State is: ");
  Serial.println(state);
  Serial.println("Input");
  for(uint8_t i = 0; i < sizeof(ibus.channels); i++)
  {
    Serial.print(i);
    Serial.print(": ");
    if(data.ch[i] != 0)
    {
      Serial.println(ibus.channels[i]);
    }
    else
    {
      Serial.println(ibus.channels[i]);
    }
  }
}

void Arm(bool armmed)
{
  if(armmed)
  {
    arming = 1800;
  }
  else if(!armmed)
  {
    arming = 1000;
  }
}

double ChannelMath(int setpoint)
{
  return((setpoint - 879.7)/.6251);
}

void LightSRLatch()
{
  if(lightOn)
  {
    digitalWrite(LED_BUILTIN, LOW);
    lightOn = false;    
  }
  else if(!lightOn)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    lightOn = true;    
  }
}

void DataSetSend()
{
  data.ch[0] = (ChannelMath(roll));
  data.ch[1] = (ChannelMath(pitch));
  data.ch[2] = (ChannelMath(throttle));
  data.ch[3] = (ChannelMath(yaw));
  data.ch[4] = (ChannelMath(arming));
  data.ch[7] = (ChannelMath(failsafe)); //mutes beeper NO KONNER ITS FAILSAFE, BEEPERMUTE IS THE SAME AS ARM
  data.ch[10] = (ChannelMath(2000)); //Signal
  data.ch[12] = (ChannelMath(killswitch)); //Killswitch
  sbus_tx.data(data);
  sbus_tx.Write();
}