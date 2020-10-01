package _01_pingpong_ideal;

@license(LGPL);

simple Txc
{
    parameters:
        bool sendMsgOnInit = default(false);
        volatile double delayTime @unit(s);
        @display("i=block/routing");
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
    submodules:
        tic: Tic;  
        toc: Toc;
    connections:
        tic.out --> {  delay = 100ms; } --> toc.in;
        tic.in <-- {  delay = 100ms; } <-- toc.out;
}
