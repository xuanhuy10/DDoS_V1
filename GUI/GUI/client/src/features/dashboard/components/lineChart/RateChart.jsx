import * as echarts from 'echarts';
import ReactEcharts from 'echarts-for-react';
import { useEffect, useState, useRef } from 'react';

import { bitFormatter, timeFormatter } from "@/lib/formatter";

import '@/features/dashboard/styles/main.css';

const RateChart = ({ rateData, time, col, paused }) => {
    const [zoomState, setZoomState] = useState({ start: 85, end: 100 });
    const chartRef = useRef(null);

    // ✅ Chuyển timestamp sang local time để hiển thị đúng
    const adjustedTime = time.map(ts => {
        const t = new Date(parseInt(ts));
        return t.getTime() - t.getTimezoneOffset() * 60000;
    });

    const bypass_attack_chart = () => {
        useEffect(() => {
            const chartInstance = chartRef.current?.getEchartsInstance();
            if (chartInstance) {
                chartInstance.on('dataZoom', (params) => {
                    const { start, end } = params.batch[0];
                    setZoomState({ start, end });
                });
            }
            return () => {
                if (chartInstance) {
                    chartInstance.off('dataZoom');
                }
            };
        }, []);
        
        return {
            tooltip: {
                trigger: 'axis',
                axisPointer: {
                    type: 'cross',
                    animation: false,
                    label: {
                        backgroundColor: '#f2ebeb',
                        borderColor: '#aaa',
                        borderWidth: 1,
                        shadowBlur: 0,
                        shadowOffsetX: 0,
                        shadowOffsetY: 0,
                        color: '#222',
                        formatter: function (params) {
                            if (params.axisDimension === 'x') {
                                return timeFormatter(params.value);
                            } 
                            if (params.axisDimension === 'y') {
                                return params.value.toFixed(2);
                            }
                        },
                    },
                },
                formatter: function (params) {
                    let result = `${timeFormatter(params[0].name)}<br/>`;
                    params.forEach((param) => {
                        if (param.value !== 0) {
                            result += `
                                <div style="display:flex; justify-content:space-between; min-width:150px; max-width:100%; gap:10px;">
                                    <span style="display:flex; align-items:center; white-space:nowrap;">
                                        <span style="width:10px; height:10px; border-radius:50%; background-color:${param.color}; margin-right:5px;"></span>
                                        ${param.seriesName}:
                                    </span>
                                    <span style="white-space:nowrap; font-weight: bold;">${bitFormatter(param.value)}</span>
                                </div>
                            `;
                        }
                    });
                    return result;
                },
            },
            color: ['#4d77ff', '#ff4683'],
            legend: {
                data: ['Bypass', 'Attack'],
            },
            grid: {
                left: '0.5%',
                right: '0.5%',
                top: '10%',
                bottom: '5%',
                containLabel: true,
            },
            xAxis: {
                type: 'category',
                data: adjustedTime, // ✅ Đã chỉnh
                boundaryGap: false,
                axisLabel: {
                    formatter: timeFormatter
                }
            },
            yAxis: {
                type: 'value',
                axisLabel: {
                    formatter: (value) => bitFormatter(value),
                },
            },
            dataZoom: [
                {
                    type: 'inside',
                    start: zoomState.start,
                    end: zoomState.end,
                },
                {
                    start: zoomState.start,
                    end: zoomState.end,
                    show: paused,
                },
            ],
            series: [
                {
                    name: 'Bypass',
                    type: 'line',
                    stack: 'Total',
                    symbol: 'none',
                    smooth: true,
                    showSymbol: false,
                    areaStyle: {
                        opacity: 0.5,
                        color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                            {
                                offset: 0.4,
                                color: '#0143DD',
                            },
                            {
                                offset: 1,
                                color: '#00ddff',
                            },
                        ]),
                    },
                    lineStyle: {
                        width: 0,
                        color: '#0143DD',
                    },
                    emphasis: {
                        focus: 'series',
                    },
                    data: rateData?.total.bypass.bits,
                },
                {
                    name: 'Attack',
                    type: 'line',
                    stack: 'Total',
                    symbol: 'none',
                    smooth: true,
                    showSymbol: false,
                    areaStyle: {
                        opacity: 0.6,
                        color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                            {
                                offset: 0.4,
                                color: '#DD0101',
                            },
                            {
                                offset: 1,
                                color: '#ff9e44',
                            },
                        ]),
                    },
                    lineStyle: {
                        width: 0,
                        color: '#DD0101',
                    },
                    emphasis: {
                        focus: 'series',
                    },
                    data: rateData?.total.attack.bits,
                },
            ],
        };
    };

    return (
        <ReactEcharts
            ref={chartRef}
            option={bypass_attack_chart()}
            style={{ width: '100%', height: '100%' }}
        />
    );
};

export default RateChart;
