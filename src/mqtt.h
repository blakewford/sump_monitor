namespace mqtt
{
    int connect(const char* hostname, const char* username, const char* password);
    void publish(int sock, const char* message);
    void disconnect(int sock);
}