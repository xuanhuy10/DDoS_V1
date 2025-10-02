import React from 'react';
import ReactEcharts from 'echarts-for-react';

const CpuBarChart = ({ data, maxCores = 4 }) => {
  if (!data || !data.cpuUsage) return null;

  // Process CPU usage data and limit to `maxCores`
  const coreUsage = data.cpuUsage.slice(0, maxCores).map((core) => {
    const usageValue = parseFloat(core.usage.replace('%', '')) || 0;

    // Dynamically assign colors based on usage
    let color = 'rgb(0, 255, 255)'; // Low usage
    if (usageValue > 50) color = 'rgb(255, 100, 0)'; // Medium usage
    if (usageValue > 80) color = 'rgb(200, 0, 0)'; // High usage

    return {
      value: usageValue,
      itemStyle: { color }, // Dynamically assign color
      name: core.core,
    };
  });

  const totalUsage = coreUsage.reduce((sum, { value }) => sum + value, 0);
  const coreCount = coreUsage.length || 1;
  const averageUsage = totalUsage / coreCount;

  // Dynamically set max Y-axis value (scale up if usage > 100%)
  const maxYAxis = Math.max(100, Math.ceil(averageUsage));

  const option = {
    tooltip: {
      trigger: 'axis',
      formatter: (params) => {
        let result = `${params[0].axisValue}<br/>`;
        params.forEach((item) => {
          result += `${item.marker} ${item.seriesName}: ${item.value.toFixed(2)}%<br/>`;
        });
        return result;
      },
      axisPointer: {
        type: 'shadow',
      },
    },
    legend: {
      data: ['CPU Usage'],
    },
    grid: {
      left: '3%',
      right: '4%',
      bottom: '3%',
      containLabel: true,
    },
    xAxis: {
      type: 'category',
      data: coreUsage.map((_, index) => `Core ${index + 1}`),
      axisTick: {
        alignWithLabel: true,
      },
    },
    yAxis: {
      type: 'value',
      name: 'Usage (%)',
      max: maxYAxis, // Dynamic max based on the data
      axisLine: {
        show: true,
      },
    },
    series: [
      {
        name: 'CPU Usage',
        type: 'bar',
        data: coreUsage, // Pass dynamically colored data
        barWidth: '50%',
      },
    ],
  };

  return <ReactEcharts option={option} style={{ marginTop: '10px', width: '100%', height: '400px' }} />;
};

export default CpuBarChart;
