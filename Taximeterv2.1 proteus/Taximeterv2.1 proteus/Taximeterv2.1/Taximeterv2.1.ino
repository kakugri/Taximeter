#include<LiquidCrystal.h>
#include<TinyGPS.h>
#include<Keypad.h>
#include<GSM.h>

//Create GSM parameters

//Create GSPS parameters
TinyGPS gps;
//Create bufffers for each possible passenger
float flat [4];
float flon [4];
float flat1 [4];
float flon1 [4];
float bLat,bLon,bLat1,bLon1;
unsigned long int age,age1;
float distance [4] = {0,0,0,0};
float cost [4] = {0,0,0,0};
//Create Checker for each passenger
int checker [4] = {0,0,0,0};
bool onBoard [4] ={false,false,false,false};
bool stop = false;
char buff;
bool correct = false;
int numberOnBoard = 0;
float cost_per_km = 0.40595;

//InitializeLCD
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);

//Initialize Keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
};

byte rowPins[ROWS] = {9,8,7,6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {13, 12, 11}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char key;

volatile bool state = false;
volatile bool stop1 = false;
int stop_check = 0;

void setup(){
  //begin Serial
  Serial3.begin(9600);
  //Connect to server and get new rates
  lcd.begin(20,4);
  lcd.print("Connecting to site");
  //Do neccessary stuff
  delay(500);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Connection complete");
  delay(100);
  lcd.clear();
  //Interrupts n LED part
  pinMode(4,OUTPUT);
  digitalWrite(4,LOW);
  pinMode(22,OUTPUT);
  digitalWrite(22,LOW);
  //Setup Interrupts
  pinMode(3,INPUT);
  pinMode(18,INPUT);
  pinMode(5,OUTPUT);
  pinMode(16,OUTPUT);
  digitalWrite(5,HIGH);
  digitalWrite(16,HIGH);
  pinMode(5,OUTPUT);
  digitalWrite(5,HIGH);
  attachInterrupt(1,add,CHANGE);
  attachInterrupt(5,end_Session,CHANGE);
  //Prompt User
  lcd.print("Taximeter");
  lcd.setCursor(0,1);
  lcd.println("1.Start 2.Cost");
  }
  void loop(){
    interrupts();
    //get user's choice
    key = keypad.waitForKey();
    lcd.clear();
    
      switch(key){
    case '1':
      lcd.clear();
      lcd.print("Enter number of passenger");
      lcd.setCursor(0,1);
      lcd.print("After this prompt");
      while(!correct)
      {
      buff = keypad.waitForKey();
      switch(buff){
        case '1':
             numberOnBoard = 1;
             correct = true;
             break;
        case '2':
             numberOnBoard = 2;
             correct = true;
             break;
        case '3':
             numberOnBoard = 3;
             correct = true;
             break;           
        case '4':
             numberOnBoard = 4;
             correct = true;
             break;
        default:
            lcd.clear();
            lcd.print("Please enter correct number");
            delay(200);
      }
      }
      if(correct){
        lcd.clear();
      lcd.print("Session Starting");
      delay(100);
      start();
      }
      break;
    case '2':
    for(int i =0; i < 4; i++){
        lcd.clear();
        lcd.print(i+1);lcd.print(".Distance covered in ");
        lcd.setCursor(0,1);
        lcd.print(" km & cost in cedis");
        lcd.setCursor(0,2);
        lcd.print(distance[i]/1000); lcd.print(" km");
        lcd.setCursor(0,3);
        lcd.print(act_cost(distance[i])); lcd.print(" cedis");
        delay(1000);
      }
      break;
    
  }  
 }
 
void start(){
  //Create new variables
  bool newData = false;
  unsigned long chars;
  stop = false;
  for(int i = 0; i < numberOnBoard; i++){
  onBoard[i] = true;
  }
  while(!stop){
    lcd.clear();
    //Check serial every 0.5 seconds to see if there is new data
    for(unsigned long start = millis(); millis() -start < 500;)
    {
      while(Serial3.available())
      {
        char c = Serial3.read();
        if(gps.encode(c))
        newData = true;
      }
    }
    
    
    if(newData){
          gps.f_get_position(&bLat, &bLon, &age);   
          lcd.print("Lat = ");
          lcd.print(bLat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : bLat, 11);
          Serial3.print("\n");
          lcd.print("Lon = ");
          lcd.print(bLon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : bLon, 11);
          lcd.println("\n");
          delay(1000);
    }
    
    //Check each passenger to see if this is their first coordinate
    for(int i = 0; i < 4 ; i++){
      if(onBoard [i]== false){
        continue;
      }
      else{
      if(checker[i] = 0)
      {
        flat[i] = bLat;
        flon[i] = bLon;
        checker[i] = checker[i] + 1;
      }
      
      else{
        flat1[i] = bLat;
        flon1[i] = bLon;
        //Calculate Distance
        distance [i]=calc_dist(flat[i],flon[i],flat1[i],flon1[i])+ distance[i];
        flat[i] = flat1 [i];
        flon[i] = flon1 [i];
      }
    }
    }
   if(state){
   addPassenger();
   state = false;
   digitalWrite(4,LOW);
   }
   
   if(stop1){
     Stop();
     stop1=false;
     digitalWrite(22,LOW);
     
     if(numberOnBoard <= 0){
       stop = true;
       lcd.clear();
       lcd.print("1.Start 2.Cost");
     }
   }
}
}
void add(){
  if(digitalRead(3))
  state = true;
}

void addPassenger(){
  //When LED lights///user can choose option
  digitalWrite(4,HIGH);
  lcd.clear();
  key = keypad.waitForKey();
  state = false;
  switch(key){
     case '1':
           if(!onBoard[0]){
           onBoard[0] = true;
           distance[0] = 0;
           numberOnBoard++;
           lcd.setCursor(0,1);
           lcd.print("Passenger 1 added");
           delay(500);
           }
           else
           {
             lcd.setCursor(0,1);
             lcd.print("Passenger already in");
             delay(500);
           }
           break;
    case '2':
           if(!onBoard[1]){
           onBoard[1] = true;
           distance[1] = 0;
           numberOnBoard++;
           lcd.setCursor(0,1);
           lcd.print("Passenger 2 added");
           delay(500);
           }
           else
           {
             lcd.setCursor(0,1);
             lcd.print("Passenger already in");
             delay(500);
           }
           break;
    case '3':
           if(!onBoard[2]){
           onBoard[2] = true;
           distance[2] = 0;
           numberOnBoard++;
           lcd.setCursor(0,1);
           lcd.print("Passenger 3 added");
           delay(500);
           }
           else
           {
             lcd.setCursor(0,1);
             lcd.print("Passenger already in");
             delay(500);
           }
           break;
     case '4':
           if(!onBoard[3]){
           onBoard[3] = true;
           distance[3] = 0;
           numberOnBoard++;
           lcd.setCursor(0,1);
           lcd.print("Passenger 4 added");
           delay(500);
           }
           else
           {
             lcd.setCursor(0,1);
             lcd.print("Passenger already in");
             delay(500);
           }
           break;
  
}
}

//Function for distance calculation
float calc_dist(float flat1, float flon1, float flat2, float flon2){
float dist_calc=0;
float dist_calc2=0;
float diflat=0;
float diflon=0;

//Create buffers for holding radians of coordinates
float radian_flat1,radian_flat2;

//I've to spplit all the calculation in several steps. If i try to do it in a single line the arduino will explode.
diflat=radians(flat2-flat1);
radian_flat1=radians(flat1);
radian_flat2=radians(flat2);
diflon=radians((flon2)-(flon1));

dist_calc = (sin(diflat/2.0)*sin(diflat/2.0));
dist_calc2= cos(radian_flat1);
dist_calc2*=cos(radian_flat2);
dist_calc2*=sin(diflon/2.0);
dist_calc2*=sin(diflon/2.0);
dist_calc +=dist_calc2;

dist_calc=(2*atan2(sqrt(dist_calc),sqrt(1.0-dist_calc)));

dist_calc*=6371000.0; //Converting to meters
//Serial.println(dist_calc);
return dist_calc;
}

void Stop(){
  //When LED lights///user can choose option
  digitalWrite(22,HIGH);
  lcd.clear();
  key= keypad.waitForKey();
  switch(key){
         case '1':
               if(onBoard[0]){
               onBoard[0] = false;
               numberOnBoard--;
               lcd.print("Passenger1 removed");
               lcd.setCursor(0,1);
               lcd.print("Distance & cost");
               lcd.setCursor(0,2);
               lcd.print(distance[0]); lcd.print("km ");
               lcd.setCursor(0,3);
               cost[0]=act_cost(distance[0]);
               lcd.print(cost[0]); lcd.print("cedis ");
               distance[0] = 0;
               flat[0]= 0; flat1[0] = 0;
               flon[0] = 0; flon1 [0] = 0;
               checker [0] = 0;
               delay(500);
               }
               else
               {
                 lcd.setCursor(0,1);
                 lcd.print("Passenger1 already out");
                 delay(500);
               }
               break;
        case '2':
               if(onBoard[1]){
               onBoard[1] = false;
               numberOnBoard--;
               lcd.print("Passenger 2 removed");
               lcd.setCursor(0,1);
               lcd.print("Distance & cost");
               lcd.setCursor(0,2);
               lcd.print(distance[1]/1000); lcd.print("km ");
               lcd.setCursor(0,3);
               cost[1]=act_cost(distance[1]);
               lcd.print(cost[1]); lcd.print("cedis ");
               distance[1] = 0;
               flat[1]= 0; flat1[1] = 0;
               flon[1] = 0; flon1[1] = 0;
               checker [1] = 0;
               delay(500);
               }
               else
               {
                 lcd.setCursor(0,1);
                 lcd.print("Passenger2 already out");
                 delay(500);
               }
               break;
        case '3':
               if(onBoard[2]){
               onBoard[2] = false;
               numberOnBoard--;
               lcd.print("Passenger3 removed");
               lcd.setCursor(0,1);
               lcd.print("Distance & cost");
               lcd.setCursor(0,2);
               lcd.print(distance[2]/1000); lcd.print("km ");
               lcd.setCursor(0,3);
               cost[2]=act_cost(distance[2]);
               lcd.print(cost[2]); lcd.print("cedis ");
               distance[2] = 0;
               flat[2]= 0; flat1[2] = 0;
               flon[2] = 0; flon1 [2] = 0;
               checker [2] = 0;
               delay(500);
               }
               else
               {
                 lcd.setCursor(0,1);
                 lcd.print("Passenger3 already out");
                 delay(500);
               }
               break;
         case '4':
               if(onBoard[3]){
               onBoard[3] = false;
               numberOnBoard--;
               lcd.print("Passenger 4 removed");
               lcd.setCursor(0,1);
               lcd.print("Distance & cost");
               lcd.setCursor(0,2);
               lcd.print(distance[3]/1000); lcd.print("km ");
               cost[3]=act_cost(distance[3]);
               lcd.setCursor(0,3);
               lcd.print(cost[3]); lcd.print("cedis ");
               distance[3] = 0;
               flat[3]= 0; flat1[3] = 0;
               flon[3] = 0; flon1[3] = 0;
               checker[3] = 0;
               delay(500);
               }
               else
               {
                 lcd.setCursor(0,1);
                 lcd.print("Passenger4 already out");
                 delay(500);
               }
               break;
         case '#':
             for(int i = 0; i< 4; i++)
             {
               onBoard[i] = false;
               checker[i] = 0;
               flon[i] = 0; flon1[i] = 0;
               flat[i] = 0; flat1[i] = 0;
             }
             numberOnBoard = 0;
             lcd.print("All removed");
             delay(500);
             break;
         default:
           lcd.print("Option Invalid");
           delay(500);
           break;
     
} 
}

//Function for cost
float act_cost(float distance1)
{
 float km_cost = 0;
 float m_cost = 0;
 km_cost = cost_per_km*(distance1/1000);
 //m_cost = km_cost/1000;
 return km_cost;
}

void end_Session(){
  if(digitalRead(18))
    {
      stop1 = true;
    }
}




  
