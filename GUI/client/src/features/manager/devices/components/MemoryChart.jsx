import React, { useEffect, useRef, useState } from "react";
import ReactEcharts from "echarts-for-react";
import * as echarts from "echarts";
import { bitFormatter, byteFormatter } from "@/lib/formatter";

const MemoryChart = ({ heapUsed, totalMemory, freeMemory }) => {
  const [chartOption, setChartOption] = useState({});
  const [timestamps, setTimestamps] = useState([]);
  const [memoryData, setMemoryData] = useState({ usage: [], free: [] });
  const [maxMemory, setMaxMemory] = useState(0);
  console.log("MemoryChart props:", { heapUsed, totalMemory, freeMemory });
  const dataRef = useRef({ heapUsed, totalMemory, freeMemory });

  useEffect(() => {
    dataRef.current = { heapUsed, totalMemory, freeMemory };
  }, [heapUsed, totalMemory, freeMemory]);

  const processData = ({ heapUsed, freeMemory, totalMemory }) => {
    const heap = parseFloat(heapUsed?.replace(" Gb", "")) || 0;
    const free = parseFloat(freeMemory?.replace(" Gb", "")) || 0;
    const total = parseFloat(totalMemory?.replace(" Gb", "")) || 0;
    console.log("Processing data:", { heap, free, total });
    setMaxMemory(total);

    const now = new Date().toTimeString().slice(0, 8);
    setTimestamps((prev) => [...prev.slice(-9), now]);
    setMemoryData((prev) => ({
      usage: [...prev.usage.slice(-9), heap],
      free: [...prev.free.slice(-9), free],
    }));
  };

  const areaChart = (timestamps, memoryData, maxMemory) => ({
    tooltip: {
      trigger: "axis",
      axisPointer: { type: "cross", label: { backgroundColor: "#6a7985" } },
      formatter: (params) => {
        return `
          ${params[0].axisValue}<br/>
          ${params
            .map((item) => {
              const bytes = item.data * 1024 * 1024;
              const [value, unit] = byteFormatter(bytes).split(" ");
              
              return `
                <span style="display:inline-block;margin-right:5px;border-radius:10px;width:10px;height:10px;background-color:${item.color};"></span>
                ${item.seriesName}
                <span style="font-weight:bold;float:right;margin-left:8px;">${value} ${unit}</span>
              `;
            })
            .join("<br/>")}
        `;
      },
    },
    grid: { left: "3%", right: "4%", bottom: "3%", containLabel: true },
    xAxis: { type: "category", data: timestamps, boundaryGap: false },
    yAxis: {
      type: "value",
      name: "Memory (Gb)",
      min: 0,
      max: maxMemory,
    },
    legend: { data: ["Heap Used", "Free Memory"] },
    series: [
      {
        name: "Heap Used",
        type: "line",
        stack: "Total",
        data: memoryData.usage,
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: "rgb(255, 51, 0)" },
            { offset: 1, color: "rgba(255, 51, 0, 0.5)" },
          ]),
        },
        emphasis: { focus: "series" },
      },
      {
        name: "Free Memory",
        type: "line",
        stack: "Total",
        data: memoryData.free,
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: "rgb(0, 255, 255)" },
            { offset: 1, color: "rgba(0, 255, 255, 0.4)" },
          ]),
        },
        emphasis: { focus: "series" },
      },
    ],
  });

  useEffect(() => {
    if (timestamps.length > 0 && memoryData.usage.length > 0) {
      setChartOption(areaChart(timestamps, memoryData, maxMemory));
    }
  }, [timestamps, memoryData, maxMemory]);

  useEffect(() => {
    const interval = setInterval(() => processData(dataRef.current), 1000);
    return () => clearInterval(interval);
  }, []);

  return (
    <ReactEcharts
      option={chartOption}
      style={{ width: "100%", height: "410px" }}
    />
  );
};

export default MemoryChart;
