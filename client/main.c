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

int main(int argc, char **argv) {
    signal(SIGINT, handle_sigint);
    const char *server_ip = (argc > 1) ? argv[1] : SERVER_DEFAULT_IP;

    if (enet_initialize() != 0) {
        fprintf(stderr, "[CLIENT] Failed to initialize ENet.\n");
        return EXIT_FAILURE;
    }

    ENetHost *client = enet_host_create(NULL, 1, 2, 0, 0);
    if (client == NULL) {
        fprintf(stderr, "[CLIENT] Failed to create ENet client.\n");
        enet_deinitialize();
        return EXIT_FAILURE;
    }

    ENetAddress address = {0};
    enet_address_set_host(&address, server_ip);
    address.port = SERVER_PORT;

    printf("[CLIENT] Connecting to %s:%d...\n", server_ip, SERVER_PORT);
    ENetPeer *peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL) {
        fprintf(stderr, "[CLIENT] No available peers.\n");
        enet_host_destroy(client);
        enet_deinitialize();
        return EXIT_FAILURE;
    }

    ENetEvent event;
    bool connected = false;
    uint32_t last_ping = 0;

    while (is_running) {
        while (enet_host_service(client, &event, 10) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    uint8_t id = (uint8_t)event.peer->incomingPeerID;
                    char ip_str[64] = "unknown";
                    enet_address_get_host_ip(&event.peer->address, ip_str, sizeof(ip_str));

                    printf("[SERVER] Client connected from %s:%u -> Assigned ID: %u\n",
                           ip_str, event.peer->address.port, id);

                    PacketWelcome pkt = {
                        .type = PACKET_WELCOME,
                        .player_id = id,
                        .server_tick = current_tick
                    };
                    ENetPacket *packet = enet_packet_create(&pkt, sizeof(pkt), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE:
                    if (event.packet->dataLength >= 1) {
                        uint8_t type = event.packet->data[0];
                        if (type == PACKET_WELCOME && event.packet->dataLength == sizeof(PacketWelcome)) {
                            PacketWelcome *welcome = (PacketWelcome *)event.packet->data;
                            printf("[CLIENT] Welcome received! My ID: %u, Server Tick: %u\n",
                                   welcome->player_id, welcome->server_tick);
                        }
                    }
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("[CLIENT] Disconnected from server.\n");
                    connected = false;
                    is_running = false;
                    break;

                default:
                    break;
            }
        }

        // Раз в секунду отправляем тестовый пакет
        uint32_t now = enet_time_get();
        if (connected && (now - last_ping >= 1000)) {
            last_ping = now;
            PacketPing ping = {
                .type = PACKET_PING,
                .client_time = now
            };
            ENetPacket *packet = enet_packet_create(&ping, sizeof(ping), ENET_PACKET_FLAG_UNSEQUENCED);
            enet_peer_send(peer, 1, packet);
        }
    }

    if (connected) {
        enet_peer_disconnect_now(peer, 0);
    }
    enet_host_destroy(client);
    enet_deinitialize();
    printf("[CLIENT] Exited cleanly.\n");
    return EXIT_SUCCESS;
}
