import ReactEcharts from "echarts-for-react";
import { useEffect, useState, useRef } from "react";

import { theme as antdTheme } from "antd";

import { bitFormatter, timeFormatter } from "@/lib/formatter";

import "@/features/dashboard/styles/main.css";

const AttackChart = ({ attackData, time, col, paused }) => {
  const [zoomState, setZoomState] = useState({ start: 85, end: 100 }); // State to store the zoom range

  const { useToken } = antdTheme;
  const { token: theme } = useToken();

  const chartRef = useRef(null);
  console.log("attackData", attackData);

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
                return timeFormatter(Number(params.value)); // ép về số
              }

              if (params.axisDimension === "y") {
                return params.value.toFixed(2);
              }
            },
          },
        },
        formatter: function (params) {
          let result = `${timeFormatter(Number(params[0].name))}<br/>`; // ép về số
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
        theme.synFloodColor,
        theme.udpFloodColor,
        theme.icmpFloodColor,
        theme.dnsFloodColor,
        theme.httpFloodColor,
        theme.httpsFloodColor,
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
          "HTTPs Flood",
          "IPSec IKE Flood",
          "TCP Fragment",
          "UDP Fragment",
          "LAND Attack",
          "Normal",
        ],
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
          show: paused,
        },
      ],
      series: [
        {
          name: "SYN Flood",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.synFloodColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.synFlood.bits,
        },
        {
          name: "UDP Flood",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.udpFloodColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.udpFlood.bits,
        },
        {
          name: "ICMP Flood",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.icmpFloodColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.icmpFlood.bits,
        },
        {
          name: "DNS Flood",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.dnsFloodColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.dnsFlood.bits,
        },
        {
          name: "HTTP Flood",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.httpFloodColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.httpFlood.bits,
        },
        {
          name: "HTTPs Flood",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.httpsFloodColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.httpsFlood.bits,
        },
        {
          name: "IPSec IKE Flood",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.ipsecColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.ipsec.bits,
        },
        {
          name: "TCP Fragment",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.tcpFragColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.tcpFrag.bits,
        },
        {
          name: "UDP Fragment",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.udpFragColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.udpFrag.bits,
        },
        {
          name: "LAND Attack",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.landColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.attacks.land.bits,
        },
        {
          name: "Normal",
          type: "line",
          symbol: "none",
          sampling: "lttb",
          smooth: true,
          showSymbol: false,
          lineStyle: {
            width: 1.5,
            color: theme.bypassColor,
          },
          emphasis: {
            focus: "series",
          },
          data: attackData?.total.bypass.bits,
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

export default AttackChart;
