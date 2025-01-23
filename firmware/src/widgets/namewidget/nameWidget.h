#ifndef NAME_WIDGET_H
#define NAME_WIDGET_H

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <widget.h>

#include "utils.h"

class NameWidget : public Widget {
   public:
    NameWidget(ScreenManager &manager, String name);
    ~NameWidget() override;
    void setup() override;
    void update(bool force = false) override;
    void draw(bool force = false) override;
    void buttonPressed(uint8_t buttonId, ButtonState state) override;
    String getName() override;

    void changeMode();

   private:
    int m_lastUpdate = 0;
    int m_updateDelay = 1000;
    String s_name;
    int32_t m_defaultColor = TFT_WHITE;
    int32_t m_defaultBackground = TFT_BLACK;
};
#endif  // NAME_WIDGET_H