/////////////////////////////////////////////////////////////////

#include "Button2.h"; //  https://github.com/LennartHennigs/Button2
#include "ESPRotary.h";
//#include "HID-Project.h";
//#include "Keyboard.h";
#include "LiquidCrystal_I2C.h";
#include "AltSoftSerial.h";
# include "Enums.h";

/////////////////////////////////////////////////////////////////

#define COM_ROTARY_PIN1	9
#define COM_ROTARY_PIN2	10
#define COM_BUTTON_PIN	7
#define COM_XFER_BUTTON_PIN 12

#define NAV_ROTARY_PIN1	5
#define NAV_ROTARY_PIN2	6
#define NAV_BUTTON_PIN	3
#define NAV_XFER_BUTTON_PIN 11

#define CLICKS_PER_STEP 4


/////////////////////////////////////////////////////////////////

AltSoftSerial altSerial;

ESPRotary com_rotary = ESPRotary(COM_ROTARY_PIN1, COM_ROTARY_PIN2, CLICKS_PER_STEP);
Button2 com_rotaryButton = Button2(COM_BUTTON_PIN);
Button2 com_xferButton = Button2(COM_XFER_BUTTON_PIN);

ESPRotary nav_rotary = ESPRotary(NAV_ROTARY_PIN1, NAV_ROTARY_PIN2, CLICKS_PER_STEP);
Button2 nav_rotaryButton = Button2(NAV_BUTTON_PIN);
Button2 nav_xferButton = Button2(NAV_XFER_BUTTON_PIN);

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

static int comPreviousPosition = 0;
static int comMajorPostion = true;

static int navPreviousPosition = 0;
static int navMajorPostion = true;

char messageBuffer[80];

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  altSerial.begin(9600);
  
  com_rotary.setChangedHandler(comRotate);
  com_rotaryButton.setTapHandler(comMajorMinor);
  com_xferButton.setTapHandler(comXfer);

  nav_rotary.setChangedHandler(navRotate);
  nav_rotaryButton.setTapHandler(navMajorMinor);
  nav_xferButton.setTapHandler(navXfer);

  // init the lcd
  lcd.init();
  lcd.backlight();
}

void loop() {
  
  com_rotary.loop();
  com_rotaryButton.loop();
  com_xferButton.loop();
  
  nav_rotary.loop();
  nav_rotaryButton.loop();
  nav_xferButton.loop();
  
  if(Serial.available() > 0)
  {
    if (readline(Serial.read(), messageBuffer, 80) > 0) {
          altSerial.print("Msg: >");
          altSerial.print(messageBuffer);
          ProcessMessage(messageBuffer);
          altSerial.println("<");
    }
  }
}

/////////////////////////////////////////////////////////////////


void ProcessMessage(String rawMessage)
{
  // first 3 chars are the code
  MessageType code = rawMessage.substring(0,3).toInt();

  // rest is the data
  String data = rawMessage.substring(3);

  switch(code)
  {
    case COM1ACTV:
      DisplayComActiveFreq(PretifyFreq(data));
      break;      

    case COM1STBY:
      DisplayComStandbyFreq(PretifyFreq(data));
      break;

    case NAV1ACTV:
      DisplayNavActiveFreq(PretifyFreq(data));
      break;      

    case NAV1STBY:
      DisplayNavStandbyFreq(PretifyFreq(data));
      break; 

    default:
      altSerial.println("Unknown message recieved: " + rawMessage);
  }  
}


String PretifyFreq(String freq)
{
  // make the freq nice to display
  int diff = 7 - freq.length();
  for(int i = 0; i < diff; i++)
  {
    freq += "0";
  }
  
  // Fix the display to retain decimal point if minor is all zeros
  if (freq.charAt(3) != '.') {
    freq.setCharAt(3, '.');
  }

  return freq;
}

void DisplayComActiveFreq(String freq)
{
    lcd.setCursor(0,0);
    lcd.print(freq);
}

void DisplayComStandbyFreq(String freq)
{
    lcd.setCursor(9,0);
    lcd.print(freq);
}

void DisplayNavActiveFreq(String freq)
{
    lcd.setCursor(0,1);
    lcd.print(freq);
}

void DisplayNavStandbyFreq(String freq)
{
    lcd.setCursor(9,1);
    lcd.print(freq);
}

void SendMsg(ActivityType type, String data)
{
  String message = String(type) + data;

  altSerial.print("Sent: ");
  altSerial.println(message); // debugging
  Serial.print(message); // send to PC
  Serial.print(char(3));
}

void comRotate(ESPRotary& r) {
    
    int pos = r.getPosition();

    if(pos > comPreviousPosition)
    {
      altSerial.println("UP");

      if(comMajorPostion)
      {
        SendMsg(COM1_MAJOR_UP, "major up");  
      }
      else
      {
        SendMsg(COM1_MINOR_UP, "minor up");
      }
    }
    else
    {
      altSerial.println("DOWN");

      if(comMajorPostion)
      {
        SendMsg(COM1_MAJOR_DOWN, "major down");  
      }
      else
      {
        SendMsg(COM1_MINOR_DOWN, "minor down");
      }
    }

    comPreviousPosition = pos;
}
 
void comMajorMinor(Button2& btn) {

  comMajorPostion = !comMajorPostion;
  altSerial.println("Major/Minor flip!" + String(comMajorPostion));
}

void comXfer(Button2& btn) {
  SendMsg(COM1_SWAP, "swap");  
}

void navRotate(ESPRotary& r) {
    
    int pos = r.getPosition();

    if(pos > navPreviousPosition)
    {
      altSerial.println("UP");

      if(navMajorPostion)
      {
        SendMsg(NAV1_MAJOR_UP, "major up");  
      }
      else
      {
        SendMsg(NAV1_MINOR_UP, "minor up");
      }
    }
    else
    {
      altSerial.println("DOWN");

      if(comMajorPostion)
      {
        SendMsg(NAV1_MAJOR_DOWN, "major down");  
      }
      else
      {
        SendMsg(NAV1_MINOR_DOWN, "minor down");
      }
    }

    navPreviousPosition = pos;
}

void navMajorMinor(Button2& btn) {

  navMajorPostion = !navMajorPostion;
  altSerial.println("Major/Minor flip!" + String(navMajorPostion)); 
}

void navXfer(Button2& btn) {
  SendMsg(NAV1_SWAP, "swap");  
}

int readline(int readch, char *buffer, int len) {
    static int pos = 0;
    int rpos;

    if (readch > 0) {
        switch (readch) {
            case 3: // ETX - end of text
                rpos = pos;
                pos = 0;  // Reset position index ready for next time
                return rpos;
            default:
                if (pos < len-1) {
                    buffer[pos++] = readch;
                    buffer[pos] = 0;
                }
        }
    }
    return 0;
}




/////////////////////////////////////////////////////////////////
