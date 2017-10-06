/*
HORIZONTAL ADJUSTABLE SIZE BAR METERS, LINEAR or AUDIO LOGARITHMIC, WITH or WITHOUT PEAK MEMORY

Allows to print on LCD precise horizontal bar graphs (up to 40 simultaneosly...) passing: 
percentage from 0 to 100%, XY position of base of bar, maximum length (in character positions),
linear or logaritmic bar, peak memory option with separate decay times for each bar.
Please understand that when peak is in the same box of the tip of current bar,
it will not be shown, this is due to limitation to 8 custom characters, but you will not actually notice it :-)

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
double a, a2;
unsigned int segment, segment2;
double perc=100;
boolean logar;
byte bn;   // bn BAR NUMBER every bar have to be numbered from 1 to 40                     
unsigned int xpos, ypos, blen;
double pkval[41]; // to store the last peak value for each bar
int    pkcyc[41][2];    // set the num. of printbar recall TO DO [1] and DONE [2] for each bar before the peak decays, 
                        // if pkcyc[bn][1] == 0 no peaks wanted
                        // it's a workaround to avoid to waste time in delay functions (internal or external)
                        // that may interfere your application performances
                        
// Bar characters definitions (8 maximum allowed)
byte block[8][8]=  
{
  { 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10 },  // define characters for bar
  { 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18 },
  { 0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C },
  { 0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E },

  { 0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08 },  // define characters for peak
  { 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04 },
  { 0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02 },
  { 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01 },
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

void loop() 
{
          digitalWrite(potalim, HIGH);  // REMOVE IT, TEST purposes, activates 5V on pin 12 for the signal potentiometer
          delay(del);                   // REMOVE IT, to slow down display without a real application running
  
// Assign value (in this case from Analog Input 0)
  unsigned int value = analogRead(0);
  perc = value/1023.0*100;       // perc is the value from 0 to 100 for the length of the bargraph of the meter

// Setting parameters for bars ( you can put directly values inside the function)
// xpos=0; 
// ypos=1; 
// blen= 20;
// remember that bar starts from lower point raising of blen positions
// LEFT UPPER ANGLE IS xpos = 0 AND ypos = 0
logar = false; 
// bn = 1;   // BAR NUMBER every bar have to be numbered from 1 to 40

for (int i=1;i<41;i++)  // setting 15 cycles for all bars... YOUR CHOICE !!
    { pkcyc[i][1]=  15;}

//==============================================================
//           EXAMPLE OF PRINTING ON DISPLAY 
//   (I use same input for all bars for demo purposes)

// printbar (bn, perc, xpos, ypos, blen, logar);

lcd.setCursor(0,0); lcd.print ("COOL BARS!  R   L");
printbar (1,perc,17,0,3, true);
printbar (2,perc,13,0,3, true);

lcd.setCursor(0,1); lcd.print ("lin");
pkcyc[3][1]=  0;  // don't want peak for bar n. 3 
printbar (3,perc,3,1,17, false);

lcd.setCursor(0,2); lcd.print ("  R");
lcd.setCursor(0,3); lcd.print ("  L");
printbar (4,perc,3,2,17, true);
printbar (5,perc,3,3,17, true);

//          END OF EXAMPLE
//==============================================================
}

//  METER BARS PRINTING FUNCTION
// passing percentage, x and y position, positions bar length, linear/audio logaritmic bar
// barnum is used to manage times of decayn of each peak separately (see definitions)

void printbar (byte bn, double perc, int xpos, int ypos, int blen, boolean logar) 
{
  if ((logar == true) && (perc > 0))   // logaritmic bar
    {
    perc = ( log10(perc ) )*50;    // 10 * log10 (value) linear to logaritmic for AUDIO conversion       
    if ( perc < 0 ) { perc = 0; }  // avoid negative values 
    }

  a=blen/99.5*perc;      // calculate length of bar
  b = 0;

  if ( pkcyc[bn][1] > 0 )              // if PEAK is activated
  {
    if ( (a > (pkval[bn]-0.01)) || (pkcyc[bn][2] > pkcyc[bn][1]) )   // new peak (w little histeresys) or expiration of peak
    {
      pkval[bn] = a;    
      pkcyc[bn][2] = 0;    // reset cycles
    }

    pkcyc[bn][2]++;
        
   }

  // drawing filled rectangles  
  if (a>=1) {  
    
    for (int i=1;i<a;i++) {
      lcd.setCursor(xpos-1+i,ypos);
      lcd.write(255);  
      b=i;
      }
 
     a=a-b;
  }
  segment= a*5;

// drawing final part of the bar
if (b < blen)
{
  lcd.setCursor(xpos+b,ypos);
  
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
   }  
  }
  
// cleaning rest of line
  for (int i =0;i<(blen-b-1);i++) 
  {
    lcd.setCursor(xpos+ b+ 1+ i, ypos);
    lcd.print(" ");
  }

  b2= (int) pkval[bn];
  a2= pkval[bn]-b2;
  segment2= a2*5; 
  
  if ( (pkcyc[bn][1] > 0) && (    // DRAWING PEAK 
            ((b + segment) == 0)                            // if bar empty
         || (b2 > b)                                        // or different box position
         || ( (b2 == b) && segment == 0 && segment2 >0 )    // special case, too long to explain :-)
        ))
  {
  lcd.setCursor(xpos + b2, ypos);
 
  switch (segment2) {
  case 0:
   if ( (b2 > 0) || (b2 > b+1))
     { 
     lcd.setCursor(xpos + b2-1, ypos); 
     lcd.write(7);
     };  
    break;
  case 1:
    lcd.write(byte(0));
    break;
  case 2:
    lcd.write(4);
    break;
  case 3:
    lcd.write(5);
    break;
  case 4:
    lcd.write(6);
    break;
    }
}
}
