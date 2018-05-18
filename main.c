#include "stdmansos.h"
#include <net/socket.h>
#include <net/routing.h>
#include <net/address.h>

#define KEY 0xaa
#define ADDITIONAL_DELAY

typedef struct {
    uint16_t key;
    uint16_t id;
    uint16_t counter;
} Payload_t;
Payload_t Payload;

static uint8_t radioBuffer[RADIO_MAX_PACKET];

void recvRadio(void) {
    int16_t len;

    len = radioRecv(radioBuffer, sizeof(radioBuffer));

    if (len == sizeof(Payload_t)) {
        PRINTF("radio received %d bytes\n", len);
        Payload_t* recvPayload = (Payload_t*) &radioBuffer;

        if(recvPayload->key == KEY) {
            PRINTF("radio received from: %d with counter %d\n", recvPayload->id, recvPayload->counter);
        }
    }
}

void appMain(void) {
    radioSetReceiveHandle(recvRadio);
    radioOn();

    Payload_t message;
    message.key = KEY;
    message.id = localAddress;
    message.counter = 0;

    uint32_t now = getTimeMs();
    randomSeed(now);

    while (1) {
        radioSend((void*)&message, sizeof(message));
        message.counter++;


        #ifdef ADDITIONAL_DELAY
            uint16_t random = randomInRange(1, 500);
            // 0.1s to 0.5s of delay
            uint16_t i = 0;
            while(i <= random) {
                i++;
                mdelay(1);
            }
        #endif

        mdelay(1000);
    }
}
