import { Flex, Space, Button } from "antd";
import React, { useEffect, useRef } from "react";
import ReactEcharts from "echarts-for-react";

import { theme as antdTheme } from 'antd'

import { byteFormatter, bitFormatter, cntFormatter } from "@/lib/formatter";
import "@/features/dashboard/styles/main.css";



export const SummaryByteChart = ({ sdata, fdata }) => {
    const { useToken } = antdTheme
    const { token: theme } = useToken()

    const [chartType, setChartType] = React.useState('protocol');
    const chartRef = useRef(null);

    useEffect(() => {
        const timer = setTimeout(() => {
            if (chartRef.current) {
                chartRef.current.getEchartsInstance().resize();
            }
        }, 300); // Delay to allow Modal to fully open

        return () => clearTimeout(timer);
    }, []); // Only once on mount


    // Thêm log dữ liệu đầu vào
    console.log('SummaryByteChart sdata:', sdata);
    console.log('SummaryByteChart fdata:', fdata);

    const handleChartTypeChange = (type) => {
        setChartType(type);
        // Log khi đổi loại chart
        console.log('Chart type changed to:', type);
    };

    const ProtocolOption = {
        tooltip: {
            trigger: 'item',
            formatter: function (params) {
                return `
                    <strong>Total bits by Protocol</strong> <br/> 
                    <span style="display:inline-block;margin-right:5px;
                        width:10px;height:10px;
                        border-radius:50%;
                        background-color:${params.color};">
                    </span>
                    ${params.name} <strong>${bitFormatter(params.value)}<strong/> (${params.percent}%)
                `;
            }
        },
        color: [theme.tcpColor, theme.udpColor, theme.dnsColor, theme.icmpColor, theme.httpColor, theme.espColor, theme.unknownColor],
        legend: {
            data: ['TCP', 'UDP', 'DNS', 'ICMP', 'HTTP', 'ESP', 'Unknown'],
        },
        series: [
            {
                name: 'Total bits by Protocol', // <-- Sửa tên này
                type: 'pie',
                roseType: 'area',
                itemStyle: {
                    borderRadius: 5
                },
                selectedMode: 'single',
                radius: ['15%', '65%'],
                data: [
                    { value: (fdata?.summary.protocol.tcp.bypass.bits ?? 0) + (fdata?.summary.protocol.tcp.attack.bits ?? 0), name: 'TCP' },
                    { value: (fdata?.summary.protocol.udp.bypass.bits ?? 0) + (fdata?.summary.protocol.udp.attack.bits ?? 0), name: 'UDP' },
                    { value: (fdata?.summary.protocol.dns.bypass.bits ?? 0) + (fdata?.summary.protocol.dns.attack.bits ?? 0), name: 'DNS' },
                    { value: (fdata?.summary.protocol.icmp.bypass.bits ?? 0) + (fdata?.summary.protocol.icmp.attack.bits ?? 0), name: 'ICMP' },
                    { value: (fdata?.summary.protocol.http.bypass.bits ?? 0) + (fdata?.summary.protocol.http.attack.bits ?? 0), name: 'HTTP' },
                    { value: (fdata?.summary.protocol.esp.bypass.bits ?? 0) + (fdata?.summary.protocol.esp.attack.bits ?? 0), name: 'ESP' },
                    { value: (fdata?.summary.protocol.unknown.bypass.bits ?? 0) + (fdata?.summary.protocol.unknown.attack.bits ?? 0), name: 'Unknown' }
                ].filter(item => item.value > 0) // Remove items with zero value
            },
        ]
    };


    const AttackOption = {
        tooltip: {
            trigger: 'item',
            formatter: function (params) {
                return `
                    <strong>Total bits by Attack Type</strong> <br/> 
                    <span style="display:inline-block;margin-right:5px;
                        width:10px;height:10px;
                        border-radius:50%;
                        background-color:${params.color};">
                    </span>${params.name}: <strong>${bitFormatter(params.value)}<strong/> (${params.percent}%)
                `;
            }
        },
        color: [theme.synFloodColor, theme.udpFloodColor, theme.icmpFloodColor, theme.dnsFloodColor, theme.httpFloodColor, theme.ipsecColor, theme.tcpFragColor, theme.udpFragColor, theme.landColor, theme.normal],
        legend: {
            data: [
                'SYN Flood', 'UDP Flood', 'ICMP Flood', 'DNS Flood', 'HTTP Flood', 'IPSec IKE', 'TCP Fragment', 'UDP Fragment', 'Land Attack', 'Normal'
            ],
        },
        series: [
            {
                name: 'Total bits by Attack Type', // <-- Sửa tên này
                type: 'pie',
                roseType: 'area',
                itemStyle: {
                    borderRadius: 5
                },
                selectedMode: 'single',
                radius: ['15%', '60%'],

                data: [
                    { value: fdata?.summary.attack.synFlood.bits ?? 0, name: 'SYN Flood' },
                    { value: fdata?.summary.attack.udpFlood.bits ?? 0, name: 'UDP Flood' },
                    { value: fdata?.summary.attack.icmpFlood.bits ?? 0, name: 'ICMP Flood' },
                    { value: fdata?.summary.attack.dnsFlood.bits ?? 0, name: 'DNS Flood' },
                    { value: fdata?.summary.attack.httpFlood.bits ?? 0, name: 'HTTP Flood' },
                    { value: fdata?.summary.attack.ipsec.bits ?? 0, name: 'IPSec IKE' },
                    { value: fdata?.summary.attack.tcpFrag.bits ?? 0, name: 'TCP Fragment' },
                    { value: fdata?.summary.attack.udpFrag.bits ?? 0, name: 'UDP Fragment' },
                    { value: fdata?.summary.attack.land.bits ?? 0, name: 'Land Attack' },
                    { value: sdata?.total?.bits.received ?? 0, name: 'Normal' }
                ].filter(item => item.value > 0)
            },
        ]
    }


    return (
        <div className="summary-chart-container">
            
            <ReactEcharts
                ref={chartRef}
                option={chartType === 'protocol' ? ProtocolOption : AttackOption}
                style={{ height: '500px' }}
            />

            <Flex className="chart-type-buttons" align="middle" justify="center" gap={10}>
                <Button
                    color="cyan"
                    variant="solid"
                    data-type="protocol"
                    className={chartType === 'protocol' ? 'active-chart-btn' : ''}
                    onClick={() => handleChartTypeChange('protocol')}
                >
                    Protocol
                </Button>
                <Button
                    color="danger"
                    variant="solid"
                    data-type="attack"
                    className={chartType === 'attack' ? 'active-chart-btn' : ''}
                    onClick={() => handleChartTypeChange('attack')}
                >
                    Attack
                </Button>
            </Flex>
        </div>
    );
}

