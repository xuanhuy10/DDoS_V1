import { useState, useEffect } from 'react';

import { Card, Space, Flex, Typography, Divider, Row, Col } from "antd";
import { BarChartOutlined } from "@ant-design/icons";
const { Title } = Typography;

import ReactEcharts from 'echarts-for-react';

import { bitFormatter } from "@/lib/formatter";

import '@/features/dashboard/styles/main.css';

const BarChart = ({ sdata }) => {
    console.log('BarChart', sdata);
    
    const getOption = () => {
        const allZero = [
            sdata?.total?.bits?.received ?? 0,
            sdata?.tcp?.bits?.received ?? 0,
            sdata?.udp?.bits?.received ?? 0,
            sdata?.icmp?.bits?.received ?? 0,
            sdata?.http?.bits?.received ?? 0,
            sdata?.dns?.bits?.received ?? 0,
            sdata?.esp?.bits?.received ?? 0
        ].every(v => v === 0);

        // Helper function to ensure minimum visible value
        const ensureMinimumValue = (value, originalValue) => {
            if (originalValue === 0) return 0;
            const minVisibleValue = 0.01; // Minimum value to show on chart
            return Math.max(Math.abs(value), minVisibleValue) * (value < 0 ? -1 : 1);
        };

        // Get all data arrays
        const receivedData = [
            sdata?.total?.bits?.received ?? 0,
            sdata?.tcp?.bits?.received ?? 0,
            sdata?.udp?.bits?.received ?? 0,
            sdata?.icmp?.bits?.received ?? 0,
            sdata?.http?.bits?.received ?? 0,
            sdata?.dns?.bits?.received ?? 0,
            sdata?.esp?.bits?.received ?? 0
        ];

        const totalData = [
            sdata?.total?.bits?.total ?? 0,
            sdata?.tcp?.bits?.total ?? 0,
            sdata?.udp?.bits?.total ?? 0,
            sdata?.icmp?.bits?.total ?? 0,
            sdata?.http?.bits?.total ?? 0,
            sdata?.dns?.bits?.total ?? 0,
            sdata?.esp?.bits?.total ?? 0
        ];

        const dropData = [
            sdata?.total?.bits?.drop ?? 0,
            sdata?.tcp?.bits?.drop ?? 0,
            sdata?.udp?.bits?.drop ?? 0,
            sdata?.icmp?.bits?.drop ?? 0,
            sdata?.http?.bits?.drop ?? 0,
            sdata?.dns?.bits?.drop ?? 0,
            sdata?.esp?.bits?.drop ?? 0
        ];

        // Create display data with minimum values
        const receivedDisplayData = receivedData.map((val, idx) => ensureMinimumValue(val, val));
        const totalDisplayData = totalData.map((val, idx) => ensureMinimumValue(val, val));
        const dropDisplayData = dropData.map((val, idx) => ensureMinimumValue(val, val));

        return {
            tooltip: {
                trigger: 'axis',
                axisPointer: {
                    type: 'shadow'
                },
                formatter: function (params) {
                    return params.map((param, index) => {
                        let value = param.value;
                        let originalValue = value;
                        
                        // Get original value for display
                        if (param.seriesName === 'Received') {
                            originalValue = receivedData[param.dataIndex];
                        } else if (param.seriesName === 'Total') {
                            originalValue = totalData[param.dataIndex];
                        } else if (param.seriesName === 'Drop') {
                            originalValue = dropData[param.dataIndex];
                        }
                        
                        // Show positive values in tooltip for Drop series
                        if (param.seriesName === 'Drop') {
                            originalValue = Math.abs(originalValue);
                        }
                        
                        return `
                            <div style="display:flex; justify-content:space-between; width:200px;">
                                <span style="display:flex; align-items:center;">
                                    <span style="width:10px; height:10px; border-radius:50%; background-color:${param.color}; margin-right:5px;"></span>
                                    <strong>${param.seriesName}:</strong>
                                </span>
                                <span>${bitFormatter(originalValue)}</span>
                            </div>
                        `;
                    }).join('');
                }
            },
            legend: {
                data: ['Received', 'Drop', 'Total']
            },
            grid: {
                left: '3%',
                right: '4%',
                bottom: '3%',
                containLabel: true
            },
            xAxis: [
                {
                    type: 'value',
                    axisLabel: {
                        formatter: allZero
                            ? (value) => (Math.abs(value) * 100) + ' Kb'
                            : (value) => bitFormatter(Math.abs(value)) // Show positive values on axis
                    }
                }
            ],
            yAxis: [
                {
                    type: 'category',
                    axisTick: {
                        show: false
                    },
                    data: ['Total', 'TCP', 'UDP', 'ICMP', 'HTTP', 'DNS', 'ESP']
                }
            ],
            series: [
                {
                    name: 'Received',
                    type: 'bar',
                    label: {
                        position: 'inside',
                        formatter: (params) => {
                            const originalValue = receivedData[params.dataIndex];
                            return bitFormatter(originalValue);
                        }
                    },
                    emphasis: {
                        focus: 'series'
                    },
                    data: receivedDisplayData
                },
                {
                    name: 'Total',
                    type: 'bar',
                    stack: 'Total',
                    label: {
                        formatter: (params) => {
                            const originalValue = totalData[params.dataIndex];
                            return bitFormatter(originalValue);
                        }
                    },
                    emphasis: {
                        focus: 'series'
                    },
                    data: totalDisplayData
                },
                {
                    name: 'Drop',
                    type: 'bar',
                    stack: 'Total',
                    label: {
                        position: 'left',
                        formatter: (params) => {
                            const originalValue = dropData[params.dataIndex];
                            return bitFormatter(Math.abs(originalValue)); // Display positive value on label
                        }
                    },
                    emphasis: {
                        focus: 'series'
                    },
                    data: dropDisplayData
                }
            ]
        };
    };

    return (
        <Card bordered={false} className="db-chart-area">
            <Flex className="area-selector" align="middle">
                <Space align="center">
                    <BarChartOutlined style={{ fontSize: '1.3rem' }} />
                    <Title level={5}>Drop / Received</Title>
                </Space>
            </Flex>
            <Divider style={{ margin: '0.3rem 0', borderColor: '#BCBCBC' }} />
            <div className="chart-container">
                <Row gutter={[16, 16]}>
                    <Col
                        key='bar-chart'
                        xs={24}
                        sm={24}
                        md={24}
                        lg={24}
                        xl={24}
                        style={{ height: '100%' }}
                    >
                        <ReactEcharts
                            option={getOption()}
                            style={{ width: '100%', height: '100%' }}
                        />
                    </Col>
                </Row>
            </div>
        </Card>
    );
}

export default BarChart;    