import React, { useEffect, useState } from "react";
import { useParams } from "react-router-dom";
import { Flex, Space, Col, Row, Card } from "antd";

import { socket } from "@/utils/socket";

import PageTitle from "@/components/common/PageTitle";
import FlowTable from "@/features/monitor/components/FlowTable";
import AttackSettings from "@/features/monitor/components/AttackSettings";
import AttackInfo from "@/features/monitor/components/AttackInfo";

export default function AttackDetails() {
  //const [ attackInfo, setAttackInfo ] = useState({ title: 'Unknown', type: 'unknown', description: 'An attack that overwhelms a server by sending a large number of packets, consuming resources and disrupting legitimate connections.' });

  const { attackType } = useParams();

  const [attackInfo] = useState(() => {
    switch (attackType) {
      case "synFlood":
        return {
          title: "SYN Flood",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending TCP SYN requests without completing the handshake, exhausting resources and disrupting legitimate connections.",
        };
      case "udpFlood":
        return {
          title: "UDP Flood",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending a large number of UDP packets, consuming resources and disrupting legitimate connections.",
        };
      case "icmpFlood":
        return {
          title: "ICMP Flood",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending a large number of ICMP packets, consuming resources and disrupting legitimate connections.",
        };
      case "dnsFlood":
        return {
          title: "DNS Flood",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending a large number of DNS requests, consuming resources and disrupting legitimate connections.",
        };
      case "httpFlood":
        return {
          title: "HTTP Flood",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending a large number of HTTP requests, consuming resources and disrupting legitimate connections.",
        };
      case "httpsFlood":
        return {
          title: "HTTPs Flood",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending a large number of HTTPs requests, consuming resources and disrupting legitimate connections.",
        };
      case "land":
        return {
          title: "LAND Attack",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending packets with the same source and destination IP address, causing a denial of service.",
        };
      case "ipsec":
        return {
          title: "IPSec IKE Flood",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending a large number of IPSec IKE packets, consuming resources and disrupting legitimate connections.",
        };
      case "tcpFrag":
        return {
          title: "TCP Fragment",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending fragmented TCP packets, consuming resources and disrupting legitimate connections.",
        };
      case "udpFrag":
        return {
          title: "UDP Fragment",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending fragmented UDP packets, consuming resources and disrupting legitimate connections.",
        };
      default:
        return {
          title: "Unknown",
          type: attackType,
          description:
            "An attack that overwhelms a server by sending a large number of packets, consuming resources and disrupting legitimate connections.",
        };
    }
  });

  const [pkt, setPkt] = useState(0);

  useEffect(() => {
    socket.on("packetMonit", (pkt) => {
      setPkt(pkt.filter((p) => p.attackType === attackInfo.title));
    });

    return () => {
      socket.off("packetMonit");
    };
  }, []);

  return (
    <>
      <PageTitle
        title={attackInfo.title}
        description={attackInfo.description}
      />
      <Row gutter={[16, 16]}>
        <Col span={5}>
          <AttackSettings attack_info={attackInfo} setThreshold={setPkt} />
        </Col>
        <Col span={19}>
          <Card
            size="small"
            title={
              <Flex>
                <span style={{ fontSize: "0.9rem" }}>
                  {attackInfo.title}'s Traffic
                </span>
                <Space></Space>
              </Flex>
            }
            style={{ height: "100%" }}
          >
            <AttackInfo pkt={pkt} attack_info={attackInfo} />
          </Card>
        </Col>
        <Col span={24}>
          <FlowTable pkt={pkt} attack_info={attackInfo} />
        </Col>
      </Row>
    </>
  );
}
