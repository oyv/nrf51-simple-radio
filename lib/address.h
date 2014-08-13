#ifndef ADDRESS_H_
#define ADDRESS_H_

#define ADDRESS_LEN (5)
#define BASE_LEN (ADDRESS_LEN-1)
#define BROADCAST_ADDRESS_NUM (0)
#define DEV_ADDRESS_NUM (1)
#define TX_ADDRESS_NUM (7)

typedef struct address_t
{
    uint8_t address[ADDRESS_LEN];   
} address_t;

#endif
