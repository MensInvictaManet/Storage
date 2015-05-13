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
  setPins(p1, p2, p3, p4);  //  10--
  setPins(p2, p1, p3, p4);  //  01--
  setPins(p1, p3, p2, p4);  //  1-0-
  setPins(p3, p1, p2, p4);  //  0-1-
  
  setPins(p1, p4, p2, p3);  //  1--0
  setPins(p4, p1, p2, p3);  //  0--1
  setPins(p2, p3, p1, p4);  //  -10-
  setPins(p3, p2, p1, p4);  //  -01-
  
  setPins(p2, p4, p1, p3);  //  -1-0
  setPins(p4, p2, p1, p3);  //  -0-1
  setPins(p3, p4, p1, p2);  //  --10
  setPins(p4, p3, p1, p2);  //  --01
}
