#include <WiFiNINA.h>
#include <RTCZero.h>
#include <GxEPD2_BW.h>
#include <ArduinoLowPower.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <ArduinoJson.h>

RTCZero alarm;
WiFiSSLClient client;

const int CS_PIN = 10;
const int DC_PIN = 9;
const int RST_PIN = 8;
const int BUSY_PIN = 7;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

const int GMT = 3;

GxEPD2_BW<GxEPD2_266_BN, GxEPD2_266_BN::HEIGHT> display(GxEPD2_266_BN(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

void setup() {
  Serial.begin(9600);
  delay(2000);

  alarm.begin();
  WiFi.lowPowerMode();
}

void loop() {
  Serial.println("start loop");

  wifiConnect();
  setTime();

  const DynamicJsonDocument doc = apiGet();

  wifiEnd();

  writeScreen(doc);

  delay(1000);

  const int minutes = 60 - alarm.getMinutes();

  Serial.print("sleep ");
  Serial.print(minutes);
  Serial.println(" minutes");

  LowPower.deepSleep(minutes * 60000);
}

void wifiConnect() {
  while (status != WL_CONNECTED) {
    Serial.print("connecting to ");
    Serial.println(ssid);

    status = WiFi.begin(ssid, pass);
    delay(300);
  }

  Serial.print("ssid: ");
  Serial.println(WiFi.SSID());
  Serial.print("signal strength: ");
  Serial.println(WiFi.RSSI());
  Serial.print("ip address: ");
  Serial.println(WiFi.localIP());
}

void wifiEnd() {
  Serial.print("disconnect from ");
  Serial.println(WiFi.SSID());

  WiFi.end();

  status = WL_DISCONNECTED;
}

void setTime() {
  unsigned long epoch = 0;

  while (epoch == 0) {
    Serial.println("geting time");
    epoch = WiFi.getTime();

    delay(600);
  }

  unsigned long epochGMT = epoch + GMT * 60 * 60;
  alarm.setEpoch(epochGMT);

  Serial.print("set time to ");
  Serial.println(epochGMT);
}

DynamicJsonDocument apiGet() {
  Serial.println("connect to api.roots.ee");

  DynamicJsonDocument doc(4096);

  if (client.connect("api.roots.ee", 443)) {
    client.println("GET /elekter HTTP/1.1");
    client.println("Host: api.roots.ee");
    client.println("Connection: close");
    client.println();

    Serial.println("request GET /elekter sent");
  }

  while (!client.available()) {
    ; // wait for API response
  }

  while (client.available()) {
    String payload = client.readString();
    String body = payload.substring(payload.indexOf("[["), payload.indexOf("]]") + 2);

    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      Serial.println(error.c_str());
      return doc;
    }
  }


  if (!client.connected()) {
    Serial.println("disconnect from api.roots.ee");
    client.stop();
  }

  return doc;
}

void writeScreen(DynamicJsonDocument doc)  {
  Serial.println("update screen");

  String dateStr = getDateString();
  String timeStr = getTimeString();

  display.init();
  display.setRotation(2);
  display.setTextSize(1);
  display.setFullWindow();

  int16_t timePositionX, timePositionY;
  uint16_t timeWidth, timeHeight;
  display.setFont(&FreeSansBold9pt7b);
  display.getTextBounds(timeStr, 0, 0, &timePositionX, &timePositionY, &timeWidth, &timeHeight);
  uint16_t timeCursorX = display.width() - timeWidth - 3;
  uint16_t timeCursorY = timeHeight;

  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    display.setFont(&FreeSansBold9pt7b);
    display.setCursor(0, timeCursorY);
    display.print(dateStr);
    display.setCursor(timeCursorX, timeCursorY);
    display.print(timeStr);

    display.drawFastHLine(0, timeCursorY + 5, display.width(), GxEPD_BLACK);

    display.setFont(&FreeMono9pt7b);

    int16_t pricePositionX, pricePositionY;
    uint16_t priceWidth, priceHeight;

    uint16_t priceCursorY = timeHeight + 22;

    for (int i = 0; i <= 24; i++) {
      JsonArray row = doc[i];

      int hour = row[3];
      float price = row[4];
      int priceRounded = round(price / 10);

      String hourStr = getPriceString(hour, priceRounded);

      display.setCursor(0, priceCursorY);
      display.print(hourStr);

      priceCursorY = priceCursorY + 20;
    }
  } while (display.nextPage());

  display.hibernate();
}

String getDateString() {
  char result[5];
  sprintf_P(result, PSTR("%02d.%02d"), alarm.getDay(), alarm.getMonth());

  return result;
}

String getTimeString() {
  char result[5];
  sprintf_P(result, PSTR("%02d:%02d"), alarm.getHours(), alarm.getMinutes());

  return result;
}

String getPriceString(int hour, int price) {
  char result[14];
  sprintf_P(result, PSTR("%02d:00%.9d"), hour, price);

  return result;
}
