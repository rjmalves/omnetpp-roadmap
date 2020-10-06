package _01_pingpong_ideal;

@license(LGPL);

simple Txc
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
                                         record=vector,stats;
                                         interpolationmode=none);
        @signal[arrived](type="long");
        @statistic[latencySignal](title="transmission times";
                                  source="arrived";
                                  record=vector,stats;
                                  interpolationmode=none);
    gates:
        input in;
        output out;
}

simple Tic extends Txc
{
    parameters:
        timeout = 10s;
        loss = 0.1;
        sendMsgOnInit = true;
        @display("i=,cyan");
}

simple Toc extends Txc
{
    parameters:
        timeout = 10s;
        loss = 0.1;
        sendMsgOnInit = false;
        @display("i=,gold");
}

network Tictoc
{
    types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        tic: Tic;  
        toc: Toc;
    connections:
        tic.out --> Channel --> toc.in;
        tic.in <-- Channel <-- toc.out;
}
