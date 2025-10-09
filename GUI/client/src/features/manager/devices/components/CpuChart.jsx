import React, { useEffect, useState } from 'react';
import ReactEcharts from 'echarts-for-react';
import * as echarts from 'echarts';

const CpuChart = ({ data, maxCores = 4, maxDataPoints = 8 }) => {
  const [chartOption, setChartOption] = useState({});
  const [timeData, setTimeData] = useState([]);
  const [coreUsageHistory, setCoreUsageHistory] = useState({});

  const colorGradients = [
    { offset: 0, color: 'rgba(0, 90, 255, 0.8)' },   // Blue
    { offset: 1, color: 'rgba(173, 216, 230, 0.4)' }, // Light Blue
    { offset: 0, color: 'rgba(255, 165, 0, 0.8)' },   // Orange
    { offset: 1, color: 'rgba(255, 223, 186, 0.4)' }, // Light Orange
    { offset: 0, color: 'rgba(255, 69, 0, 0.8)' },    // Red
    { offset: 1, color: 'rgba(255, 160, 122, 0.4)' }, // Light Red
    { offset: 0, color: 'rgba(128, 0, 128, 0.8)' },   // Purple
    { offset: 1, color: 'rgba(221, 160, 221, 0.4)' }, // Light Purple
  ];

  // Process incoming data to update chart
  const updateData = (data) => {
    if (!data || !data.cpuUsage) return;

    const newTimestamp = new Date().toLocaleTimeString(); // Generate a new timestamp
    const newCoreUsage = {};

    // Filter and process CPU usage for the specified number of cores
    data.cpuUsage.slice(0, maxCores).forEach(({ core, usage }) => {
      const usageValue = parseFloat(usage.replace('%', '')) || 0;
      if (!newCoreUsage[core]) {
        newCoreUsage[core] = [];
      }
      newCoreUsage[core].push(usageValue);
    });

    // Update time data with a sliding window
    setTimeData((prevTime) => {
      const updatedTime = [...prevTime, newTimestamp];
      return updatedTime.length > maxDataPoints
        ? updatedTime.slice(-maxDataPoints)
        : updatedTime;
    });

    // Update core usage data with a sliding window
    setCoreUsageHistory((prevHistory) => {
      const updatedHistory = { ...prevHistory };
      Object.keys(newCoreUsage).forEach((core) => {
        if (!updatedHistory[core]) {
          updatedHistory[core] = [];
        }
        updatedHistory[core] = [...updatedHistory[core], ...newCoreUsage[core]];
        if (updatedHistory[core].length > maxDataPoints) {
          updatedHistory[core] = updatedHistory[core].slice(-maxDataPoints);
        }
      });
      return updatedHistory;
    });
  };

  // Function to generate chart options
  const generateChartOptions = (time, coreHistory) => {
    const series = Object.entries(coreHistory).map(([core, usage], index) => ({
      name: core,
      type: 'line',
      smooth: true,
      data: usage,
      areaStyle: {
        color: new echarts.graphic.LinearGradient(
          0,
          0,
          0,
          1,
          colorGradients.slice(index * 2, index * 2 + 2) // Dynamic gradient for each core
        ),
      },
      lineStyle: {
        width: 2,
        color: colorGradients[index * 2]?.color || 'rgba(0,0,0,0.8)',
      },
      emphasis: {
        focus: 'series',
      },
    }));

    return {
      tooltip: {
        trigger: 'axis',
        axisPointer: {
          type: 'cross',
          label: {
            backgroundColor: '#6a7985',
          },
        },
        formatter: (params) => {
          const tooltipContent = params
            .map(
              (item) =>
                `${item.marker} ${item.seriesName}: ${item.value.toFixed(2)}%`
            )
            .join('<br/>');
          return tooltipContent;
        },
      },
      grid: {
        left: '3%',
        right: '4%',
        bottom: '3%',
        containLabel: true,
      },
      xAxis: {
        type: 'category',
        data: time,
        boundaryGap: false,
      },
      yAxis: {
        type: 'value',
        name: 'CPU Usage (%)',
        max: 100, // Cap Y-axis to 100%
      },
      legend: {
        data: Object.keys(coreHistory),
        top: '0px',
      },
      series: series,
    };
  };

  // Update chart options when data changes
  useEffect(() => {
    if (data) {
      updateData(data); // Process new data
    }
  }, [data]);

  // Update the chart option whenever timeData or coreUsageHistory changes
  useEffect(() => {
    const options = generateChartOptions(timeData, coreUsageHistory);
    setChartOption(options);
  }, [timeData, coreUsageHistory]);

  return (
    <ReactEcharts
      option={chartOption}
      style={{ width: '100%', height: '350px' }}
    />
  );
};

export default CpuChart;
