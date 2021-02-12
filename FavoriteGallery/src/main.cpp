#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "LGFX_Config_TTGO_TMusic.hpp"
#include <WiFi.h>
#include <HTTPClient.h>

const char* keyword = "【キーワード】";
const char* base_url = "【サーバのURL】/search-image";
const char* wifi_ssid = "【WiFiアクセスポイントのSSID】";
const char* wifi_password = "【WiFiアクセスポイントのパスワード】";
#define UPDATE_INTERVAL   (60 * 20)
#define NUM_OF_SEARCH     20

static LGFX lcd;

#define BACKGROUND_BUFFER_SIZE  70000
unsigned long background_buffer_length;
unsigned char background_buffer[BACKGROUND_BUFFER_SIZE];

void wifi_connect(const char *ssid, const char *password);
long doHttpGet(String url, uint8_t *p_buffer, unsigned long *p_len);
String urlencode(String str);

void setup() {
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
  lcd.setColorDepth(24);
  Serial.begin(9600);
  
  wifi_connect(wifi_ssid, wifi_password);
}

void loop() {
  String url = base_url;
  url += "?keyword=";
  url += urlencode(keyword);
  url += "&num=" + String(NUM_OF_SEARCH);

  Serial.println(url);

  background_buffer_length = sizeof(background_buffer);
  long ret = doHttpGet(url, background_buffer, &background_buffer_length);
  if( ret != 0 ){
    Serial.println("doHttpGet Error");
    delay(1000);
    return;
  }

  lcd.drawJpg(background_buffer, background_buffer_length, 0, 0);

  delay(1000UL * UPDATE_INTERVAL);
}

void wifi_connect(const char *ssid, const char *password){
  Serial.println("");
  Serial.print("WiFi Connenting");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.print("Connected : ");
  Serial.println(WiFi.localIP());
}

long doHttpGet(String url, uint8_t *p_buffer, unsigned long *p_len){
  HTTPClient http;

  Serial.print("[HTTP] GET begin...\n");
  // configure traged server and url
  http.begin(url);

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  unsigned long index = 0;

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
        // get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        // get lenght of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        Serial.printf("[HTTP] Content-Length=%d\n", len);
        if( len != -1 && len > *p_len ){
          Serial.printf("[HTTP] buffer size over\n");
          http.end();
          return -1;
        }

        // read all data from server
        while(http.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = stream->available();

            if(size > 0) {
                // read up to 128 byte
                if( (index + size ) > *p_len){
                  Serial.printf("[HTTP] buffer size over\n");
                  http.end();
                  return -1;
                }
                int c = stream->readBytes(&p_buffer[index], size);

                index += c;
                if(len > 0) {
                    len -= c;
                }
            }
            delay(1);
        }
      }else{
        http.end();
        return -1;
      }
  } else {
    http.end();
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    return -1;
  }

  http.end();
  *p_len = index;

  return 0;
}

String urlencode(String str){
    String encodedString = "";
    char c;
    char code0;
    char code1;
//    char code2;
    for (int i = 0 ; i < str.length() ; i++){
      c = str.charAt(i);
      if (c == ' '){
        encodedString += '+';
      } else if (isalnum(c)){
        encodedString += c;
      } else{
        code1 = (c & 0xf) + '0';
        if ((c & 0xf) > 9){
            code1 = (c & 0xf) - 10 + 'A';
        }
        c = (c >> 4) & 0xf;
        code0 = c + '0';
        if (c > 9){
            code0 = c - 10 + 'A';
        }
//        code2 = '\0';
        encodedString += '%';
        encodedString += code0;
        encodedString += code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}