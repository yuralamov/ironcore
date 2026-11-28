#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <enet/enet.h>
#include "../common/protocol.h"

static volatile bool is_running = true;

void handle_sigint(int sig) {
    (void)sig;
    is_running = false;
}

int main(void) {
    signal(SIGINT, handle_sigint);

    if (enet_initialize() != 0) {
        fprintf(stderr, "[SERVER] Failed to initialize ENet.\n");
        return EXIT_FAILURE;
    }

    ENetAddress address = {0};
    address.host = ENET_HOST_ANY;
    address.port = SERVER_PORT;

    ENetHost *server = enet_host_create(&address, MAX_PLAYERS, 2, 0, 0);
    if (server == NULL) {
        fprintf(stderr, "[SERVER] Failed to create ENet host on port %d.\n", SERVER_PORT);
        enet_deinitialize();
        return EXIT_FAILURE;
    }

    printf("[SERVER] IronCore server started on 0.0.0.0:%d (Tickrate: %d Hz)\n", SERVER_PORT, TICK_RATE);

    uint32_t current_tick = 0;
    const uint32_t tick_ms = 1000 / TICK_RATE;
    uint32_t last_tick_time = enet_time_get();

    ENetEvent event;
    while (is_running) {
        // Обрабатываем сетевые события с минимальной задержкой
        while (enet_host_service(server, &event, 1) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    uint8_t id = (uint8_t)event.peer->incomingPeerID;
                    printf("[SERVER] Client connected from %x:%u -> Assigned ID: %u\n",
                           event.peer->address.host, event.peer->address.port, id);

                    PacketWelcome pkt = {
                        .type = PACKET_WELCOME,
                        .player_id = id,
                        .server_tick = current_tick
                    };
                    ENetPacket *packet = enet_packet_create(&pkt, sizeof(pkt), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    if (event.packet->dataLength >= 1) {
                        uint8_t type = event.packet->data[0];
                        if (type == PACKET_PING && event.packet->dataLength == sizeof(PacketPing)) {
                            PacketPing *ping = (PacketPing *)event.packet->data;
                            printf("[SERVER] Ping from ID %u (client_time: %u)\n",
                                   event.peer->incomingPeerID, ping->client_time);
                        }
                    }
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    printf("[SERVER] Client ID %u disconnected.\n", event.peer->incomingPeerID);
                    break;
                }
                default:
                    break;
            }
        }

        // Фиксированный шаг симуляции
        uint32_t now = enet_time_get();
        if (now - last_tick_time >= tick_ms) {
            last_tick_time += tick_ms;
            current_tick++;
            // Здесь в будущем будет физика и рассылка снапшотов
        }
    }

    printf("\n[SERVER] Shutting down...\n");
    enet_host_destroy(server);
    enet_deinitialize();
    return EXIT_SUCCESS;
}
