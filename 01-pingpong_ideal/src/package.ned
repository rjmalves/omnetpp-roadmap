package _01_pingpong_ideal;

@license(LGPL);

simple Txc
{
    parameters:
        int address;
        int msgDestination;
        bool sendMsgOnInit = default(false);
        volatile double delayTime @unit(s);
        @display("i=block/routing");
        // Statistics
        @signal[processed](type="long");
        @statistic[processingTime](title="processing times";
                                   source="processed";
                                   record=vector,stats;
                                   interpolationmode=none);
        @signal[arrived](type="long");
        @statistic[latencyTime](title="transmission times";
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
        address = 0;
        msgDestination = 1;
        sendMsgOnInit = true;
        @display("i=,cyan");
}

simple Toc extends Txc
{
    parameters:
        address = 1;
        msgDestination = 0;
        sendMsgOnInit = false;
        @display("i=,gold");
}

network Tictoc
{
    types:
        channel Channel extends ned.DelayChannel {
            delay = truncnormal(0.5s, 0.1s);
        }
    submodules:
        tic: Tic;  
        toc: Toc;
    connections:
        tic.out --> Channel --> toc.in;
        tic.in <-- Channel <-- toc.out;
}
