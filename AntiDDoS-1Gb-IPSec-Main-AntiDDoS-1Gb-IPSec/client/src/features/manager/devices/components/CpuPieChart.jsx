import React, { useEffect, useState } from 'react';
import ReactEcharts from 'echarts-for-react';

const CpuPieChart = ({ data }) => {
  const [chartOption, setChartOption] = useState({});

  const processData = (data) => {
    if (!data || !data.nCPUs || !data.nCPUs.cpuPercentage) return;

    // Extract the CPU usage value and remove the "%" symbol
    const cpuUsageString = data.nCPUs.cpuPercentage || "0%";
    const cpu1Usage = parseFloat(cpuUsageString.replace('%', '')) || 0; // Convert to a numeric value

    // Calculate the remaining (idle) CPU usage
    const idleCpuUsage = Math.max(0, 100 - cpu1Usage);

    // Update the chart options
    setChartOption({
      tooltip: {
        trigger: 'item',
        formatter: '{a} <br/>{b}: {c}% ({d}%)',
      },
      legend: {
        orient: 'vertical',
        left: 'left',
        data: ['Used CPU', 'Idle CPU'],
      },
      series: [
        {
          name: 'CPU Utilization',
          type: 'pie',
          radius: ['40%', '70%'], // Doughnut style
          avoidLabelOverlap: false,
          label: {
            show: false,
            position:'center',
            formatter: '{b}: {d}%', // Show percentage in the chart
          },
          emphasis: {
            label: {
              show: true,
              fontSize: '16',
              fontWeight: 'bold',
            },
          },
          labelLine: {
            show: true,
          },
          data: [
            {
              value: cpu1Usage.toFixed(2), // Used CPU
              name: 'Used CPU',
              itemStyle: { color: 'rgb(255, 51, 0)' }, // Light Blue
            },
            {
              value: idleCpuUsage.toFixed(2), // Idle CPU
              name: 'Idle CPU',
              itemStyle: { color: 'rgb(0, 255, 255)' }, // Orange
            },
          ],
        },
      ],
    });
  };

  useEffect(() => {
    if (data) {
      processData(data);
    }
  }, [data]);

  return (
    <ReactEcharts
      option={chartOption}
      style={{ width: '100%', height: '300px' }}
    />
  );
};

export default CpuPieChart;
