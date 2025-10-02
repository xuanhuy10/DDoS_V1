const processPacketDate = (data) => {
    try {
        if (!data) return;
        const shortedData = {
            total: {
                received: data.total.bypass.packets,
                drop: -data.total.attack.packets,
                total: data.total.bypass.packets + data.total.attack.packets
            },
            icmp: {
                received: data.icmp.bypass.packets,
                drop: -data.icmp.attack.icmpFlood.packets,
                total: data.icmp.bypass.packets + data.icmp.attack.icmpFlood.packets
            },
            udp: {
                received: data.udp.bypass.packets,
                drop: -(data.udp.attack.udpFlood.packets + data.udp.attack.dnsFlood.packets),
                total: data.udp.bypass.packets + data.udp.attack.udpFlood.packets + data.udp.attack.dnsFlood.packets
            },
            tcp: {
                received: data.tcp.bypass.packets,
                drop: -(data.tcp.attack.tcpFrag.packets + data.tcp.attack.synFlood.packets),
                total: data.tcp.bypass.packets + data.tcp.attack.tcpFrag.packets + data.tcp.attack.synFlood.packets
            },
            http: {
                received: data.http.bypass.packets,
                drop: -(data.http.attack.httpFlood.packets),
                total: data.http.bypass.packets + data.http.attack.httpFlood.packets
            },
            esp: {
                received: data.esp.bypass.packets,
                drop: -(data.esp.attack.ipsec.packets),
                total: data.esp.bypass.packets + data.esp.attack.ipsec.packets
            },
            unknown: {
                received: data.unknown.bypass.packets,
                drop: -(data.unknown.attack.unknown.packets),
                total: data.unknown.bypass.packets + data.unknown.attack.unknown.packets
            }
        };
        setChartPacketData(shortedData);
    } catch (error) {
        console.log(error);
    }
}