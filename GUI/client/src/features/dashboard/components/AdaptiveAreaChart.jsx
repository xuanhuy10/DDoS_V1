import React, { useEffect, useState } from "react";
import ReactECharts from "echarts-for-react";
import * as echarts from "echarts";

const AdaptiveAreaChart = ({
  data,
  label = "Usage",
  color = "#007bff",
  height = 100,
  width = 150,
}) => {
  const [chartData, setChartData] = useState([]);
  const [timestamps, setTimestamps] = useState([]);
  const [maxValue, setMaxValue] = useState(1); // Start with a default value

  // Parse the formatted data (e.g., "117.71 Kb" -> 117.71)
  const parseFormattedData = (value) => {
    if (typeof value === "number") return value;
    if (typeof value === "string") {
      const numericPart = parseFloat(value.replace(/[^\d.-]/g, ""));
      return isNaN(numericPart) ? 0 : numericPart;
    }
    return 0;
  };

  useEffect(() => {
    console.log(`${label} received data:`, data);

    if (data !== undefined && data !== null) {
      const now = new Date().toLocaleTimeString().slice(3, 8);

      setTimestamps((prev) => {
        const newTimestamps = [...prev, now].slice(-10);
        console.log(`${label} timestamps:`, newTimestamps);
        return newTimestamps;
      });

      const newValue = parseFormattedData(data);
      console.log(`${label} parsed value:`, newValue);

      setChartData((prev) => {
        const newData = [...prev, newValue].slice(-10);
        console.log(`${label} chart data:`, newData);

        const allZero = newData.every((val) => val === 0);
        // Improved max value calculation
        let newMax;
        if (allZero) {
          newMax = 0;
        } else {
          const currentMax = Math.max(...newData);
          // Add 20% padding to the max value, with a minimum of 10
          newMax = Math.max(Math.ceil(currentMax * 1.2), 1);
        }
        console.log(`${label} max value:`, newMax);
        setMaxValue(newMax);
        return newData;
      });
    }
  }, [data, label]);

  const option = {
    backgroundColor: "transparent",
    grid: {
      left: 5,
      right: 5,
      top: 5,
      bottom: 5,
      containLabel: false,
    },
    xAxis: {
      type: "category",
      boundaryGap: false,
      data: timestamps.length ? timestamps : ["00:00"],
      show: false,
    },
    yAxis: {
      type: "value",
      show: false,
      min: 0,
      max: maxValue,
    },
    series: [
      {
        name: label,
        data: chartData.length ? chartData : [0],
        type: "line",
        smooth: true,
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: `${color}CC` },
            { offset: 1, color: `${color}33` },
          ]),
          opacity: chartData.every((val) => val === 0) ? 0 : 1,
        },
        lineStyle: {
          color: color,
          width: chartData.every((val) => val === 0) ? 0 : 2,
        },
        symbol: "none",
      },
    ],
    animation: true, // Enable animation for smoother transitions
    animationDuration: 300, // Short animation duration
  };

  return (
    <ReactECharts
      option={option}
      style={{ height: height, width: width }}
      opts={{ renderer: "canvas" }}
      notMerge={true} // Force chart to re-render completely
    />
  );
};

export default AdaptiveAreaChart;
