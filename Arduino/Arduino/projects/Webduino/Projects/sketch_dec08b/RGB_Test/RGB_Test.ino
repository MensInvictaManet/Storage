int p1 = 7;
int p2 = 6;
int p3 = 5;
int p4 = 4;

void setPins(int on, int off, int input1, int input2)
{
  pinMode(on, OUTPUT);
  digitalWrite(on, HIGH);
  
  pinMode(off, OUTPUT);
  digitalWrite(off, LOW);
  
  pinMode(input1, INPUT);
  pinMode(input2, INPUT);
  
  delay(200);
}

void setup()
{                
  pinMode(p1, INPUT);     
  pinMode(p2, INPUT);  
  pinMode(p3, INPUT);  
  pinMode(p4, INPUT);  
}

void loop()
{
/*
  pinMode(p1, OUTPUT);
  digitalWrite(p1, HIGH);
  
  pinMode(p4, OUTPUT);
  digitalWrite(p4, LOW);
  
  pinMode(p2, INPUT);
  pinMode(p3, INPUT);
  
  delay(200);
*/
  
  pinMode(p4, OUTPUT);
  digitalWrite(p4, HIGH);
  
  pinMode(p1, OUTPUT);
  digitalWrite(p1, LOW);
  
  pinMode(p2, INPUT);
  pinMode(p3, INPUT);
  
  delay(200);
}
