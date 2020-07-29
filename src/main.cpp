#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include <U8x8lib.h>
#include "main.h"
#include <SPI.h>

// # defines
#define ROTARY_ENCODER_A_PIN 25// CLK (A pin) - to any microcontroler intput pin with interrupt -> in this example pin 32
#define ROTARY_ENCODER_B_PIN 26// DT (B pin) - to any microcontroler intput pin with interrupt -> in this example pin 21
#define ROTARY_ENCODER_BUTTON_PIN 27// SW (button pin) - to any microcontroler intput pin -> in this example pin 25
#define ROTARY_ENCODER_VCC_PIN -1 // VCC - to microcontroler VCC /*put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */
#define ROTARY_ENCODER_STEPS 4 //Number of encoder steps per detent
#define U8X8_HAVE_HW_SPI

// OBJECTS
U8X8_SSD1327_WS_128X128_4W_SW_SPI u8x8(/* clock=*/18, /* data=*/23, /* cs=*/5, /* dc=*/17, /* reset=*/16);
//U8X8_SSD1327_WS_128X128_4W_HW_SPI u8x8(/* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16);
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);



// Global Variables
int16_t encoderDelta;
int16_t encoderValue;
int test_limits = 2;
int Volume = -29;
int Bass = 0;
int Treble = 0;
int Source = 0;
int Screen_Selector = 0;
bool First_Boot = true;
int Old_Value = 100;
int Old_Screen_Selector = 0;
const String Screen_message[] = {"Volume","Bass","Treble","Source"};
String Output;

/*
PROTOTYPES
*/
void Display_update(int Value);

void rotary_onButtonClick() 
{
	u8x8.clear();
	if (Screen_Selector < 3) {Screen_Selector++; return;}
	if (Screen_Selector == 3) {Screen_Selector = 0; return;}
	
	// else{Screen_Selector = 0;}
}

void rotary_loop() {
	//first lets handle rotary encoder button click
	if (rotaryEncoder.currentButtonState() == BUT_RELEASED) {
		//we can process it here or call separate function like:
		rotary_onButtonClick();
	}

	//lets see if anything changed
	encoderDelta = rotaryEncoder.encoderChanged();
	
	//optionally we can ignore whenever there is no change
	if (encoderDelta == 0) return;
	
	//for some cases we only want to know if value is increased or decreased (typically for menu items)
	if (encoderDelta>0) Serial.print("+");
	if (encoderDelta<0) Serial.print("-");

	//for other cases we want to know what is current value. Additionally often we only want if something changed
	//example: when using rotary encoder to set termostat temperature, or sound volume etc
	
	//if value is changed compared to our last read
	if (encoderDelta!=0) {
		//now we need current value
		encoderValue = rotaryEncoder.readEncoder();
		//process new value. Here is simple output.
		Serial.print("Value: ");
		Serial.println(encoderValue);
	} 
	
}

void setup() {

	Serial.begin(115200);
  u8x8.begin();
  u8x8.clear();
	//we must initialize rorary encoder 
	rotaryEncoder.begin();
	rotaryEncoder.setup([]{rotaryEncoder.readEncoder_ISR();});
	//optionally we can set boundaries and if values should cycle or not
	rotaryEncoder.setBoundaries(-99, 0, false); //minValue, maxValue, cycle values (when max go to min and vice versa)
  rotaryEncoder.enable();
}

void loop()
 {
	//in loop call your custom function which will process rotary encoder values
	rotary_loop();
	delay(50);															 
	if (First_Boot == true) 
 	{	
		if (Volume > -30) {Volume = -70;} //Prevent Ear Blast if last volume was too high
		rotaryEncoder.encoder0Pos = Volume * 4;
		encoderValue = Volume;
		
    	Display_update(Volume);
		First_Boot = false;

 	}
	Volume = encoderValue;
	Display_update(Volume);
	// switch (Screen_Selector)
	// {
	// case 1: // Bass
	// 	rotaryEncoder.setBoundaries(-9, 9, false);
	// 	Bass = Bass + encoderDelta;
	// 	break;
	
	// case 2: // Treble
	// 	rotaryEncoder.setBoundaries(-9, 9, false);
	// 	Treble = Treble + encoderValue;
	// 	break;
	
	// case 3: // Source
	// 	rotaryEncoder.setBoundaries(0, 4, false);
	// 	Source = Source + encoderValue;
	// 	break;

	// default: //Volume
	// 	rotaryEncoder.setBoundaries(-99, 0, false);
	// 	break;
	// }
}

void Display_update(int Value)
{
 if (Value != Old_Value || Screen_Selector != Old_Screen_Selector)
 {
  u8x8.setFont(u8x8_font_inb21_2x4_f);
  char *Print_the_name = (char *)Screen_message[Screen_Selector].c_str();
  u8x8.drawString(0, 0, (const char *)Print_the_name);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  u8x8.drawString(0, 4, "________________");
  
  u8x8.setFont(u8x8_font_inb33_3x6_f);
  
  if (Value > -10){Output = (String(" ") + Value + ("dB"));}
  if (Value < -9) {Output = (String(Value) + ("dB"));}
  if (Value == 0) {Output = (" MAX");u8x8.clearLine(8);u8x8.clearLine(9);u8x8.clearLine(10);u8x8.clearLine(11);u8x8.clearLine(12);u8x8.clearLine(13);}
  char *Print_the_value = (char *)Output.c_str();
  u8x8.drawString(0, 8, (const char *)Print_the_value);
  Serial.println(Print_the_value);
  }
  Old_Value = Value;
  Old_Screen_Selector = Screen_Selector;
}