import React, { useEffect, useState } from "react";
import { Checkbox, Row, Col, Card, Alert } from "antd";

import { socket } from "@/utils/socket";

import AttackCard from "@/features/monitor/components/AttackCard";

import { getAllThresholdByActiveDefenseProfile } from "@/features/api/DefenseProfiles";

export default function Monitor() {
  // const [checkedCards, setCheckedCards] = useState([]);

  // const cards = new Array(9).fill(null).map((_, i) => `Attack Type ${i + 1}`);

  // const handleCheckboxChange = (card, checked) => {
  //   setCheckedCards((prev) => {
  //     if (!checked) {
  //       return prev.filter((item) => item !== card);
  //     }
  //     return prev.includes(card) ? prev : [...prev, card];
  //   });
  // };
  const [error, setError] = useState(null);
  const [attackSettingsRate, setAttackRates] = useState([]);

  const [sec, setSec] = useState({
    attack: {
      synFlood: {
        bits: 0,
        packets: 0,
      },
      udpFlood: {
        bits: 0,
        packets: 0,
      },
      icmpFlood: {
        bits: 0,
        packets: 0,
      },
      dnsFlood: {
        bits: 0,
        packets: 0,
      },
      httpFlood: {
        bits: 0,
        packets: 0,
      },
      land: {
        bits: 0,
        packets: 0,
      },
      ipsec: {
        bits: 0,
        packets: 0,
      },
      tcpFrag: {
        bits: 0,
        packets: 0,
      },
      udpFrag: {
        bits: 0,
        packets: 0,
      },
    },
  });

  const fetchAttacksRate = async () => {
    try {
      const response = await getAllThresholdByActiveDefenseProfile();
      setAttackRates(response.data);
    } catch {
      setError("Failed to retrieve monitor attack data");
    }
  };

  useEffect(() => {
    fetchAttacksRate();

    socket.on("traffic", (data) => {
      setSec((prev) => ({
        ...prev,
        attack: data.onsec?.attack || prev.attack,
      }));
    });

    return () => {
      socket.off("traffic");
    };
  }, []);

  // console.log("data in synsFlood:", sec.attack.synFlood);
  // console.log("data in dnsFlood:", sec.attack.dnsFlood);
  // console.log("data in unknown:", sec.attack.unknown);
  console.log(
    "attackSettingRate in synsFlood:",
    attackSettingsRate.SYNFloodRate
  );
  console.log(
    "attackSettingRate in dnsFlood:",
    attackSettingsRate.DNSFloodRate
  );
  console.log("attackSettingRate in unknown:", attackSettingsRate.UnknownRate);

  return (
    <div style={{ padding: "25px 5px" }}>
      {error && (
        <Alert
          message={error}
          type="error"
          showIcon
          style={{ marginBottom: 16 }}
        />
      )}
      <Row gutter={[32, 24]}>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="synFlood"
            rate={attackSettingsRate.SYNFloodRate || 0}
            sec={sec.attack.synFlood || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="udpFlood"
            rate={attackSettingsRate.UDPFloodRate || 0}
            sec={sec.attack.udpFlood || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="icmpFlood"
            rate={attackSettingsRate.ICMPFloodRate || 0}
            sec={sec.attack.icmpFlood || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="dnsFlood"
            rate={attackSettingsRate.DNSFloodRate || 0}
            sec={sec.attack.dnsFlood || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="httpFlood"
            rate={attackSettingsRate.SYNFloodRate || 0}
            sec={sec.attack.httpFlood || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="httpsFlood"
            rate={attackSettingsRate.SYNFloodRate || 0}
            sec={sec.attack.httpFlood || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="land"
            rate={attackSettingsRate.SYNFloodRate || 0}
            sec={sec.attack.land || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="ipsec"
            rate={attackSettingsRate.UDPFloodRate || 0}
            sec={sec.attack.ipsec || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="tcpFrag"
            rate={attackSettingsRate.SYNFloodRate || 0}
            sec={sec.attack.tcpFrag || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
        <Col xs={24} sm={24} md={12} lg={6}>
          <AttackCard
            type="udpFrag"
            rate={attackSettingsRate.UDPFloodRate || 0}
            sec={sec.attack.udpFrag || { bits: 0, packets: 0 }}
            fetchRate={fetchAttacksRate}
          />
        </Col>
      </Row>
    </div>
  );
}
