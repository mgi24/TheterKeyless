#include "WiFi.h"
#include <Preferences.h>
String SSIDkey[10];
int RSSItresh[10];
int keynumber = 0;
Preferences preferences;

void loadSSIDKeyList() {
  preferences.begin("wifi_keys", true); // read-only
  keynumber = preferences.getInt("keynumber", 0);
  Serial.printf("Number of saved keys: %d\n", keynumber);
  Serial.println("saved SSID list:");
  for (int i = 0; i < keynumber; ++i) {
    String loadssid = "ssid" + String(i);
    String loadrssi = "rssi" + String(i);
    SSIDkey[i] = preferences.getString(loadssid.c_str(), "");
    RSSItresh[i] = preferences.getInt(loadrssi.c_str(), -100);
    Serial.printf("%d. %s (RSSI Threshold: %d)\n", i + 1, SSIDkey[i].c_str(), RSSItresh[i]);
  }
  preferences.end();
}

void mainmenu(){
  Serial.println();
  Serial.println("Main Menu:");
  Serial.println("1. Register WIFI Key");
  Serial.println("2. Remove WIFI Key");
  Serial.println("3. Saved key List");
  Serial.println("4. Distance Settings");
  Serial.println();
}

int wifinum = 0;
String ssidlist[1000]; 

void scanwifi(){
  Serial.println("Scan start");
  int n = WiFi.scanNetworks();
  wifinum = n;
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      ssidlist[i] = WiFi.SSID(i);
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4ld", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2ld", WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            Serial.print("open"); break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
        default:                        Serial.print("unknown");
      }
      Serial.println();
      delay(10);
    }
  }
  Serial.println("");
  WiFi.scanDelete();
  delay(1000);
}

void runDetected(){
  digitalWrite(2, HIGH);
}
void runNotDetected(){
  digitalWrite(2, LOW);
}

void detectwifi(){
  int n = WiFi.scanNetworks();
  bool found = false;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < keynumber; ++j) {
      if (WiFi.SSID(i) == SSIDkey[j]) {
        found = true;
        Serial.printf("key (%s terdeteksi)\n", SSIDkey[j].c_str());
        WiFi.scanDelete();
        runDetected();
        delay(1000);
        break;
      }
    }

  }
  if (!found){
    Serial.println("NOT DETECT ANYTHING!");
    WiFi.scanDelete();
    runNotDetected();
      delay(1000);
  }
  
}



void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  loadSSIDKeyList();
  mainmenu();
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
}

int mode=0;

void loop() {
  if (Serial.available() > 0){
    String inputStr = Serial.readStringUntil('\n');
    int input = inputStr.toInt();
    if (mode == 0){
      if (input == 1) {
        mode = 1;
        Serial.println("Choose one of the wifi as KEY:");
        scanwifi();
        return;
      } else if (input == 2) {
        Serial.println("Enter the index of the key to remove:");
        for (int i = 0; i < keynumber; ++i) {
          Serial.printf("%d. %s (RSSI Threshold: %d)\n", i + 1, SSIDkey[i].c_str(), RSSItresh[i]);
        }
        mode = 2;
        return;
      } else if (input == 3) {
        loadSSIDKeyList();
        mainmenu();
        mode = 0;
        return;
      } else if (input == 4) {
        if (keynumber == 0) {
          Serial.println("Key empty, please register key first.");
          mode = 0;
          mainmenu();
        } else {
          Serial.println("Select SSID to change RSSI:");
          for (int i = 0; i < keynumber; ++i) {
            Serial.printf("%d. %s (Current RSSI Threshold: %d)\n", i + 1, SSIDkey[i].c_str(), RSSItresh[i]);
          }
          mode = 4;
        }
        return;
      } else {
        Serial.println("Invalid option, please try again.");
        return;
      }
    }
    else if (mode == 1){
      int selected = inputStr.toInt();
      if (selected > 0 && selected <= wifinum) {
        SSIDkey[keynumber] = ssidlist[selected - 1];
        RSSItresh[keynumber] = -80;
        Serial.printf("Selected Key: %s (RSSI Threshold: %d)\n", SSIDkey[keynumber].c_str(), RSSItresh[keynumber]);
        preferences.begin("wifi_keys", false); // write mode
        preferences.putString(("ssid" + String(keynumber)).c_str(), SSIDkey[keynumber]);
        preferences.putInt(("rssi" + String(keynumber)).c_str(), RSSItresh[keynumber]);
        keynumber++;
        preferences.putInt("keynumber", keynumber);
        preferences.end();
        Serial.println("Key saved!");
        mode = 0;
        mainmenu();
        return;
      } else {
        Serial.println("Invalid selection. Returning to main menu.");
        mode = 0;
        mainmenu();
        return;
      }
    }
    else if (mode == 2){
      if (input > 0 && input <= keynumber) {
        preferences.begin("wifi_keys", false); // write mode
        preferences.remove(("ssid" + String(input - 1)).c_str());
        preferences.remove(("rssi" + String(input - 1)).c_str());
        for (int i = input - 1; i < keynumber - 1; ++i) {
          SSIDkey[i] = SSIDkey[i + 1];
          RSSItresh[i] = RSSItresh[i + 1];
          preferences.putString(("ssid" + String(i)).c_str(), SSIDkey[i]);
          preferences.putInt(("rssi" + String(i)).c_str(), RSSItresh[i]);
        }
        keynumber--;
        preferences.putInt("keynumber", keynumber);
        preferences.end();
        Serial.println("Key removed!");
      } else {
        Serial.println("Invalid selection.");
      }
      mode = 0;
      mainmenu();
      return;
    }
    else if (mode == 4){
      int selected = inputStr.toInt();
      if (selected > 0 && selected <= keynumber) {
        Serial.println("Set new RSSI number:");
        while (Serial.available() == 0) {
          // wait for user input
          delay(10);
        }
        String rssiStr = Serial.readStringUntil('\n');
        int newRSSI = rssiStr.toInt();
        RSSItresh[selected - 1] = newRSSI;
        preferences.begin("wifi_keys", false);
        preferences.putInt(("rssi" + String(selected - 1)).c_str(), newRSSI);
        preferences.end();
        Serial.printf("RSSI threshold for %s set to %d\n", SSIDkey[selected - 1].c_str(), newRSSI);
      } else {
        Serial.println("Invalid selection. Returning to main menu.");
      }
      mode = 0;
      mainmenu();
      return;
    }
  }
  if (mode == 0){
    if (keynumber > 0){
      detectwifi();
    }
    
  }

}
