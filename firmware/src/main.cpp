#include "core/wifiWidget.h"
#include "screenManager.h"
#include "widgetSet.h"
#include "widgets/clockWidget.h"
#include "widgets/nameWidget.h"
#include "widgets/weatherWidget.h"
#include "widgets/webDataWidget.h"
#include <Arduino.h>
#include <Button.h>
#include <config.h>
#include <globalTime.h>
#include <widgets/stockWidget.h>

TFT_eSPI tft = TFT_eSPI();

// Button states
bool lastButtonOKState = HIGH;
bool lastButtonLeftState = HIGH;
bool lastButtonRightState = HIGH;

Button buttonOK(BUTTON_OK);
Button buttonLeft(BUTTON_LEFT);
Button buttonRight(BUTTON_RIGHT);

GlobalTime *globalTime; // Initialize the global time
time_t lastButtonTime = 0;
bool isScreensAsleep = false;

String connectingString{""};

WifiWidget *wifiWidget{nullptr};

int connectionTimer{0};
const int connectionTimeout{10000};
bool isConnected{true};

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
    if (y >= tft.height())
        return 0;
    tft.pushImage(x, y, w, h, bitmap);
    return 1;
}

ScreenManager *sm;
WidgetSet *widgetSet;

void setup() {

    buttonLeft.begin();
    buttonOK.begin();
    buttonRight.begin();

    Serial.begin(115200);
    Serial.println();
    Serial.println("Starting up...");

    sm = new ScreenManager(tft);
    sm->selectAllScreens();
    sm->getDisplay().fillScreen(TFT_WHITE);
    sm->reset();
    widgetSet = new WidgetSet(sm);

    TJpgDec.setSwapBytes(true); // jpeg rendering setup
    TJpgDec.setCallback(tft_output);

#ifdef GC9A01_DRIVER
    Serial.println("GC9A01 Driver");
#endif
#ifdef ILI9341_DRIVER
    Serial.println("ILI9341 Driver");
#endif
#if HARDWARE == WOKWI
    Serial.println("Wokwi Build");
#endif

    pinMode(BUSY_PIN, OUTPUT);
    Serial.println("Connecting to: " + String(WIFI_SSID));

    wifiWidget = new WifiWidget(*sm);
    wifiWidget->setup();

    globalTime = GlobalTime::getInstance();

    widgetSet->add(new ClockWidget(*sm));
    widgetSet->add(new StockWidget(*sm));
    widgetSet->add(new WeatherWidget(*sm));
    widgetSet->add(new NameWidget(*sm, OWNER_NAME));
#ifdef WEB_DATA_WIDGET_URL
    widgetSet->add(new WebDataWidget(*sm, WEB_DATA_WIDGET_URL));
#endif
#ifdef WEB_DATA_STOCK_WIDGET_URL
    widgetSet->add(new WebDataWidget(*sm, WEB_DATA_STOCK_WIDGET_URL));
#endif
}

void setScreenSleep(bool enable) {
    sm->selectAllScreens();
    if (enable) {
        sm->getDisplay().writecommand(0x28);
    } else {
        sm->getDisplay().writecommand(0x29);
    }
    isScreensAsleep = enable;
}

void loop() {
    if (wifiWidget->isConnected() == false) {
        wifiWidget->update();
        wifiWidget->draw();
        widgetSet->setClearScreensOnDrawCurrent(); // clear screen after wifiWidget
        delay(100);
    } else {
        if (!widgetSet->initialUpdateDone()) {
            widgetSet->initializeAllWidgetsData();
        }
        globalTime->updateTime();
        if (lastButtonTime == 0) {
            lastButtonTime = globalTime->getUnixEpoch();
        }

        if (buttonLeft.pressed()) {
            Serial.println("Left button pressed");
            if (!isScreensAsleep) {
                widgetSet->prev();
            } else {
                setScreenSleep(false);
            }
            lastButtonTime = globalTime->getUnixEpoch();
        }
        if (buttonOK.pressed()) {
            Serial.println("OK button pressed");
            if (!isScreensAsleep) {
                widgetSet->changeMode();
            } else {
                setScreenSleep(false);
            }
            lastButtonTime = globalTime->getUnixEpoch();
        }
        if (buttonRight.pressed()) {
            Serial.println("Right button pressed");
            if (!isScreensAsleep) {
                widgetSet->next();
            } else {
                setScreenSleep(false);
            }
            lastButtonTime = globalTime->getUnixEpoch();
        }

        if (globalTime->getUnixEpoch() - lastButtonTime > SCREEN_TIMEOUT && isScreensAsleep == false) {
            setScreenSleep(true);
        }

        widgetSet->updateCurrent();
        widgetSet->drawCurrent();
    }
}
