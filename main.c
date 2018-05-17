#include "stdmansos.h"
#include <stdlib.h> // rand()
#include <string.h>
#include <net/socket.h>
#include <net/routing.h>
#include <net/address.h>

// localAddress // uint16_t or uint8_t
#define KEY 0xaa
// #define KEY_SIZE sizeof(KEY)

typedef struct {
    uint16_t key;
    uint16_t id;
} Payload_t;
Payload_t Payload;

static uint8_t radioBuffer[RADIO_MAX_PACKET];

void recvRadio(void) {
    int16_t len;

    len = radioRecv(radioBuffer, sizeof(radioBuffer)); // get the length of received packet
    //greenLedToggle();

    if (len == sizeof(Payload_t)) {
        PRINTF("radio receive %d bytes\n", len);
        Payload_t* recvPayload = (Payload_t*) &radioBuffer;

        if(recvPayload->key == KEY) {
            PRINTF("radio received from: %d\n", recvPayload->id);
        }
    }
}

void appMain(void) {
    radioSetReceiveHandle(recvRadio);
    radioOn();

    Payload_t message;
    message.key = KEY;
    message.id = localAddress;
    // TODO: add counter - katru reizi kad dzird ziņu no kaimiņa, piefiksē counter, kad saņem nākamo ziņu no kaimiņa, tad var
    // paskatīties vai kāda ziņa ir izlaista

    while (1) {
        radioSend((void*)&message, sizeof(message));
        //redLedToggle();

        // TODO: sekunde + rand, kas ir ar DEFINE, lai būtu 1 - 1.5s delay
        mdelay(1000 * 2);
    }
}
