import { useEffect, useState, useRef } from "react";
import { Card, Space, Flex, Typography, Button, Divider, Row, Col } from "antd";

import ReactEcharts from "echarts-for-react";
import { theme as antdTheme } from "antd";

import "@/features/dashboard/styles/main.css";

import { timeFormatter, bitFormatter } from "@/lib/formatter";

const ProtocolChart = ({ protocolData, time, paused }) => {
  const { useToken } = antdTheme;
  const { token: theme } = useToken();

  const [zoomState, setZoomState] = useState({ start: 85, end: 100 }); // State to store the zoom range

  const chartRef = useRef(null);

  const protocol_chart = () => {
    useEffect(() => {
      const chartInstance = chartRef.current?.getEchartsInstance();
      if (chartInstance) {
        chartInstance.on("dataZoom", (params) => {
          const { start, end } = params.batch[0];
          setZoomState({ start, end });
        });
      }
      return () => {
        if (chartInstance) {
          chartInstance.off("dataZoom");
        }
      };
    }, []);

    // Convert UTC timestamp to local time (milliseconds)
    const adjustedTime = time.map((ts) => {
      const date = new Date(parseInt(ts));
      return date.getTime() - date.getTimezoneOffset() * 60000;
    });

    const hasData =
      protocolData?.protocol?.tcp?.attack?.bits?.some((v) => v > 0) ||
      protocolData?.protocol?.udp?.attack?.bits?.some((v) => v > 0) ||
      protocolData?.protocol?.icmp?.attack?.bits?.some((v) => v > 0) ||
      protocolData?.protocol?.dns?.attack?.bits?.some((v) => v > 0) ||
      protocolData?.protocol?.http?.attack?.bits?.some((v) => v > 0) ||
      protocolData?.protocol?.esp?.attack?.bits?.some((v) => v > 0) ||
      protocolData?.protocol?.unknown?.attack?.bits?.some((v) => v > 0);

    return {
      tooltip: {
        trigger: "axis",
        axisPointer: {
          type: "cross",
          animation: false,
          label: {
            backgroundColor: "#f2ebeb",
            borderColor: "#aaa",
            borderWidth: 1,
            shadowBlur: 0,
            shadowOffsetX: 0,
            shadowOffsetY: 0,
            color: "#222",
            formatter: function (params) {
              if (params.axisDimension === "x") {
                return timeFormatter(params.value);
              }
              if (params.axisDimension === "y") {
                return params.value.toFixed(2); // Ensure bitFormatter always returns a string
              }
            },
          },
        },
        formatter: function (params) {
          let result = `${timeFormatter(Number(params[0].name))}<br/>`; // Đảm bảo truyền vào là số UTC timestamp
          params.forEach((param) => {
            if (param.value !== 0) {
              result += `
                                <div style="display:flex; justify-content:space-between; min-width:200px; max-width:100%; gap:10px;">
                                    <span style="display:flex; align-items:center; white-space:nowrap;">
                                        <span style="width:10px; height:10px; border-radius:50%; background-color:${
                                          param.color
                                        }; margin-right:5px;"></span>
                                        ${param.seriesName}:
                                    </span>
                                    <span style="white-space:nowrap; font-weight: bold;">${bitFormatter(
                                      param.value
                                    )}</span>
                                </div>
                            `;
            }
          });
          return result;
        },
      },
      color: [
        theme.tcpColor,
        theme.udpColor,
        theme.icmpColor,
        theme.dnsColor,
        theme.httpColor,
        theme.espColor,
        theme.unknownColor,
      ],
      legend: {
        data: [
          "TCP drop",
          "TCP received",
          "UDP drop",
          "UDP received",
          "ICMP drop",
          "ICMP received",
          "DNS drop",
          "DNS received",
          "HTTP drop",
          "HTTP received",
          "ESP drop",
          "ESP received",
          "Unknown drop",
          "Unknown received",
        ],
        // show: col === 1,
        type: "scroll",
      },
      grid: {
        left: "3%",
        right: "3%",
        top: "15%",
        bottom: 0,
        containLabel: true,
      },
      xAxis: {
        type: "category",
        data: adjustedTime,
        boundaryGap: false,
        axisLabel: {
          formatter: (value) => timeFormatter(Number(value)),
        },
      },
      // yAxis: {
      //     type: 'value',
      //     axisLabel: {
      //         formatter: (value) => bitFormatter(value),
      //     },
      // },
      yAxis: {
        type: "value",
        axisLabel: {
          formatter: (value) => bitFormatter(value),
        },
      },
      dataZoom: [
        {
          type: "inside",
          start: zoomState.start, // Use zoomState start
          end: zoomState.end, // Use zoomState end
        },
        {
          start: zoomState.start, // Use zoomState start
          end: zoomState.end, // Use zoomState end
          show: paused, // Hide the dataZoom slider
        },
      ],
      series: [
        {
          name: "TCP drop",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.tcpColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.tcp.attack.bits,
        },
        {
          name: "UDP drop",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.udpColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.udp.attack.bits,
        },
        {
          name: "ICMP drop",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.icmpColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.icmp.attack.bits,
        },
        {
          name: "DNS drop",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.dnsColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.dns.attack.bits,
        },
        {
          name: "HTTP drop",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.httpColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.http.attack.bits,
        },
        {
          name: "ESP drop",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.espColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.esp.attack.bits,
        },
        {
          name: "Unknown drop",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.unknownColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.unknown.attack.bits,
        },
        {
          name: "TCP received",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1,
            type: "dashed",
            color: theme.tcpColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.tcp.bypass.bits,
        },
        {
          name: "UDP received",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1,
            type: "dashed",
            color: theme.udpColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.udp.bypass.bits,
        },
        {
          name: "ICMP received",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1,
            type: "dashed",
            color: theme.icmpColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.icmp.bypass.bits,
        },
        {
          name: "DNS received",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1,
            type: "dashed",
            color: theme.dnsColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.dns.bypass.bits,
        },
        {
          name: "HTTP received",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1,
            type: "dashed",
            color: theme.httpColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.http.bypass.bits,
        },
        {
          name: "ESP received",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1,
            type: "dashed",
            color: theme.espColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.esp.bypass.bits,
        },
        {
          name: "Unknown received",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1,
            type: "dashed",
            color: theme.unknownColor,
          },
          emphasis: {
            focus: "series",
          },
          data: protocolData?.protocol.unknown.bypass.bits,
        },
      ],
    };
  };

  return (
    <ReactEcharts
      ref={chartRef}
      option={protocol_chart()}
      style={{ width: "100%", height: "100%" }}
    />
  );
};

export default ProtocolChart;
