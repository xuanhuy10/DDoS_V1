import { useEffect, useState, useRef } from "react";
import { Space, Divider, Flex, Typography } from "antd";
import ReactEcharts from "echarts-for-react";

const { Title } = Typography;

import { socket } from "@/utils/socket";
import { bitFormatter, cntFormatter } from "@/lib/formatter";
import { getAllThresholdByActiveDefenseProfile } from "@/features/api/DefenseProfiles";

const AttackInfo = ({ pkt, attack_info }) => {
  const [dataNow, setDataNow] = useState({ packets: 0, bits: 0 });
  const [attackSettingsRate, setAttackRates] = useState(0); // Changed to number
  const [error, setError] = useState(null); // Added for error handling

  const attackChartRef = useRef(null);
  const chartDataRef = useRef([]);
  const maxPoints = 30;

  const fetchAttacksRate = async () => {
    try {
      const response = await getAllThresholdByActiveDefenseProfile();

      let rate;
      if (attack_info.type === "synFlood") {
        rate = response.data.SYNFloodRate;
      } else if (attack_info.type === "udpFlood") {
        rate = response.data.UDPFloodRate;
      }
      setAttackRates(rate || 0);
    } catch (error) {
      setError("Failed to retrieve monitor attack data");
    }
  };

  const aggregatePacketSize = (data) => {
    if (!Array.isArray(data)) return [];
    const result = {};
    data.forEach((record) => {
      result[record.dstIP] =
        (result[record.dstIP] || 0) + (record.packetSize || 0);
    });
    return Object.keys(result)
      .map((dstIP) => ({
        dstIP,
        sumOfAllPacketSize: result[dstIP],
      }))
      .sort((a, b) => b.sumOfAllPacketSize - a.sumOfAllPacketSize);
  };

  const thresholdDiff =
    attackSettingsRate > 0
      ? ((dataNow.packets - attackSettingsRate) / attackSettingsRate) * 100
      : 0;

  useEffect(() => {
    fetchAttacksRate();
    const updateChartData = (sec) => {
      if (
        !sec ||
        !sec.timeStamp ||
        !sec.attack ||
        !sec.attack[attack_info.type]
      )
        return;

      if (chartDataRef.current.length > maxPoints) {
        chartDataRef.current.shift();
      }
      chartDataRef.current.push([
        sec.timeStamp,
        sec.attack[attack_info.type].packets,
        sec.attack[attack_info.type].bits,
      ]);

      if (attackChartRef.current) {
        attackChartRef.current.getEchartsInstance().setOption({
          dataset: { source: chartDataRef.current },
        });
      }
    };

    const handleTraffic = (data) => {
      const sec = data?.onsec;
      if (!sec || !sec.attack || !sec.attack[attack_info.type]) {
        console.warn("Invalid sec data");
        return;
      }
      setDataNow(sec.attack[attack_info.type]);
      updateChartData(sec);
    };

    socket.on("traffic", handleTraffic);

    return () => {
      socket.off("traffic", handleTraffic);
    };
  }, [attack_info.type]);

  const attackChartOpt = {
    animation: false,
    grid: { left: 40, right: 50, top: 30, bottom: "21%" },
    xAxis: {
      type: "time",
      splitLine: { show: false },
      axisPointer: {
        snap: true,
        lineStyle: { color: "#004E52", opacity: 0.5, width: 1.5 },
        label: { show: true, backgroundColor: "#004E52" },
        handle: { show: true, color: "#004E52" },
      },
    },
    tooltip: {
      triggerOn: "none",
      position: (pt) => [pt[0], 130],
    },
    yAxis: [
      {
        type: "value",
        name: "pps",
        boundaryGap: [0, "100%"],
        alignTicks: true,
        axisLabel: { formatter: cntFormatter },
        axisLine: { show: true, lineStyle: { color: "#5470C6" } },
      },
      {
        type: "value",
        name: "bps",
        alignTicks: true,
        boundaryGap: [0, "100%"],
        position: "right",
        axisLabel: { formatter: bitFormatter },
        axisLine: { show: true, lineStyle: { color: "#91CC75" } },
      },
    ],
    dataset: {
      source: chartDataRef.current,
      dimensions: ["time", "packets", "bits"],
    },
    series: [
      {
        name: "pps",
        type: "line",
        encode: { x: "time", y: "packets" },
        animationDurationUpdate: 0,
      },
      {
        name: "bps",
        type: "bar",
        encode: { x: "time", y: "bits" },
        animationDurationUpdate: 0,
        yAxisIndex: 1,
      },
    ],
  };

  return (
    <div style={{ height: "350px" }} className="attacks-detail">
      <Flex style={{ height: "100%" }} gap={10} justify="space-between">
        <ReactEcharts
          ref={attackChartRef}
          option={attackChartOpt}
          style={{ width: "72%", height: "100%" }}
        />
        <div className="attacks-data">
          <Flex vertical size="small" gap={10}>
            <Title style={{ marginBottom: 0 }} level={4}>
              <span
                className="attack-counter"
                style={{
                  backgroundColor: pkt.length > 0 ? "red" : "#24c724",
                }}
              >
                {pkt.length}
              </span>
              Active attacks
            </Title>

            <Divider style={{ margin: 0 }} orientation="left" plain>
              Attack's metrics
            </Divider>

            <Space direction="vertical" size={0}>
              <p>
                {cntFormatter(dataNow.packets)}{" "}
                <span className="unit">packets/s</span>
              </p>
              <span>{thresholdDiff}% above threshold</span>
            </Space>

            <p>{bitFormatter(dataNow.bits)}/s</p>

            <Divider style={{ margin: 0 }} orientation="left" plain>
              Attack's destination
            </Divider>
            <ul>
              {aggregatePacketSize(pkt).map(
                ({ dstIP, sumOfAllPacketSize }, index) => (
                  <li
                    key={dstIP}
                    style={{
                      display: "flex",
                      justifyContent: "space-between",
                      width: "100%",
                      fontFamily: "monospace",
                      fontSize: "14px",
                      padding: "5px 0",
                      borderBottom: "1px solid #eee",
                    }}
                  >
                    <span style={{ minWidth: "150px" }}>
                      <strong>
                        {index + 1}. {dstIP}
                      </strong>
                    </span>
                    <span>{bitFormatter(sumOfAllPacketSize)}</span>
                  </li>
                )
              )}
            </ul>
          </Flex>
        </div>
      </Flex>
    </div>
  );
};

export default AttackInfo;
