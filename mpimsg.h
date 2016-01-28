#ifndef MPIMSG_HEADER_FILE
#define MPIMSG_HEADER_FILE

#define WMSG_READY (1<<0)
#define MSG_RUN (1<<1)
#define MSG_QUIT (1<<2)

#define MSG(_cmd, _data) ((_cmd) << 24 | _data)
#define MSG_CMD(_msg) ((_msg) >> 24)
#define MSG_DATA(_msg) ((_msg) & 0x00FFFFFF)

#endif // MPIMSG_HEADER_FILE
