import { Flex, Space, Form, InputNumber, Switch, Typography, Input, Radio, Divider } from 'antd';
import { CheckOutlined, CloseOutlined } from '@ant-design/icons';
import ClockCircleOutlined from '@ant-design/icons/ClockCircleOutlined';

const { Title } = Typography;
import '@/features/defense/defenseprofile/styles/main.css';


const ConfigPane = ({ name, title, children }) => (
    <div className='config-panes'>
        <Space align="middle" gap="small">
            <Form.Item
                name={name}
                rules={[{ required: true, message: `Please set ${title.toLowerCase()}!` }]}
            >
                <Switch
                    checkedChildren="Enabled"
                    unCheckedChildren="Disabled"
                />
            </Form.Item>
            <Title level={4}>{title}</Title>
        </Space>
        {children}
    </div>
);

const ThresholdInput = ({ label, name, min = 1, max = 65535, prefix }) => (
    <Form.Item
        label={<Title level={5}>{label}</Title>}
        name={name}
        rules={[{ required: true, message: `Please set ${label.toLowerCase()}!` }]}
    >
        <InputNumber
            min={min}
            max={max}
            style={{ width: '100%' }}
            prefix={prefix}
        />
    </Form.Item>
);

const SettingGeneral = () => (
    <div className='config-panes'>
        <Form.Item
            label={<Title level={4}>Profile name</Title>}
            name="DefenseProfileName"
            rules={[{ required: true, message: 'Please set your profile name!' }]}>
            <Input />
        </Form.Item>

        <Form.Item
            label={<Title level={5}>Profile Description</Title>}
            name="DefenseProfileDescription"
            rules={[{ required: true, message: 'Please give a description for your profile!' }]}>
            <Input.TextArea />
        </Form.Item>

        <Divider variant="solid" style={{ borderColor: '#001529' }}>General Settings</Divider>

        <Form.Item
            label={<Title level={5}>Attack detection times</Title>}
            name="DetectionTime"
            rules={[{ required: true, message: 'Please set attack detection times!' }]}>
            <InputNumber min={1} max={20} style={{ width: '100%' }} prefix={<ClockCircleOutlined />} />
        </Form.Item>

        <Form.Item
            label={<Title level={5}>Defense mode</Title>}
            name="DefenseMode"
            rules={[{ required: true, message: 'Please choose defense mode!' }]}>
            <Radio.Group>
                <Radio value="aggregate">Aggregate</Radio>
                <Radio value="classified">Classified</Radio>
            </Radio.Group>
        </Form.Item>
    </div>
);

const SettingTCP = () => (
    <Flex vertical gap="middle">
        <ConfigPane name="SYNFloodEnable" title="SYN Flood">
            <ThresholdInput label="SYN packets threshold in detection times (packets/detection times)" name="SYNFloodSYNThreshold" />
            <ThresholdInput label="ACK packets threshold in detection times (packets/detection times)" name="SYNFloodACKThreshold" />
            <ThresholdInput label="On Whitelist duration" name="SYNFloodWhiteListTimeOut" min={1} max={200} prefix={<ClockCircleOutlined />} />
        </ConfigPane>
        <ConfigPane name="TCPFragmentEnable" title="TCP Fragmentation Attack" />
        <ConfigPane name="LandAttackEnable" title="Land Attack" />
    </Flex>
);

const SettingUDP = () => (
    <Flex vertical gap="middle">
        <ConfigPane name="UDPFloodEnable" title="UDP Flood">
            <ThresholdInput label="UDP packets threshold in detection times (packets/detection times)" name="UDPFloodThreshold" />
            <ThresholdInput label="UDP packets threshold in 1 second (packets/s)" name="UDPFloodRate" />
        </ConfigPane>
        <ConfigPane name="UDPFragmentEnable" title="UDP Fragmentation Attack" />
        <ConfigPane name="IPSecIKEEnable" title="IPSEC IKE Flood">
            <ThresholdInput label="IPSec IKE packets threshold in detection times (packets/detection times)" name="IPSecIKEThreshold" />
        </ConfigPane>
    </Flex>
);

