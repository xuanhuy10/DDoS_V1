import React from 'react';
import { Card, Badge, Divider, Descriptions, Space, Button, Modal } from 'antd';
import create from '@ant-design/icons/lib/components/IconFont';

const SettingTemplates = ({setTemplate}) => {
    const createConfig = (key, label, children, span = 3) => ({ key, label, children, span });

    const tcpConfig = [
        createConfig('synflood', 'SYN Flood', <Badge {...({ status: 'success' })} text={'Enable'} />),
        createConfig('synfconf', 'SYN Flood settings', <>SYN Packets threshold: 1000<br />ACK Packets threshold: 1000<br />Whitelist duration: 30</>),
        createConfig('tcpfrag',  'TCP Fragmentation', <Badge {...({ status: 'success' })} text={'Enable'} />),
        createConfig('landatk',  'Land Attack', <Badge {...({ status: 'success' })} text={'Enable'} />),
    ];

    const udpConfig = [
        createConfig('udpflood', 'UDP Flood', <Badge {...({ status: 'success' })} text={'Enable'} />),
        createConfig('udpfconf', 'UDP Flood settings', <>UDP Packets threshold: 1000<br />UDP Packets threshold in 1 second: 1000</>),
        createConfig('udpfrag', 'UDP Fragmentation', <Badge {...({ status: 'success' })} text={'Enable'} />),
        createConfig('ipsec', 'IPSec IKE Flood', <Badge {...({ status: 'success' })} text={'Enable'} />),
        createConfig('udpfconf', 'IPSec IKE settings', <>IPSec IKE Packets threshold: 1000</>),
    ];

    const icmpConfig = [
        createConfig('icmpflood', 'ICMP Flood', <Badge {...({ status: 'success' })} text={'Enable'} />),
        createConfig('ICMPFconf', 'ICMP Flood settings', <>ICMP Packets threshold: 1000<br />ICMP Packets threshold in 1 second: 1000</>),
    ];

    const dnsConfig = [
        createConfig('dnsflood', 'DNS Flood', <Badge {...({ status: 'success' })} text={'Enable'} />),
        createConfig('dnsfconf', 'DNS Flood settings', <>DNS Packets threshold: 1000</>),
    ];

    const httpConfig = [
        createConfig('httpflood', 'HTTP Flood', <Badge {...({ status: 'success' })} text={'Enable'} />),
        createConfig('httpsflood', 'HTTPs Flood', <Badge {...({ status: 'success' })} text={'Enable'} />),
    ];
    
    const renderDescriptions = (title, items) => (
        <>
            <Divider variant="solid" style={{ borderColor: '#001529', margin: '0px 0' }}>{title}</Divider>
            <Descriptions bordered title="" size="small" items={items} />
        </>
    );

    return (
        <Card title="Default settings" extra={
            <Button size="small" onClick={() => {
                Modal.confirm({
                    title: 'Reset to default settings',
                    content: 'Are you sure you want to reset the current settings to default?, all of your current settings will be lost.',
                    onOk() {
                        setTemplate('default');
                    },
                });
            }}>
                Reset current setting to default
            </Button>
        }>
            <Space direction='vertical' size='small' style={{ width: '100%' }}>
                {renderDescriptions('TCP', tcpConfig)}
                {renderDescriptions('UDP', udpConfig)}
                {renderDescriptions('ICMP', icmpConfig)}
                {renderDescriptions('DNS', dnsConfig)}
                {renderDescriptions('HTTP / HTTPs', httpConfig)}
            </Space>
        </Card>
    );
};

export default SettingTemplates;