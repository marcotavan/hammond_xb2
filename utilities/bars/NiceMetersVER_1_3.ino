/*
VERTICAL ADJUSTABLE SIZE BAR METERS, LINEAR or AUDIO LOGARITHMIC (+ PEAK MEMORY BAR)
Allows to print on LCD precise horizontal bar graphs (up to 40 simultaneosly...) passing: 
percentage from 0 to 100%, XY position of base of bar, maximum length (in character positions),
linear or logaritmic bar.
Since the number of custom characters isn't enough, the peak memory can be used as a side bar showing just peaks.

One good thing I tried to do is to avoid to use delay functions, even to set a "time" for the peak expiration,
So instead of milliseconds I simply used a n. of cycles, counted every time a bar is printed (identified by a number),
it works very well and doesn't interfere with your application performances using delays.
Just adjust the n. of cycles to have the desired peak expiration time.

Hardware: 
- Arduino UNO or compatible
- LCD text display compatible Hitachi HD44780 driver, any colour and dimensions (i.e. 16x2, 20x4...)
  NO NEED OF SERIAL INTERFACE
- one 10K (or somewhat higher) trimmer if you want LCD Contrast Adjustment

============================================================
The circuit:

LCD DISPLAY     ARDUINO
 1  VSS          +5V
 2  VDD          GND
 3  V0           to the wiper of 10K potentiometer, its ends one to +5V, the other to GND
 4  RS           digital pin 7
 5  R/W          GND
 6  Enable       digital pin 6
 11 D4           digital pin 5
 12 D5           digital pin 4
 13 D6           digital pin 3
 14 D7           digital pin 2
 15 A +backlit   +5V
 16 K -backlit   GND
 
   FOR TESTING PURPOSES ONLY:

POTENTIOMETER (10K or higher)
 wiper           analog in A0
 one end         digital pin 12
 the other end   GND
============================================================

*******************************************************

Written by Roberto Baldassar 8 april 2014 v 1.2
- now it shows peaks really in all possible cases
- added logarithmic audio option

UPDATE 13 april 2014 v 1.2
- better description of circuit and code for testing

UPDATE 15 april 2014 v 1.3
- since there are no pratical limitations on no. of bars I decided to 
  separate normal bar and peak bar, so you can decide to print main or peak 
  bars alone (sometimes useful to visually compare values). Better Flexibility
  
********************************************************

*/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd( 7,  // RS
                   6,  // E
                   5,  // D4
                   4,  // D5
                   3,  // D6
                   2   // D7
                   );

#define lcdlength 20  // DEFINES YOUR display length in columns
#define lcdheigth 4   // DEFINES YOUR display heigth in rows

unsigned char b, b2;
double a;
unsigned int segment;
double perc=100;
boolean logar;
byte bn;       // bn BAR NUMBER every bar have to be numbered from 1 to 40                     
unsigned int xpos, ypos, blen;
double pkval[41]; // to store the last peak value for each bar
int    pkcyc[41][2];    // if pkcyc[bn][1];== 0 STANDARD BAR WITHOUT PEAK
                        // if    "     "    > 0 sets the the num. of printbar recall TO DO [bn][1] and DONE [bn][2] 
                        //                                for each bar before the peak decays 
                       
// Bar characters definitions (8 maximum allowed)
byte block[8][8]=  
{
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F },  // define characters for bar
  { 0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x1F },
  { 0x00,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F },
  { 0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F },
  { 0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F },  
  { 0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F },
  { 0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F },
  { 0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F },
};

#define del 100                         // REMOVE IT, delay for testing purposes only
int potalim = 12;                       // REMOVE IT, to supply 5V al to testing potentiometer  

//==============
void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(lcdlength, lcdheigth);
  
           pinMode(potalim, OUTPUT);    // REMOVE IT, test purposes, defines pin 12 to power signal potentiometer

  for( int i=0 ; i<8 ; i++ )
    lcd.createChar( i,block[i] );
}