export const SummaryPacketChart = ({ sdata, fdata }) => {
    const [chartType, setChartType] = React.useState('protocol');

    const { useToken } = antdTheme
    const { token: theme } = useToken()

    const handleChartTypeChange = (type) => {
        setChartType(type);
    };
    const chartRef = useRef(null);

    useEffect(() => {
        const timer = setTimeout(() => {
            if (chartRef.current) {
                chartRef.current.getEchartsInstance().resize();
            }
        }, 300); // Delay to allow Modal to fully open

        return () => clearTimeout(timer);
    }, []); // Only once on mount


    const ProtocolOption = {
        tooltip: {
            trigger: 'item',
            formatter: function (params) {
                return `
                    <strong>${params.seriesName}</strong> <br/> 
                    <span style="display:inline-block;margin-right:5px;
                        width:10px;height:10px;
                        border-radius:50%;
                        background-color:${params.color};">
                    </span>
                    ${params.name} <strong>${cntFormatter(params.value)}<strong/> (${params.percent}%)
                `;
            }
        },
        color: [theme.tcpColor, theme.udpColor, theme.dnsColor, theme.icmpColor, theme.httpColor, theme.espColor, theme.unknownColor],
        legend: {
            data: [
                'TCP', 'UDP', 'DNS', 'ICMP', 'HTTP', 'ESP', 'Unknown'
            ],
        },
        series: [
            {
                name: 'Total Packets by Protocol',
                type: 'pie',
                roseType: 'area',
                itemStyle: {
                    borderRadius: 5
                },
                selectedMode: 'single',
                radius: ['15%', '65%'],

                data: [
                    { value: (fdata?.summary.protocol.tcp.bypass.packets ?? 0) + (fdata?.summary.protocol.tcp.attack.packets ?? 0), name: 'TCP' },
                    { value: (fdata?.summary.protocol.udp.bypass.packets ?? 0) + (fdata?.summary.protocol.udp.attack.packets ?? 0), name: 'UDP' },
                    { value: (fdata?.summary.protocol.dns.bypass.packets ?? 0) + (fdata?.summary.protocol.dns.attack.packets ?? 0), name: 'DNS' },
                    { value: (fdata?.summary.protocol.icmp.bypass.packets ?? 0) + (fdata?.summary.protocol.icmp.attack.packets ?? 0), name: 'ICMP' },
                    { value: (fdata?.summary.protocol.http.bypass.packets ?? 0) + (fdata?.summary.protocol.http.attack.packets ?? 0), name: 'HTTP' },
                    { value: (fdata?.summary.protocol.esp.bypass.packets ?? 0) + (fdata?.summary.protocol.esp.attack.packets ?? 0), name: 'ESP' },
                    { value: (fdata?.summary.protocol.unknown.bypass.packets ?? 0) + (fdata?.summary.protocol.unknown.attack.packets ?? 0), name: 'Unknown' }
                ].filter(item => item.value > 0)
            },
        ]
    };

    const AttackOption = {
        tooltip: {
            trigger: 'item',
            formatter: function (params) {
                return `
                    <strong>${params.seriesName}</strong> <br/> 
                    <span style="display:inline-block;margin-right:5px;
                        width:10px;height:10px;
                        border-radius:50%;
                        background-color:${params.color};">
                    </span>
                    ${params.name} <strong>${cntFormatter(params.value)}<strong/> (${params.percent}%)
                `;
            }
        },
        color: [theme.synFloodColor, theme.udpFloodColor, theme.icmpFloodColor, theme.dnsFloodColor, theme.httpFloodColor, theme.ipsecColor, theme.tcpFragColor, theme.udpFragColor, theme.landColor, theme.normal],
        legend: {
            data: [
                'SYN Flood', 'UDP Flood', 'ICMP Flood', 'HTTP Flood', 'DNS Flood', 'IPSec IKE', 'TCP Fragment', 'UDP Fragment', 'Land Attack', 'Normal'
            ],
        },
        series: [
            {
                name: 'Total Packets by Attack Type',
                type: 'pie',
                roseType: 'area',
                itemStyle: {
                    borderRadius: 5
                },
                selectedMode: 'single',
                radius: ['15%', '60%'],

                data: [
                    { value: fdata?.summary.attack.synFlood.packets ?? 0, name: 'SYN Flood' },
                    { value: fdata?.summary.attack.udpFlood.packets ?? 0, name: 'UDP Flood' },
                    { value: fdata?.summary.attack.icmpFlood.packets ?? 0, name: 'ICMP Flood' },
                    { value: fdata?.summary.attack.dnsFlood.packets ?? 0, name: 'DNS Flood' },
                    { value: fdata?.summary.attack.httpFlood.packets ?? 0, name: 'HTTP Flood' },
                    { value: fdata?.summary.attack.ipsec.packets ?? 0, name: 'IPSec IKE' },
                    { value: fdata?.summary.attack.tcpFrag.packets ?? 0, name: 'TCP Fragment' },
                    { value: fdata?.summary.attack.udpFrag.packets ?? 0, name: 'UDP Fragment' },
                    { value: fdata?.summary.attack.land.packets ?? 0, name: 'Land Attack' },
                    { value: sdata?.total?.packets.received ?? 0, name: 'Normal' }
                ].filter(item => item.value > 0)
            },
        ]
    }


    return (
        <div className="summary-chart-container">
            <ReactEcharts
                ref={chartRef}
                option={chartType === 'protocol' ? ProtocolOption : AttackOption}
                style={{ height: '500px' }}
            />
            <Flex className="chart-type-buttons" align="middle" justify="center" gap={10}>
                <Button
                    color="cyan"
                    variant="solid"
                    data-type="protocol"
                    className={chartType === 'protocol' ? 'active-chart-btn' : ''}
                    onClick={() => handleChartTypeChange('protocol')}
                >
                    Protocol
                </Button>
                <Button
                    color="danger"
                    variant="solid"
                    data-type="attack"
                    className={chartType === 'attack' ? 'active-chart-btn' : ''}
                    onClick={() => handleChartTypeChange('attack')}
                >
                    Attack
                </Button>
            </Flex>
        </div>
    );
}