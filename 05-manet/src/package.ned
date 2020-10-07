package _05_manet;

@license(LGPL);

network MANET
{
    parameters:
        int numHosts;
    submodules:
        configurator: inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
        radioMedium: inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
        host[numHosts]: inet.node.inet.AdhocHost;
}