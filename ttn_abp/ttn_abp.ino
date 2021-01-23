#include <TTN_esp32.h>
#include "heltec.h"
#include "TTN_CayenneLPP.h"

/*************************************************************
 * Arquivo em Arduino/libraries/TTN_esp32/project_config/lmic_project_config.h com as definições abaixo:
    #define CFG_au915 1
    #define CFG_sx1276_radio 1
    #define DISABLE_PING

   Foi necessário comentar linhas no arquivo Arduino/libraries/TTN_esp32/src/lmic/lmic/au921.c para poder compilar o código
   As alterações no aruivo au921.c foram as seguinte:
   - Comentar a variável _DR2RPS_CRC (linha 38 a 57)
   - Comentar as funções setupChannel, disableChannel, enableChannel, enableSubBand, disableSubBand, selectSubBand (linha 146 a 231)

   A varíavel e as funções tiveram que ser comentadas pois geravam duplicidades com variável e funções de mesmo nome presentes
   no arquivo au915.c
 */

/***************************************************************************
 *  Go to your TTN console register a device then the copy fields
 ****************************************************************************/
// LoRaWAN NwkSKey, network session key msb format
static const PROGMEM u1_t NWKSKEY[16] = { 0xEB, 0x74, 0x9D, 0x8F, 0xFD, 0x21, 0x31, 0x52, 0x9E, 0xE2, 0x6C, 0x79, 0xD7, 0xCD, 0xFF, 0x1E };

// LoRaWAN AppSKey, application session key msb format
static const u1_t PROGMEM APPSKEY[16] = { 0xB7, 0x00, 0x2C, 0x6C, 0x18, 0xBF, 0xF5, 0x69, 0x81, 0x79, 0xE2, 0x55, 0xEE, 0x66, 0xDA, 0x44 };
static const u4_t DEVADDR = 0x26031430; 

TTN_esp32 ttn ;
TTN_CayenneLPP lpp;

void message(const uint8_t* payload, size_t size, int rssi)
{
    Serial.println("-- MESSAGE");
    Serial.print("Received " + String(size) + " bytes RSSI= " + String(rssi) + "dB");

    for (int i = 0; i < size; i++)
    {
        Serial.print(" " + String(payload[i]));
        // Serial.write(payload[i]);
    }

    Serial.println();
}

void OLEDDisplay() {
   Heltec.display->clear();
   Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
   Heltec.display->drawString(0,0, "LORAWAN_STS - SHC");
   Heltec.display->drawString(0,10, "Frequencia: 916.8MHz");
   Heltec.display->drawString(0,20, "by PU2SDM e Ferauche");
   char devAddr[16];
   sprintf(devAddr, "%08X", DEVADDR);
    
   Heltec.display->drawString(0,30, devAddr);
   Heltec.display->drawString(0,40, "Iniciado!");
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting");

    //Heltec sendo utilizada somente para o display, o controle do Lora é feito pelo objeto ttn
    Heltec.begin(true /*displayEnable*/, false/*Lora Enable*/, false /*Serial Enable*/);
    
    ttn.begin();
    ttn.onMessage(message); // declare callback function when is downlink from server
    ttn.personalize("0", "0", "0"); //as chaves são iniciadas com 0, pois sao substituídas no código das linhas seguintes para serem copiadas na memória do ESP

    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x13, DEVADDR, nwkskey, appskey);
    
    for (int c = 0; c < 72; c++){
      LMIC_disableChannel(c);
    }
    
    // We'll only enable Channel 8 (916.8Mhz) since we're transmitting on a single-channel
    LMIC_enableChannel(8);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink
    LMIC_setDrTxpow(DR_SF7,14);
    
    //Heltec.display->init();
  
    Heltec.display->display();
    ttn.showStatus();
}

char txt[20];

void loop()
{      
    static float nb = 18.2;
    nb += 0.1;
    lpp.reset();
    lpp.addTemperature(1, nb);
    if (ttn.sendBytes(lpp.getBuffer(), lpp.getSize()))
    {
        Serial.printf("Temp: %f TTN_CayenneLPP: %d %x %02X%02X\n", nb, lpp.getBuffer()[0], lpp.getBuffer()[1],
            lpp.getBuffer()[2], lpp.getBuffer()[3]);
        sprintf(txt, "Enviando %.2f", nb);
        OLEDDisplay();
        Heltec.display->drawString(0,50, txt);
        Heltec.display->display();
    }
    delay(10000);
}
