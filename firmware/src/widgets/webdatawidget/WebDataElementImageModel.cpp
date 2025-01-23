#include "WebDataElementImageModel.h"

#include <HTTPClient.h>
#include <LittleFS.h>
#include <TJpg_Decoder.h>
#include <WiFi.h>

int32_t WebDataElementImageModel::getX() {
    return m_x;
}

void WebDataElementImageModel::setX(int32_t x) {
    if (m_x != x) {
        m_x = x;
        m_changed = true;
    }
}

int32_t WebDataElementImageModel::getY() {
    return m_y;
}

void WebDataElementImageModel::setY(int32_t y) {
    if (m_y != y) {
        m_y = y;
        m_changed = true;
    }
}

String WebDataElementImageModel::getImage() {
    return m_image;
}

void WebDataElementImageModel::setImage(String image) {
    if (m_image != image) {
        m_image = image;
        m_changed = true;
    }
}

void WebDataElementImageModel::setForce(bool value) {
    m_force = value;
}

void WebDataElementImageModel::setScale(uint8_t scale) {
    if (m_scale != scale) {
        m_scale = scale;
        m_changed = true;
    }
}

uint8_t WebDataElementImageModel::getScale() {
    return m_scale;
}

void WebDataElementImageModel::parseData(const JsonObject &doc, int32_t defaultColor, int32_t defaultBackground) {
    if (doc["x"].is<int32_t>()) {
        setX(doc["x"].as<int32_t>());
    }
    if (doc["y"].is<int32_t>()) {
        setY(doc["y"].as<int32_t>());
    }
    if (const char *image = doc["image"]) {
        setImage(image);
    }
    if (doc["forceReload"].is<bool>()) {
        setForce(doc["forceReload"].as<bool>());
    }
    if (doc["scale"].is<uint8_t>()) {
        setScale(doc["scale"].as<uint8_t>());
    }

    // if (const char* imageUrl = doc["imageUrl"]) {
    //     setImageUrl(imageUrl);
    // }
    // if (const char* imageBytes = doc["imageBytes"]) {
    //     setImageBytes(imageBytes);
    // }
}

bool getFile(String url, String filename, bool force);

void WebDataElementImageModel::draw(ScreenManager &manager) {
    // TODO implement displaying an image
    if (m_changed) {
        getFile(m_image, "/image.jpg", m_force);
    }

fs:
    File f = LittleFS.open("/image.jpg", "r");

    uint32_t jpgDataSize = f.size();
    uint8_t *jpgData = (uint8_t *) malloc(jpgDataSize);

    f.read((uint8_t *) jpgData, jpgDataSize);

    TJpgDec.setJpgScale(getScale());
    uint16_t w = 0, h = 0;
    TJpgDec.getJpgSize(&w, &h, jpgData, jpgDataSize);
    TJpgDec.drawJpg(m_x, m_y, jpgData, jpgDataSize);

    free(jpgData);
    //  display.drawImage(getImage(), getX(), getY());
}

// Fetch a file from the URL given and save it in LittleFS
// Return 1 if a web fetch was needed or 0 if file already exists
bool getFile(String url, String filename, bool force) {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return 0;
    } else {
        Serial.println("LittleFS Initialized");
    }

    // If it exists then no need to fetch it
    if (LittleFS.exists(filename) == true) {
        Serial.println("Found " + filename);
        if (force == true) {
            Serial.println("Forced reload by config.");
        } else {
            return 0;
        }
    }

    Serial.println("Downloading " + filename + " from " + url);

    // Check WiFi connection
    if ((WiFi.status() == WL_CONNECTED)) {
        Serial.print("[HTTP] begin...\n");

        HTTPClient http;
        // Configure server and url
        http.begin(url);

        Serial.print("[HTTP] GET...\n");
        // Start connection and send HTTP header
        int httpCode = http.GET();
        if (httpCode == 200) {
            fs::File f = LittleFS.open(filename, "w+");
            if (!f) {
                Serial.println("file open failed");
                return 0;
            }
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // File found at server
            if (httpCode == HTTP_CODE_OK) {
                // Get length of document (is -1 when Server sends no Content-Length header)
                int total = http.getSize();
                int len = total;

                // Create buffer for read
                uint8_t buff[128] = {0};

                // Get tcp stream
                WiFiClient *stream = http.getStreamPtr();

                // Read all data from server
                while (http.connected() && (len > 0 || len == -1)) {
                    // Get available data size
                    size_t size = stream->available();

                    if (size) {
                        // Read up to 128 bytes
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // Write it to file
                        f.write(buff, c);

                        // Calculate remaining bytes
                        if (len > 0) {
                            len -= c;
                        }
                    }
                    yield();
                }
                Serial.println();
                Serial.print("[HTTP] connection closed or file end.\n");
            }
            f.close();
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    return 1; // File was fetched from web
}
