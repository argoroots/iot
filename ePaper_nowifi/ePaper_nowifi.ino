#include <RTCZero.h>
#include <GxEPD2_BW.h>
#include <ArduinoLowPower.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

RTCZero alarm;

const int CS_PIN = 10;
const int DC_PIN = 9;
const int RST_PIN = 8;
const int BUSY_PIN = 7;

String oldMessage = "Start!";

GxEPD2_BW<GxEPD2_266_BN, GxEPD2_266_BN::HEIGHT> display(GxEPD2_266_BN(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

void setup() {
  //Initialize serial and wait for port to open
  Serial.begin(9600);
  //while (!Serial);
  delay(2000);

  Serial.println("start setup");

  alarm.begin();

  writeScreen(oldMessage);

  Serial.println("end setup");
  Serial.println();
}

void loop() {
  Serial.println("start loop");

  String newMessage = getTimeString();

  if (oldMessage != newMessage) {
    Serial.println("update screen");

    writeScreen("");
    oldMessage = newMessage;
  }

  Serial.println("do nothing");

  Serial.println("start sleep");
  LowPower.deepSleep(60000);
  Serial.print("end ");
  Serial.println("60s sleep");
  Serial.println();
}

void writeScreen(String text)  {
  String dateStr = getDateString();
  String timeStr = getTimeString();

  display.init();
  display.setRotation(3);
  display.setTextSize(1);
  display.setFullWindow();

  int16_t timePositionX, timePositionY;
  uint16_t timeWidth, timeHeight;
  display.setFont(&FreeSansBold12pt7b);
  display.getTextBounds(timeStr, 0, 0, &timePositionX, &timePositionY, &timeWidth, &timeHeight);
  uint16_t timeCursorX = display.width() - timeWidth - 3;
  uint16_t timeCursorY = timeHeight;

  int16_t textPositionX, textPositionY;
  uint16_t textWidth, textHeight;
  display.setFont(&FreeSans9pt7b);
  display.getTextBounds(text, 0, 0, &textPositionX, &textPositionY, &textWidth, &textHeight);
  uint16_t textCursorX = 0;
  uint16_t textCursorY = (display.height() - textHeight) / 2 + timeHeight;

  if (display.height() - textHeight < timeHeight + 22) {
    textCursorY = timeHeight + 22;
  }

  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(0, timeCursorY);
    display.print(dateStr);
    display.setCursor(timeCursorX, timeCursorY);
    display.print(timeStr);

    display.drawFastHLine(0, timeCursorY + 5, display.width(), GxEPD_BLACK);

    display.setFont(&FreeSans9pt7b);
    display.setCursor(textCursorX, textCursorY);
    display.print(text);
  } while (display.nextPage());

  display.hibernate();
}

String getDateString() {
  char result[16];
  sprintf_P(result, PSTR("%02d.%02d.%4d"), alarm.getDay(), alarm.getMonth(), alarm.getYear() + 2000);

  return result;
}

String getTimeString() {
  char result[16];
  sprintf_P(result, PSTR("%02d:%02d"), alarm.getHours(), alarm.getMinutes());

  return result;
}
