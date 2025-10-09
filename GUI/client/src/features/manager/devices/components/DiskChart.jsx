import React, { useEffect, useRef, useState } from 'react';
import ReactEcharts from 'echarts-for-react';
import { bitFormatter } from '@/lib/formatter';
import * as echarts from 'echarts';

const DiskChart = ({ diskUsed, diskFree, diskTotal, diskUsedPercentage }) => {
  const [chartOption, setChartOption] = useState({});
  const [timestamps, setTimestamps] = useState([]);
  const [diskData, setDiskData] = useState({ used: [], free: [] });
  const [usedPercentageData, setUsedPercentageData] = useState([]);

  const latestDataRef = useRef({ diskUsed, diskFree, diskTotal, diskUsedPercentage });

  useEffect(() => {
    latestDataRef.current = { diskUsed, diskFree, diskTotal, diskUsedPercentage };
  }, [diskUsed, diskFree, diskTotal, diskUsedPercentage]);

  const processData = () => {
    const { diskUsed, diskFree, diskTotal, diskUsedPercentage } = latestDataRef.current;

    const now = new Date().toTimeString().slice(0, 8);
    setTimestamps((prev) => [...prev.slice(-9), now]);
    setDiskData((prev) => ({
      used: [...prev.used.slice(-9), diskUsed],
      free: [...prev.free.slice(-9), diskFree],
    }));
    setUsedPercentageData((prev) => [...prev.slice(-9), diskUsedPercentage]);
  };

  const areaChart = (timestamps, diskData, diskTotal, usedPercentageData) => ({
    tooltip: {
      trigger: 'axis',
      formatter: (params) => {
        return `
          ${params[0].axisValue}<br/>
          ${params
            .map((item, index) => {
              const percentValue = usedPercentageData[index];
              let percent = '';
              if (typeof percentValue === 'number' && !isNaN(percentValue)) {
                percent =
                  item.seriesName === 'Used'
                    ? percentValue.toFixed(2) + '%'
                    : (100 - percentValue).toFixed(2) + '%';
              }
              const [value, unit] = bitFormatter(item.data).split(' ');
              return `
                <span style="display:inline-block;margin-right:5px;border-radius:10px;width:10px;height:10px;background-color:${item.color};"></span>
                ${item.seriesName}
                <span style="font-weight:bold;float:right;margin-left:8px;">${value} ${unit}${percent ? ' (' + percent + ')' : ''}</span>
              `;
            })
            .join('<br/>')}
        `;
      },
    },
    grid: { left: '3%', right: '4%', bottom: '3%', containLabel: true },
    xAxis: { type: 'category', data: timestamps, boundaryGap: false },
    yAxis: {
      type: 'value',
      name: 'Disk Storage',
      min: 0,
      max: diskTotal,
      axisLabel: {
        formatter: (value) => bitFormatter(value),
      },
    },
    legend: { data: ['Used', 'Free'] },
    series: [
      {
        name: 'Used',
        type: 'line',
        stack: 'Total',
        data: diskData.used,
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgb(255, 51, 0)' },
            { offset: 1, color: 'rgba(255, 51, 0, 0.5)' },
          ]),
        },
      },
      {
        name: 'Free',
        type: 'line',
        stack: 'Total',
        data: diskData.free,
        areaStyle: {
          color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
            { offset: 0, color: 'rgb(0, 255, 255)' },
            { offset: 1, color: 'rgba(0, 255, 255, 0.4)' },
          ]),
        },
      },
    ],
  });

  useEffect(() => {
    if (
      timestamps.length > 0 &&
      diskData.used.length > 0 &&
      usedPercentageData.length > 0
    ) {
      setChartOption(
        areaChart(timestamps, diskData, diskTotal, usedPercentageData)
      );
    }
  }, [timestamps, diskData, diskTotal, usedPercentageData]);

  useEffect(() => {
    const interval = setInterval(() => processData(), 1000);
    return () => clearInterval(interval);
  }, []);

  return (
    <ReactEcharts
      option={chartOption}
      style={{ width: '100%', height: '410px' }}
    />
  );
};

export default DiskChart;
