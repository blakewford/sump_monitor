namespace mqtt
{
    int connect(const char* ip);
    void publish(int sock, const char* message);
    void disconnect(int sock);
}