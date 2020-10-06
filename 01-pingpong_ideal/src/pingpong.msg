message PingPongMsg
{
    simtime_t sendingTime;
    simtime_t recvTime;
    simtime_t processingTime = 0;
}