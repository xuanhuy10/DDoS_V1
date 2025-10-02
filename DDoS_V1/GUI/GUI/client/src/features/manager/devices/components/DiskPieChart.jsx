import React, { useEffect, useState } from 'react';
import ReactEcharts from 'echarts-for-react';
// import { displayBytes } from '@/utils/displayBytes';
import { byteFormatter } from '@/lib/formatter';

const DiskPieChart = ({ data }) => {
  const [chartOption, setChartOption] = useState({});

  const processData = (data) => {
    if (!data) return;

    const free = data.diskUsage.free;
    const used = data.diskUsage.used;

    setChartOption({
      tooltip: {
        trigger: 'item',
        formatter: (params) =>
          `${params.seriesName}<br>${params.name}: ${byteFormatter(params.value)} (${params.percent}%)`,
      },
      legend: { orient: 'vertical', left: 'left', data: ['Used', 'Free'] },
      series: [
        {
          name: 'Disk Usage',
          type: 'pie',
          radius: ['40%', '70%'],
          data: [
            { value: used, name: 'Used', itemStyle: { color: 'rgb(255, 51, 0)' } },
            { value: free, name: 'Free', itemStyle: { color: 'rgb(0, 255, 255)' } },
          ],
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
        },
        
      ],
    });
  };

  useEffect(() => {
    processData(data);
  }, [data]);

  return <ReactEcharts option={chartOption} style={{ width: '100%', height: '300px' }} />;
};

export default DiskPieChart;
