
/**
*   COMMAND SET
*/

#define ACK             (0x00 | (1 << 7))
#define NACK            (0x01 | (1 << 7))
#define RES_SEC         (0x0F | (1 << 7))
#define SET_TIME        (0x10 | (1 << 7))
#define SET_CONTROL     (0x11 | (1 << 7))
#define SET_CLRDEPTH    (0x12 | (1 << 7))
#define SET_TIMEON      (0x18 | (1 << 7))
#define SET_TIMEOFF     (0x19 | (1 << 7))
#define SET_TIMERON     (0x1A | (1 << 7))
#define SET_TIMEROFF    (0x1B | (1 << 7))

#define COLOUR_RED      (0x1)
#define COLOUR_GREEN    (0x2)
#define COLOUR_BLUE     (0x3)

#define SET_MINVAL      (0x14 | (1 << 7))
#define SET_MAXVAL      (0x15 | (1 << 7))

// Responses
#define OK              (0x00)
#define CHECK_CONFIG    (0x01)
#define RX_FIFO_FULL    (0x02)