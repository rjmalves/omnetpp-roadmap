package _03_ethernet_lan;

@license(LGPL);

network EthernetLAN
{
    parameters:
        int clients;
    submodules:
        configurator: inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
        switch: inet.node.ethernet.EtherSwitch;
        client[clients]: inet.node.inet.StandardHost; 
    connections:
        for i=0..clients-1 {
            client[i].ethg++ <--> inet.node.ethernet.Eth10M <--> switch.ethg++;   
        }
}

network EthernetLANWithGateway
{
    parameters:
        int clients;
    submodules:
        configurator: inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
        server: inet.node.inet.StandardHost;
        router: inet.node.inet.Router;
        switch: inet.node.ethernet.EtherSwitch;
        client[clients]: inet.node.inet.StandardHost;
    connections:
        router.pppg++ <--> inet.node.ethernet.Eth100M <--> server.pppg++;
        switch.ethg++ <--> inet.node.ethernet.Eth100M <--> router.ethg++;
        for i=0..clients-1 {
            client[i].ethg++ <--> inet.node.ethernet.Eth10M <--> switch.ethg++;   
        }
}