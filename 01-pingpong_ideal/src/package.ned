package _01_pingpong_ideal;

@license(LGPL);

simple Node
{
    parameters:
        // Message timeout for resending
        volatile double timeout @unit(s);
        // Message loss probability
        volatile double loss;
        // Defines the "starting node"
        bool sendMsgOnInit = default(false);
        volatile double processingTime @unit(s);
        @display("i=block/routing");
        // Statistics
        @signal[processed](type="long");
        @statistic[processingTimeSignal](title="processing times";
                                         source="processed";
                                         record=stats;
                                         interpolationmode=none);
        @signal[arrived](type="long");
        @statistic[latencySignal](title="transmission times";
                                  source="arrived";
                                  record=stats;
                                  interpolationmode=none);
    gates:
        input in;
        output out;
}

simple Ping extends Node
{
    parameters:
        timeout = 10s;
        loss = 0.1;
        sendMsgOnInit = true;
        @display("i=,cyan");
}

simple Pong extends Node
{
    parameters:
        timeout = 10s;
        loss = 0.1;
        sendMsgOnInit = false;
        @display("i=,gold");
}

network PingPong
{
    types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        ping: Ping;  
        pong: Pong;
    connections:
        ping.out --> Channel --> pong.in;
        ping.in <-- Channel <-- pong.out;
}
