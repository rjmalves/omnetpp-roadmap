package _02_pingpong_ethernet;

@license(LGPL);

network PingPongEth
{
    submodules:
        configurator: inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
        ping: inet.node.inet.StandardHost;
        pong: inet.node.inet.StandardHost;
    connections:
        ping.ethg++ <--> inet.node.ethernet.Eth10M <--> pong.ethg++;
}