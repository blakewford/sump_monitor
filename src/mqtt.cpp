#include <cstdio>
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>

#define MQTT_PORT 1883

namespace mqtt
{
    enum control_packet_type: uint8_t
    {
        RESERVED,
        CONNECT,
        CONNACK,
        PUBLISH,
        PUBACK,
        PUBREC,
        PUBREL,
        PUBCOMP,
        SUBSCRIBE,
        SUBACK,
        UNSUBSCRIBE,
        UNSUBACK,
        PINGREQ,
        PINGRESP,
        DISCONNECT
//      RESERVED
    };

    struct header
    {
        uint8_t type_flags = 0;
        int8_t remaining_length = 0;

        void set_type(control_packet_type type)
        {
            type_flags = type << 4;
        }
    };

    struct connect_header
    {
        const uint16_t protocol_length = htons(4);
        const char protocol[4] = {'M', 'Q', 'T', 'T'};
        const uint8_t level = 0x04;
        const uint8_t flags = 0xC2;
        const uint16_t keep_alive = htons(60); // seconds

        // Technically variable
        const uint16_t client_id_length = htons(4);
        const char client_id[4] = {'S', 'U', 'M', 'P'};
    };

    struct connack_header
    {
        uint8_t flags;
        int8_t return_code = -1;
    };

    void gethostbyname(const char* hostname, std::string& ip)
    {
        ip.clear();

        std::string command = "ping ";
        command += hostname;

        FILE* f = popen(command.c_str(), "r");

        const int32_t BUFFER_SIZE = 64;
        char buffer[BUFFER_SIZE];
        memset(buffer, '\0', BUFFER_SIZE);
        fread(buffer, BUFFER_SIZE, 1, f);
        pclose(f);

        ip = buffer + strlen(hostname) + 7;
        ip = ip.substr(0, ip.find(')'));
    }

    int connect(const char* hostname, const char* username, const char* password)
    {
        std::string ip;
        ip.clear();

        gethostbyname(hostname, ip);

        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(MQTT_PORT);
        inet_pton(AF_INET, ip.c_str(), &address.sin_addr);

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if(connect(sock, (struct sockaddr *)&address, sizeof(address)) != 0)
        {
            return -1;
        }

        header h;
        h.set_type(CONNECT);
        h.remaining_length = sizeof(connect_header) + strlen(username) + strlen(password) + 2*sizeof(uint16_t);
        ssize_t sent = send(sock, &h, sizeof(header), MSG_WAITALL);
        if(sent != sizeof(header)) return -1;

        connect_header ch;
        sent = send(sock, &ch, sizeof(connect_header), MSG_WAITALL);
        if(sent != sizeof(connect_header)) return -1;

        uint16_t string_length = htons(strlen(username));
        sent = send(sock, &string_length, sizeof(uint16_t), MSG_WAITALL);
        if(sent != sizeof(uint16_t)) return -1;
        sent = send(sock, username, strlen(username), MSG_WAITALL);
        if(sent != strlen(username)) return -1;
        string_length = htons(strlen(password));
        sent = send(sock, &string_length, sizeof(uint16_t), MSG_WAITALL);
        if(sent != sizeof(uint16_t)) return -1;
        sent = send(sock, password, strlen(password), MSG_WAITALL);
        if(sent != strlen(password)) return -1;

        connack_header ack;
        size_t returned = recv(sock, &h, sizeof(header), MSG_WAITALL);
        if(returned != sizeof(header)) return -1;
        returned = recv(sock, &ack, sizeof(connack_header), MSG_WAITALL);
        if(returned != sizeof(connack_header)) return -1;

        if(ack.return_code != 0) return -1;

        return sock;
    }

    struct publish_header
    {
        const uint16_t topic_name_length = htons(6);
        const char topic[6] = {'L', 'I', 'Q', 'U', 'I', 'D'};
    };

    void publish(int sock, const char* message)
    {
        size_t payload_length = strlen(message);

        header h;
        h.set_type(PUBLISH);
        h.remaining_length = sizeof(publish_header) + payload_length;
        send(sock, &h, sizeof(header), MSG_WAITALL);

        publish_header ph;
        send(sock, &ph, sizeof(publish_header), MSG_WAITALL);
        send(sock, message, payload_length, MSG_WAITALL);
    }

    void disconnect(int sock)
    {
        header h;
        h.set_type(DISCONNECT);
        h.remaining_length = 0;
        send(sock, &h, sizeof(header), MSG_WAITALL);
    }

}