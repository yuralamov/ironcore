#ifndef IRONCORE_PROTOCOL_H
#define IRONCORE_PROTOCOL_H

#include <stdint.h>

#define SERVER_DEFAULT_IP "192.168.0.150"
#define SERVER_PORT 7777
#define MAX_PLAYERS 16
#define TICK_RATE 30

typedef enum {
    PACKET_WELCOME = 1, // Сервер -> Клиент: подтверждение входа и ID
    PACKET_PING         // Клиент -> Сервер: тестовое сообщение
} PacketType;

#pragma pack(push, 1)

typedef struct {
    uint8_t type;       // PACKET_WELCOME
    uint8_t player_id;  // Выданный сервером слот
    uint32_t server_tick;
} PacketWelcome;

typedef struct {
    uint8_t type;       // PACKET_PING
    uint32_t client_time;
} PacketPing;

#pragma pack(pop)

#endif // IRONCORE_PROTOCOL_H
