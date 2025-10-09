import ReactEcharts from "echarts-for-react";
import { theme as antdTheme, Tooltip } from "antd";
import { bitFormatter } from "@/lib/formatter";
import { memo, useCallback } from "react";
import { InfoCircleOutlined } from "@ant-design/icons";

const ChartInfo = memo(({ title }) => {
  const handleMouseEnter = useCallback((e) => {
    e.target.style.opacity = "1";
  }, []);
  const handleMouseLeave = useCallback((e) => {
    e.target.style.opacity = "0.7";
  }, []);
  return (
    <div style={{ position: "absolute", top: 5, right: 8, zIndex: 10 }}>
      <Tooltip title={title} placement="topRight">
        <InfoCircleOutlined
          style={{
            fontSize: "16px",
            color: "#666",
            cursor: "pointer",
            opacity: 0.7,
            transition: "opacity 0.2s",
          }}
          onMouseEnter={handleMouseEnter}
          onMouseLeave={handleMouseLeave}
        />
      </Tooltip>
    </div>
  );
});

const TrendChart = ({ data, option, title = "Trend Chart" }) => {
  const { useToken } = antdTheme;
  const { token: theme } = useToken();
  const protocol_option = {
    tooltip: {
      trigger: "item",
      formatter: ({ data }) =>
        `<strong>${data.name}:</strong> ${bitFormatter(data.value)}`,
      textStyle: {
        fontWeight: "bold",
      },
    },
    color: [
      theme.tcpColor,
      theme.udpColor,
      theme.dnsColor,
      theme.icmpColor,
      theme.httpColor,
      theme.espColor,
      theme.unknownColor,
    ],
    legend: {
      data: ["TCP", "UDP", "DNS", "ICMP", "HTTP", "ESP", "Unknown"],
      textStyle: {
        color: theme.colorText,
      },
      show: false,
    },
    series: [
      {
        name: "Total Packets",
        type: "pie",
        roseType: "area",
        itemStyle: {
          borderRadius: 5,
        },
        selectedMode: "single",
        radius: ["20%", "75%"],
        data: [
          {
            value: data?.tcp?.bypass?.bits + data?.tcp?.attack?.bits || 0,
            name: "TCP",
          },
          {
            value: data?.udp?.bypass?.bits + data?.udp?.attack?.bits || 0,
            name: "UDP",
          },
          {
            value: data?.dns?.bypass?.bits + data?.dns?.attack?.bits || 0,
            name: "DNS",
          },
          {
            value: data?.icmp?.bypass?.bits + data?.icmp?.attack?.bits || 0,
            name: "ICMP",
          },
          {
            value: data?.http?.bypass?.bits + data?.http?.attack?.bits || 0,
            name: "HTTP",
          },
          {
            value: data?.esp?.bypass?.bits + data?.esp?.attack?.bits || 0,
            name: "ESP",
          },
          {
            value:
              data?.unknown?.bypass?.bits + data?.unknown?.attack?.bits || 0,
            name: "Unknown",
          },
        ],
      },
    ],
  };
  const attack_option = {
    tooltip: {
      trigger: "item",
      formatter: ({ data }) =>
        `<strong>${data.name}:</strong> ${bitFormatter(data.value)}`,
      textStyle: {
        fontWeight: "bold",
      },
    },
    color: [
      theme.synFloodColor,
      theme.udpFloodColor,
      theme.icmpFloodColor,
      theme.dnsFloodColor,
      theme.httpFloodColor,
      theme.ipsecColor,
      theme.tcpFragColor,
      theme.udpFragColor,
      theme.landColor,
      theme.normal,
    ],
    legend: {
      data: [
        "SYN Flood",
        "UDP Flood",
        "ICMP Flood",
        "DNS Flood",
        "HTTP Flood",
        "IPSec IKE",
        "TCP Fragment",
        "UDP Fragment",
        "Land Attack",
        "Normal",
      ],
      textStyle: {
        color: theme.colorText,
      },
      show: false,
    },
    series: [
      {
        name: "Total Bytes",
        type: "pie",
        roseType: "area",
        itemStyle: {
          borderRadius: 5,
        },
        selectedMode: "single",
        radius: ["20%", "75%"],
        data: [
          { value: data?.synFlood?.bits || 0, name: "SYN Flood" },
          { value: data?.udpFlood?.bits || 0, name: "UDP Flood" },
          { value: data?.icmpFlood?.bits || 0, name: "ICMP Flood" },
          { value: data?.dnsFlood?.bits || 0, name: "DNS Flood" },
          { value: data?.httpFlood?.bits || 0, name: "HTTP Flood" },
          { value: data?.ipsec?.bits || 0, name: "IPSec IKE" },
          { value: data?.tcpFrag?.bits || 0, name: "TCP Fragment" },
          { value: data?.udpFrag?.bits || 0, name: "UDP Fragment" },
          { value: data?.land?.bits || 0, name: "Land Attack" },
          { value: data?.unknown?.bits || 0, name: "Normal" },
        ],
      },
    ],
  };
  return (
    <>
      <ChartInfo title={title} />
      <ReactEcharts
        option={option === "protocol" ? protocol_option : attack_option}
        style={{ width: "100%", height: "100%" }}
      />
    </>
  );
};

export default TrendChart;
