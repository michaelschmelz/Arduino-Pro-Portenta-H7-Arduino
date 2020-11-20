#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "wiced_resource.h"
#include "certificates.h"

const bool DEBUG = true;
const bool REFORMAT = false;

QSPIFBlockDevice root(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
mbed::MBRBlockDevice wifi_data(&root, 1);
mbed::FATFileSystem wifi_data_fs("wlan");
mbed::FATFileSystem other_data_fs("fs");

long getFileSize(FILE *fp) {
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  return size;
}

void setup() {

  Serial.begin(115200);
  while (!Serial);

  Serial.println(String(DEBUG ? "Here 1" : ""));
  mbed::MBRBlockDevice::partition(&root, 1, 0x0B, 0, 1024 * 1024);
  // use space from 15.5MB to 16 MB for another fw, memory mapped

  Serial.println(String(DEBUG ? "Here 2" : ""));
  int err =  wifi_data_fs.mount(&wifi_data);
  if (err || REFORMAT) {
    // Reformat if we can't mount the filesystem
    // this should only happen on the first boot
    Serial.println(String(DEBUG ? "Here 2.1" : ""));
    Serial.println("No filesystem found, formatting...");
    Serial.println(String(DEBUG ? "Here 2.2" : ""));
    err = wifi_data_fs.reformat(&wifi_data);
  }
  Serial.println(String(DEBUG ? "Here 3" : ""));
  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir("/wlan")) != NULL) {
    Serial.println(String(DEBUG ? "Here 4" : ""));
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      Serial.println(String(DEBUG ? "Here 4.1" : ""));
      Serial.println(ent->d_name);
      Serial.println(String(DEBUG ? "Here 4.2" : ""));
      String fullname = "/wlan/" + String(ent->d_name);
      if (fullname == "/wlan/4343WA1.BIN") {
        Serial.println(String(DEBUG ? "Here 4.3" : ""));
        Serial.println("Firmware found! Force update? [Y/n]");

        /////////////////////////////
        // NEW
        bool confirmation = false;
        while (confirmation == false) {
          if (Serial.available()) {
            char choice = Serial.read();
            switch (choice) {
              case 'y':
              case 'Y':
                wifi_data_fs.reformat(&wifi_data);
                confirmation = true;
                break;
              case 'n':
              case 'N':
                confirmation = true;
                break;
              default:
                continue;
            }
          }
        }
        /////////////////////////////

        /////////////////////////////
        // OLD
//        while (1) {
//          Serial.println(String(DEBUG ? "Here 4.4" : ""));
//          delay(1000);
//          if (Serial.available()) {
//            Serial.println(String(DEBUG ? "Here 4.5" : ""));
//            int c = Serial.read();
//            if (c == 'Y' || c == 'y') {
//              Serial.println(String(DEBUG ? "Here 4.6" : ""));
//              wifi_data_fs.reformat(&wifi_data);
//              break;
//            }
//            if (c == 'N' || c == 'n') {
//              Serial.println(String(DEBUG ? "Here 4.7" : ""));
//              return;
//            }
//          }
//        }
        /////////////////////////////


        
      }
    }
    closedir (dir);
  }

  Serial.println(String(DEBUG ? "Here 5.0" : ""));
  extern const unsigned char wifi_firmware_image_data[];
  Serial.println(String(DEBUG ? "Here 5.1" : ""));
  extern const resource_hnd_t wifi_firmware_image;
  Serial.println(String(DEBUG ? "Here 5.2" : ""));
  FILE* fp = fopen("/wlan/4343WA1.BIN", "wb");
  Serial.println(String(DEBUG ? "Here 5.3" : ""));
  //  int ret = fwrite(wifi_firmware_image_data, 421098, 1, fp);  // BROKEN
  int ret = 0;
  Serial.println(String(DEBUG ? "Here 5.4" : ""));
  fclose(fp);
  Serial.println(String(DEBUG ? "Here 5.5" : ""));

  root.program(wifi_firmware_image_data, 15 * 1024 * 1024 + 1024 * 512, 421098);

  fp = fopen("/wlan/cacert.pem", "wb");
  ret = fwrite(cacert_pem, cacert_pem_len, 1, fp);
  fclose(fp);

  fp = fopen("/wlan/cacert.pem", "rb");
  char buffer[128];
  ret = fread(buffer, 1, 128, fp);
  Serial.write(buffer, ret);
  while (ret == 128) {
    ret = fread(buffer, 1, 128, fp);
    Serial.write(buffer, ret);
  }
  fclose(fp);

  Serial.println("Firmware and certificates updated!");
}

void loop() {

}
