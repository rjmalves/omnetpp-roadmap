package _01_pingpong_ideal;

@license(LGPL);

simple Txc
{
    parameters:
        bool sendMsgOnInit = default(false);
        volatile double delayTime @unit(s);
        @display("i=block/routing");
        // Statistics
        @signal[processed](type="long");
        @statistic[processingTime](title="processing times";
                                   source="processed";
                                   record=vector,stats;
                                   interpolationmode=none);
    gates:
        input in;
        output out;
}

simple Tic extends Txc
{
    parameters:
        sendMsgOnInit = true;
        @display("i=,cyan");
}

simple Toc extends Txc
{
    parameters:
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
