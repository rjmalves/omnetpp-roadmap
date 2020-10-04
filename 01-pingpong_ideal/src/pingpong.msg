message PingPongMsg
{
    int source;
    int destination;
    simtime_t sendingTime;
    simtime_t recvTime;
    simtime_t processingTime = 0;
}