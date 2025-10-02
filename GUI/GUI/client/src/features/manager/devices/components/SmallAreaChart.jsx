import React, { useEffect, useState } from "react";
import ReactECharts from "echarts-for-react";
import * as echarts from "echarts";

const SmallAreaChart = ({ data }) => {
  // const [chartData, setChartData] = useState([10, 20, 30, 40, 50, 60, 70, 80, 90, 100]);
  // const [timestamps, setTimestamps] = useState(["00:00", "00:01", "00:02", "00:03", "00:04", "00:05", "00:06", "00:07", "00:08", "00:09"]);
  console.log("SmallAreaChart data:", data);
  const [chartData, setChartData] = useState([]);
  const [timestamps, setTimestamps] = useState([]);

 useEffect(() => {
  if (typeof data === 'number' && !isNaN(data)) {
    const now = new Date().toLocaleTimeString().slice(3, 8);
    setTimestamps((prev) => [...prev, now].slice(-10));
    const newValue = Math.floor(data);
    setChartData((prev) => [...prev, newValue].slice(-10));
  } else {
    console.warn("Invalid data received in SmallAreaChart:", data);
  }
}, [data]);

useEffect(() => {
  console.log("useEffect triggered with data:", data);
  if (typeof data === 'number' && !isNaN(data)) {
    const now = new Date().toLocaleTimeString().slice(3, 8);
    setTimestamps((prev) => [...prev, now].slice(-10));
    const newValue = Math.floor(data);
    setChartData((prev) => [...prev, newValue].slice(-10));
  }
}, [data]);
  
  
  
 const option = {
  backgroundColor: "#f2f2f2",
  grid: { left: 0, right: 0, top: 5, bottom: 5 },
  xAxis: {
    type: "category",
    boundaryGap: false,
    data: timestamps.length ? timestamps : ["00:00"], // Fallback for empty timestamps
    show: false,
  },
  yAxis: {
    type: "value",
    show: false,
    min: 0,
    max: 100,
  },
  series: [
    {
      name: "Usage",
      data: chartData.length ? chartData : [0], // Fallback for empty chartData
      type: "line",
      smooth: true,
      areaStyle: {
        color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
          { offset: 0, color: "rgba(0, 123, 255, 0.8)" },
          { offset: 1, color: "rgba(0, 123, 255, 0.2)" },
        ]),
      },
      lineStyle: { color: "#007bff", width: 1.5 },
      symbol: "none",
    },
  ],
};
  

  return <ReactECharts option={option} style={{ height: 100, width: 150 }} />;
};

export default SmallAreaChart;
