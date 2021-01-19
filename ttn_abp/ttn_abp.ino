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
 *  and replace the CHANGE_ME strings below
 ****************************************************************************/
const char* devAddr = "0"; // Change to TTN Device Address
const char* nwkSKey = "0"; // Change to TTN Network Session Key
const char* appSKey = "0"; // Change to TTN Application Session Key

// LoRaWAN NwkSKey, network session key
static const PROGMEM u1_t NWKSKEY[16] = { /*em formato de vetores em hexa */};

// LoRaWAN AppSKey, application session key
static const u1_t PROGMEM APPSKEY[16] = { /*em formato de vetores em hexa */ };
static const u4_t DEVADDR = 0x00000; /*preencher com o endereço do device em Hexa */

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

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting");

    //Heltec sendo utilizada somente para o display, o controle do Lora é feito pelo objeto ttn
    Heltec.begin(true /*displayEnable*/, false /*Lora Disable*/, true /*Serial Enable*/);
    
    ttn.begin();
    ttn.onMessage(message); // declare callback function when is downlink from server
    ttn.personalize(devAddr, nwkSKey, appSKey);

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
    
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(0,0, "Iniciando...");
    Heltec.display->drawString(0,10, String(ttn.getFrequency()));
    ttn.showStatus();
}

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
    }
    delay(10000);
}
