
#define MAX_POS 158
int current_pos=0,current_direct=HIGH;
int currentState[1],currentdirState;
void togglePin(byte pin, byte direction_pin) {

  //Switch directions if end has been reached
  if (current_pos >= 158) {
    currentdirState = HIGH;
    digitalWrite(direction_pin,HIGH);
    delay(5);
  } 
  else if (current_pos <= 0) {
    Serial.println("aaaa");
    currentdirState = LOW;
    digitalWrite(direction_pin,LOW);
    delay(5);
  }

  Serial.print(current_pos);
  Serial.print(" ");
  if(currentdirState==HIGH)
    {Serial.println("HIGH");}
    else
    {Serial.println("LOW");}

  //Update currentPosition
  if (currentdirState==HIGH){
    current_pos--;
  } 
  else {
    current_pos++;
  }

  //Pulse the control pin
  digitalWrite(pin,currentState[0]);
  currentState[0] = ~currentState[0];
}
void reset(byte pin)
{
  digitalWrite(pin+1,HIGH); // Go in reverse
  for (byte s=0;s<158;s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(pin,HIGH);
    digitalWrite(pin,LOW);
    delay(5);
  }
  current_pos = 0; // We're reset.
  digitalWrite(pin+1,LOW);
  current_direct = 0; // Ready to go forward.
}
void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(7,INPUT);
  reset(2);
  Serial.begin(9600);
}

bool last_pushed = false;
void loop() {
  // put your main code here, to run repeatedly:
  bool pushed;
  pushed=digitalRead(7);
  if (pushed != last_pushed)
  {
    delay(5);
    pushed = digitalRead(7);
  }
  if(pushed)
  {
    togglePin(2,3);
  }
  else
  {
     /*if(currentState==HIGH)
    {Serial.println("HIGH");}
    else
    {Serial.println("LOW");}
    //Serial.println("off");
    //digitalWrite(2,LOW);*/
  }
  last_pushed = pushed;
  
}

