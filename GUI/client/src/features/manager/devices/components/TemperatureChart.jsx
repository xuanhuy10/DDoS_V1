import React, { useEffect, useRef, useState } from "react";
import ReactEcharts from "echarts-for-react";
//import { color } from "html2canvas/dist/types/css/types/color";

const TemperatureChart = ({ value = 0, unit = "°C" }) => {
  const [timestamps, setTimestamps] = useState([]);
  const [tempData, setTempData] = useState([]);
  const [chartOption, setChartOption] = useState({});
  const [temperatureHistory, setTemperatureHistory] = useState([]);
  const latestValueRef = useRef(value);

  useEffect(() => {
    latestValueRef.current = value;
  }, [value]);

  // Update data every second
  useEffect(() => {
    const interval = setInterval(() => {
      const now = new Date().toLocaleTimeString();
      setTimestamps((prev) => [...prev.slice(-9), now]);
      setTempData((prev) => [...prev.slice(-9), latestValueRef.current]);
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    // Determine color based on the latest value
    const getColor = (val) => {
      if (val < 40) return "#00CCCC"; // Darker cyan for < 40
      if (val < 60) return "#CC5200"; // Darker orange for < 60
      return "#990000"; // Darker red for ≥ 60
    };

    const lineColor = getColor(value);
    const areaColor = `${lineColor}33`; // 20% opacity for area

    setChartOption({
      tooltip: {
        trigger: "axis",
        formatter: (params) =>
          `${params[0].axisValue}<br/>Temperature: <b>${params[0].data} ${unit}</b>`,
      },
      grid: { left: "3%", right: "4%", bottom: "3%", containLabel: true },
      xAxis: {
        type: "category",
        boundaryGap: false,
        data: timestamps,
      },
      yAxis: {
        type: "value",
        name: `Temperature (${unit})`,
        min: 0,
        max: 100,
        axisLabel: { formatter: (v) => `${v} ${unit}` },
        splitLine: { show: true, lineStyle: { color: "#e0e0e0", type: "dashed" } },
      },
      series: [
        {
          data: tempData,
          type: "line",
          areaStyle: {}, // Để mặc định như ví dụ
          showSymbol: true,
          symbolSize: 6,
          smooth: true,
          lineStyle: { width: 2 },
        },
      ],
    });
  }, [timestamps, tempData, unit, value]); // Added value as a dependency

  useEffect(() => {
    if (typeof value === "number") {
      setTemperatureHistory((prev) => [...prev.slice(-19), value]);
    }
  }, [value]);

  return (
    <ReactEcharts
      option={chartOption}
      style={{ width: "100%", height: "410px" }}
    />
  );
};

export default TemperatureChart;