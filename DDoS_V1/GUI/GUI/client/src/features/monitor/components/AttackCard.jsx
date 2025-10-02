import React, { useState, useEffect } from "react";
import { Link } from "react-router-dom";

import { Card, Flex, Space, Tag } from "antd";
import {
  RightOutlined,
  SyncOutlined,
  ExportOutlined,
  CloseOutlined,
  SwitcherOutlined,
  ExclamationCircleTwoTone,
} from "@ant-design/icons";
import ReactEcharts from "echarts-for-react";

import { bitFormatter, cntFormatter } from "@/lib/formatter";

import "../styles/main.css";

const AttackCard = ({ type, rate, sec, fetchRate }) => {
  const [attack, setAttack] = useState({});

  const attackGaugeOption = () => {
    return {
      series: [
        {
          type: "gauge",
          startAngle: 200,
          endAngle: -20,
          center: ["50%", "60%"],
          radius: "90%",
          min: rate,
          max: rate * 1000,
          splitNumber: 3,
          axisLine: {
            lineStyle: {
              width: 9,
              color: [
                [(rate * 200 - rate) / (rate * 1000 - rate), "#00983F"],
                [(rate * 500 - rate) / (rate * 1000 - rate), "#FFE02E"],
                [1, "#FF4751"],
              ],
            },
          },
          pointer: {
            icon: "path://M12.8,0.7l12,40.1H0.7L12.8,0.7z",
            length: "10%",
            width: 9,
            offsetCenter: [0, "-55%"],
            itemStyle: {
              color: "auto",
            },
          },
          axisTick: {
            length: 9,
            lineStyle: {
              color: "auto",
              width: 2,
            },
          },
          splitLine: {
            length: 11,
            lineStyle: {
              color: "auto",
              width: 3,
            },
          },
          axisLabel: {
            color: "#464646",
            fontSize: 11,
            distance: -30,
            rotate: "tangential",
            formatter: function (value) {
              return cntFormatter(value);
            },
          },
          anchor: {
            show: false,
          },
          detail: {
            fontSize: 15,
            offsetCenter: [0, "-15%"],
            valueAnimation: true,
            formatter: function (value) {
              return cntFormatter(value);
            },
            color: "inherit",
          },
          data: [
            {
              value: sec?.packets ?? 0,
            },
          ],
        },
      ],
    };
  };

  useEffect(() => {
    switch (type) {
      case "synFlood":
        setAttack({
          title: "SYN Flood",
          link: "/monitor/synFlood",
        });
        break;
      case "udpFlood":
        setAttack({
          title: "UDP Flood",
          link: "/monitor/udpFlood",
        });
        break;
      case "icmpFlood":
        setAttack({
          title: "ICMP Flood",
          link: "/monitor/icmpFlood",
        });
        break;
      case "dnsFlood":
        setAttack({
          title: "DNS Flood",
          link: "/monitor/dnsFlood",
        });
        break;
      case "httpFlood":
        setAttack({
          title: "HTTP Flood",
          link: "/monitor/httpFlood",
        });
        break;
      case "httpsFlood":
        setAttack({
          title: "HTTPS Flood",
          link: "/monitor/httpsFlood",
        });
        break;
      case "land":
        setAttack({
          title: "Land Attack",
          link: "/monitor/land",
        });
        break;
      case "ipsec":
        setAttack({
          title: "IPSec IKE Flood",
          link: "/monitor/ipsec",
        });
        break;
      case "tcpFrag":
        setAttack({
          title: "TCP Fragment Attack",
          link: "/monitor/tcpFrag",
        });
        break;

      case "udpFrag":
        setAttack({
          title: "UDP Fragment Attack",
          link: "/monitor/udpFrag",
        });
        break;

      default:
    }
  }, []);

  return (
    <>
      <Card className={sec?.packets > 0 ? "attacking_card" : ""}>
        <Flex vertical align="center" style={{ width: "100%" }}>
          <Flex justify="space-between" style={{ width: "100%" }}>
            <Space>
              {sec?.packets > 0 ? (
                <ExclamationCircleTwoTone twoToneColor="#FF4251" />
              ) : null}
              {sec?.packets > 0 ? (
                <span className="attacking_card_title">{attack.title}</span>
              ) : (
                <span className="attack_card_title">{attack.title}</span>
              )}
            </Space>
            <Space>
              <ExportOutlined
                onClick={() => {
                  window.open(attack.link, "_blank");
                }}
              />
              <SyncOutlined
                onClick={() => {
                  fetchRate();
                }}
              />
              <SwitcherOutlined />
              <CloseOutlined />
            </Space>
          </Flex>
          <div style={{ width: "90%", height: "200px" }}>
            <ReactEcharts
              style={{ height: "100%", width: "100%" }}
              option={attackGaugeOption()}
            />
          </div>
          <Flex
            align="center"
            justify="space-between"
            className="attack_card_stats"
          >
            {sec?.packets > 0 ? (
              <Tag color="#f50">Attacking</Tag>
            ) : (
              <Tag color="#7CD356">No Attack</Tag>
            )}
            <span>Throughput: {bitFormatter(sec?.bits ?? 0)}/s</span>
            <Link to={attack.link}>
              <RightOutlined />
            </Link>
          </Flex>
        </Flex>
      </Card>
    </>
  );
};

export default AttackCard;
