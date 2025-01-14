/* an alternative approach.   swap the #if 1 / 0 values to try it
 * how to map for different rotations.
 */
#if 0

#include <Adafruit_GFX.h>
#include <Riscduino_MCUFRIEND_kbv.h>
Riscduino_MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define ORIENTATION 1   //change screen rotation

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 6, XM = A2, YP = A1, YM = 7; //ID=0x9341
const int TS_LEFT = 907, TS_RT = 136, TS_TOP = 942, TS_BOT = 139;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// forward declarations 
extern bool update_button(Adafruit_GFX_Button *b, bool down);
extern bool update_button_list(Adafruit_GFX_Button **pb);

Adafruit_GFX_Button on_btn, off_btn;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);      //because TFT control pins
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        switch (tft.getRotation() & 3) {
            // map raw ADC values to pixel coordinates
            // most apps only use a fixed rotation e.g omit unused rotations
            case 0:      //PORTRAIT
                pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
                pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
                break;
            case 1:      //LANDSCAPE
                pixel_x = map(p.y, TS_TOP, TS_BOT, 0, tft.width());
                pixel_y = map(p.x, TS_RT, TS_LEFT, 0, tft.height());
                break;
            case 2:      //PORTRAIT REV
                pixel_x = map(p.x, TS_RT, TS_LEFT, 0, tft.width());
                pixel_y = map(p.y, TS_BOT, TS_TOP, 0, tft.height());
                break;
            case 3:      //LANDSCAPE REV
                pixel_x = map(p.y, TS_BOT, TS_TOP, 0, tft.width());
                pixel_y = map(p.x, TS_LEFT, TS_RT, 0, tft.height());
                break;
        }
    }
    return pressed;
}

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void setup(void)
{
#if defined(__arm__) || defined(ESP32) //default to 12-bit ADC
    analogReadResolution(10); //Adafruit TouchScreen.h expects 10-bit
#endif
    Serial.begin(9600);
    uint16_t ID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(ID, HEX);
    Serial.println("Calibrate for your Touch Panel");
    if (ID == 0xD3D3) ID = 0x9486; // write-only shield
    tft.begin(ID);
    tft.setRotation(ORIENTATION);   // try different rotations
    tft.fillScreen(BLACK);
    on_btn.initButton(&tft,  60, 200, 100, 40, WHITE, CYAN, BLACK, "ON", 2);
    off_btn.initButton(&tft, 180, 200, 100, 40, WHITE, CYAN, BLACK, "OFF", 2);
    on_btn.drawButton(false);
    off_btn.drawButton(false);
    tft.fillRect(40, 80, 160, 80, RED);
}

/*  
 * updating multiple buttons from a list
 * 
 * anything more than two buttons gets repetitive
 * 
 * you can place button addresses in separate lists
 * e.g. for separate menu screens
 */

// Array of button addresses to behave like a list
Adafruit_GFX_Button *buttons[] = {&on_btn, &off_btn, NULL};

/* update the state of a button and redraw as reqd
 *
 * main program can use isPressed(), justPressed() etc
 */
bool update_button(Adafruit_GFX_Button *b, bool down)
{
    b->press(down && b->contains(pixel_x, pixel_y));
    if (b->justReleased())
        b->drawButton(false);
    if (b->justPressed())
        b->drawButton(true);
    return down;
}

/* most screens have different sets of buttons
 * life is easier if you process whole list in one go
 */
bool update_button_list(Adafruit_GFX_Button **pb)
{
    bool down = Touch_getXY();
    for (int i = 0 ; pb[i] != NULL; i++) {
        update_button(pb[i], down);
    }
    return down;
}

/* compare the simplicity of update_button_list()
 */
void loop(void)
{
    update_button_list(buttons);  //use helper function
    if (on_btn.justPressed()) {
        tft.fillRect(40, 80, 160, 80, GREEN);
    }
    if (off_btn.justPressed()) {
        tft.fillRect(40, 80, 160, 80, RED);
    }
}
#endif

