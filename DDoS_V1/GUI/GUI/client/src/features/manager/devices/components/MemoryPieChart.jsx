import React, { useEffect, useState } from 'react';
import ReactEcharts from 'echarts-for-react';

const MemoryPieChart = ({ data }) => {
  const [chartOption, setChartOption] = useState({});

  // Process and set data for the pie chart
  const processData = (data) => {
    if (!data) return;

    const freeMemory = parseFloat(data?.memoryUsage?.freeMemory.replace(' MB', '')) || 0;
    const heapUsed = parseFloat(data?.memoryUsage?.heapUsed.replace(' MB', '')) || 0;
    const totalMemory = (data?.totalMemory) || 0;

    // Calculate memory stats

    // Update chart options
    setChartOption({
      tooltip: {
        trigger: 'item',
        formatter: '{a} <br/>{b}: {c} MB ({d}%)',
      },
      legend: {
        orient: 'vertical',
        left: 'left',
        data: ['Heap Used', 'Free Memory'],
      },
      series: [
        {
          name: 'Memory Usage',
          type: 'pie',
          radius: ['40%', '70%'], // Doughnut style
          avoidLabelOverlap: false,
          label: {
            show: false,
            position:'center',
            formatter: '{b}: {d}%', // Show percentage in chart labels
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
            { value: heapUsed.toFixed(2), name: 'Heap Used',itemStyle:{color:'rgb(255, 51, 0)'} },
            { value: freeMemory.toFixed(2), name: 'Free Memory',itemStyle:{color:'rgb(0, 255, 255)'} },
          ],
        },
      ],
    });
  };

  // Process the data when it changes
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

export default MemoryPieChart;