//================
void loop() 
{
          digitalWrite(potalim, HIGH);  // REMOVE IT, TEST purposes, activates 5V on pin 12 for the signal potentiometer
          delay(del);                   // REMOVE IT, to slow down display without a real application running

// Assign value (in this case from Analog Input 0)
  unsigned int value = analogRead(0);
  perc = value/1023.0*100;       // perc is the value from 0 to 100 for the length of the bargraph of the meter

// Setting parameters for bars ( you can put directly values inside the function)
// xpos= 15; 
// ypos=  3; 
// blen=  4;
// remember that bar starts from lower point raising of blen positions
// LEFT UPPER ANGLE IS xpos = 0 AND ypos = 0
logar = false; 
// bn = 1;   // BAR NUMBER every bar have to be numbered from 1 to 40

for (int i=1;i<41;i++)  // setting 15 cycles for all bars... YOUR CHOICE !!
    { pkcyc[i][1]=  10;}

//==============================================================
//           EXAMPLE OF PRINTING ON DISPLAY 
//   (I use same input for all bars for demo purposes)

// printbar (bn, perc, xpos, ypos, blen, logar);

    for (int i=1;i<6;i++)    // print a series of short bars
    { 
      pkcyc[i][1]=  10;  // setting 10 cycles for peak bars
      printbar (i, perc, i*2-2, 1, 2, true);
      pkcyc[i+4][1]=  10;  // setting 10 cycles for peak bars
      printbar (i+4, perc, i*2-2, 3, 2, true);

      pkcyc[i+8][1]=  0;  // set OFF PEAKS (standard bars)
      printbar (i+8, perc, i*2-1, 1, 2, true);
      pkcyc[i+12][1]=  0;  // set OFF PEAKS (standard bars)
      printbar (i+12, perc, i*2-1, 3, 2, true);
    }

lcd.setCursor(11,1); lcd.print ("L");
lcd.setCursor(11,2); lcd.print ("i");
lcd.setCursor(11,3); lcd.print ("n");
pkcyc[21][1]=  2;  // want a peak bar set to 2 cycles delay (fast)
printbar (21, perc, 12, 3, 4, false);

lcd.setCursor(14,3); lcd.print ("R");
lcd.setCursor(19,3); lcd.print ("L");
pkcyc[22][1]=  0; 
pkcyc[23][1]=  0; 
printbar (22, perc, 15, 3, 4, true);
printbar (23, perc, 18, 3, 4, true);
pkcyc[24][1]=  10; 
pkcyc[25][1]=  10; 
printbar (24, perc, 16, 3, 4, true);
printbar (25, perc, 17, 3, 4, true);

//          END OF EXAMPLE
//==============================================================
}

//  METER BARS PRINTING FUNCTION
// passing percentage, x and y position, positions bar length, linear/audio logaritmic bar
// barnum is used to manage STANDARD/PEAK bar and decayn of each peak separately (see definitions)

void printbar (byte bn, double perc, int xpos, int ypos, int blen, boolean logar) 
{
  if ((logar == true) && (perc > 0))   // logaritmic bar
    {
    perc = ( log10(perc ) )*50;    // 10 * log10 (value) linear to logaritmic for AUDIO conversion       
    if ( perc < 0 ) { perc = 0; }  // avoid negative values 
    }

  double a=blen/99.5*perc;      // calculate length of bar
  b = 0;

  if ( pkcyc[bn][1] > 0 )              // if PEAK is activated
  {
    if ( a > (pkval[bn]-0.1) | pkcyc[bn][2] > pkcyc[bn][1] )   // new peak (w little histeresys) or expiration of peak
    {
      pkval[bn] = a;    
      pkcyc[bn][2] = 0;    // reset cycles
    }

    pkcyc[bn][2]++;   
   }

// drawing main bar
 if (pkcyc[bn][1] == 0) 
 { drawrest (xpos, ypos, b, a, blen); }
 else
 
// drawing peak bar 
 { 
  b2= (int) pkval[bn];
  a= pkval[bn];
  drawrest (xpos, ypos, b2, a, blen);
 }
}

// trace bar and cleanes (also for peak)
void drawrest (int xpos, int ypos, unsigned char b, double a, int blen) 
{ 
  // drawing filled rectangles  
  if (a>=1) {  
    
    for (int i=1;i<a;i++) {
      lcd.setCursor(xpos,ypos-i+1);
      lcd.write(255);  
      b=i;
      }
     a=a-b;
  }
  
 segment= a*8;  
 if (b < blen) 
 {
  lcd.setCursor(xpos,ypos-b);
  
  switch (segment) {
  case 0:
    lcd.print(" ");
    break;
  case 1:
    lcd.write((byte)0);
    break;
  case 2:
    lcd.write(1);
    break;
  case 3:
    lcd.write(2);
    break;
  case 4:
    lcd.write(3);
    break;
  case 5:
    lcd.write(4);
    break;
  case 6:
    lcd.write(5);
    break;
  case 7:
    lcd.write(6);
    break;
   }  
  }
  
// cleaning rest of line
  for (int i =0;i<(blen-b-1);i++) 
  {
    lcd.setCursor(xpos, ypos-b-i-1);
    lcd.print(" ");
}
}


