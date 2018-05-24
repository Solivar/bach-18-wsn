#include "stdmansos.h"
#include <net/socket.h>
#include <net/routing.h>
#include <net/address.h>

#define KEY 0xaa
#define ADDITIONAL_DELAY
// #define DEBUG_OUTPUT

typedef struct {
    uint16_t key;
    uint16_t id;
    uint16_t counter;
} Payload_t;

typedef struct node {
    uint16_t id;
    uint16_t counter;
    struct node* next;
} node_t;

static uint8_t radioBuffer[RADIO_MAX_PACKET];
static node_t* head = NULL;

int neighbourExists(uint16_t id) {
    node_t* current = head;

    while (current != NULL) {
        if (current->id == id) {
            return 1;
        }

        current = current->next;
    }

    return 0;
}

void updateNeighbour(uint16_t id, uint16_t counter) {
    node_t* current = head;

    while (current != NULL) {
        if (current->id == id) {
            #ifdef DEBUG_OUTPUT
                PRINTF("Last counter: %d, new counter: %d\n", current->counter, counter);
            #endif

            PRINTF("N %d %d %d %d\n", localAddress, current->id, current->counter, counter);

            if (counter - current->counter > 1) { // 0 means it's the first message, 1 means none were skipped
                #ifdef DEBUG_OUTPUT
                    PRINTF("Missed counter detected: %d\n", current->id);
                #endif
            }

            current->counter = counter;

            break;
        }

        current = current->next;
    }
}

void addNeighbour(uint16_t id, uint16_t counter) {
    node_t* current = head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = malloc(sizeof(node_t));
    current->next->id = id;
    current->next->counter = counter;
    current->next->next = NULL;

    #ifdef DEBUG_OUTPUT
        PRINTF("Added neighbour: %d\n", id);
    #endif
}

void addFirstNeighbour(uint16_t id, uint16_t counter) {
    head = malloc(sizeof(node_t));
    head->id = id;
    head->counter = counter;
    head->next = NULL;

    #ifdef DEBUG_OUTPUT
        PRINTF("Added first neighbour: %d\n", head->id);
    #endif
}

void recvRadio() {
    int16_t len;

    len = radioRecv(radioBuffer, sizeof(radioBuffer));

    if (len == sizeof(Payload_t)) {
        Payload_t* recvPayload = (Payload_t*) &radioBuffer;

        if(recvPayload->key == KEY) {
            #ifdef DEBUG_OUTPUT
                PRINTF("radio received from: %d with counter %d\n", recvPayload->id, recvPayload->counter);
            #endif

            if (head == NULL) {
                addFirstNeighbour(recvPayload->id, recvPayload->counter);
            }

            if (neighbourExists(recvPayload->id) == 1) {
                updateNeighbour(recvPayload->id, recvPayload->counter);
            } else {
                addNeighbour(recvPayload->id, recvPayload->counter);
            }
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
            while(i++ <= random) {
                mdelay(1);
            }
        #endif

        mdelay(1000);
    }
}
