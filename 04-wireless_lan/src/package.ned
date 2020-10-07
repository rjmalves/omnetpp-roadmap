package _04_wireless_lan;

@license(LGPL);

network WirelessLAN
{
    submodules:
        configurator: inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
        radioMedium: inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
        host1: inet.node.inet.WirelessHost { @display("p=200,100"); }
        host2: inet.node.inet.WirelessHost { @display("p=500,100"); }
        accessPoint: inet.node.wireless.AccessPoint { @display("p=374,200"); }
}
