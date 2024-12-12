
#include "widgets/NameWidget.h"

NameWidget::NameWidget(ScreenManager &manager, String name) : Widget(manager) {
    s_name = name;

    m_lastUpdate = 0;
}

NameWidget::~NameWidget() {
}

void NameWidget::setup() {
}

void NameWidget::changeMode() {
}

void NameWidget::draw(bool force) {
    TFT_eSPI &display = m_manager.getDisplay();

    for (int i = 0; i < 5; i++) {
        m_manager.selectScreen(i);
        display.setTextColor(TFT_GREEN);
        display.setTextSize(4);
        display.drawString(s_name, 120, 120, 2);
    }
}

void NameWidget::update(bool force) {
    return;
}