const SettingICMP = () => (
    <Flex vertical gap="middle">
        <ConfigPane name="ICMPFloodEnable" title="ICMP Flood">
            <ThresholdInput label="ICMP packets threshold in detection times (packets/detection times)" name="ICMPFloodThreshold" />
            <ThresholdInput label="ICMP packets threshold in 1 second (packets/s)" name="ICMPFloodRate" />
        </ConfigPane>
    </Flex>
);

const SettingDNS = () => (
    <Flex vertical gap="middle">
        <ConfigPane name="DNSFloodEnable" title="DNS Flood">
            <ThresholdInput label="DNS packets threshold in detection times (packets/detection times)" name="DNSFloodThreshold" />
        </ConfigPane>
    </Flex>
);

const SettingHTTP = () => (
    <Flex vertical gap="middle">
        <ConfigPane name="HTTPFloodEnable" title="HTTP Flood" />
        <ConfigPane name="HTTPSFloodEnable" title="HTTPs Flood" />
    </Flex>
);

const SettingNetwork = () => (
    <Flex vertical gap="middle">
        <ConfigPane name="ICMPFloodEnable" title="ICMP Flood">
            <ThresholdInput label="ICMP packets threshold in detection times (packets/detection times)" name="ICMPFloodThreshold" />
            <ThresholdInput label="ICMP packets threshold in 1 second (packets/s)" name="ICMPFloodRate" />
        </ConfigPane>
        <ConfigPane name="TCPFragmentEnable" title="TCP Fragmentation Attack" />
        <ConfigPane name="UDPFragmentEnable" title="UDP Fragmentation Attack" />
    </Flex>
);

const SettingTransport = () => (
    <Flex vertical gap="middle">

        <ConfigPane name="SYNFloodEnable" title="SYN Flood">
            <ThresholdInput label="SYN packets threshold in detection times (packets/detection times)" name="SYNFloodSYNThreshold" />
            <ThresholdInput label="ACK packets threshold in detection times (packets/detection times)" name="SYNFloodACKThreshold" />
            <ThresholdInput label="On Whitelist duration" name="SYNFloodWhiteListTimeOut" min={1} max={200} prefix={<ClockCircleOutlined />} />
        </ConfigPane>
        <ConfigPane name="UDPFloodEnable" title="UDP Flood">
            <ThresholdInput label="UDP packets threshold in detection times (packets/detection times)" name="UDPFloodThreshold" />
            <ThresholdInput label="UDP packets threshold in 1 second (packets/s)" name="UDPFloodRate" />
        </ConfigPane>
        <ConfigPane name="DNSFloodEnable" title="DNS Flood">
            <ThresholdInput label="DNS packets threshold in detection times (packets/detection times)" name="DNSFloodThreshold" />
        </ConfigPane>
        <ConfigPane name="LandAttackEnable" title="Land Attack" />
    </Flex>
);

const SettingSession = () => (
    <Flex vertical gap="middle">
        <ConfigPane name="IPSecIKEEnable" title="IPSEC IKE Flood">
            <ThresholdInput label="IPSec IKE packets threshold in detection times (packets/detection times)" name="IPSecIKEThreshold" />
        </ConfigPane>
    </Flex>
);

const SettingApplication = () => (
    <Flex vertical gap="middle">
        <ConfigPane name="HTTPFloodEnable" title="HTTP Flood" />
        <ConfigPane name="HTTPSFloodEnable" title="HTTPs Flood" />
    </Flex>
);

export { SettingGeneral, SettingICMP, SettingTCP, SettingUDP, SettingDNS, SettingHTTP, SettingNetwork, SettingTransport, SettingSession, SettingApplication };
