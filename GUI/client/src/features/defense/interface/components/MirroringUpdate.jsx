import React, {useState, useEffect, useRef} from 'react';
import PropTypes from 'prop-types';
import {
    Modal, Form, Input, Select, Button, message, Radio, Row, Col, Checkbox, Space, Progress,
} from 'antd';
import {
    RightOutlined, LeftOutlined, SaveOutlined,
    SettingOutlined, CheckCircleFilled, CloseCircleFilled
} from '@ant-design/icons';
import {getAllDeviceInterfaces, updateMirroringDeviceInterface} from '@/features/api/DeviceInterfaces';

export default function MirroringUpdate({isModalOpen, onCancel, mirrorInterfaceData, onUpdated}) {
    const [form] = Form.useForm();
    const [interfaces, setInterfaces] = useState([]);
    const [initialValues, setInitialValues] = useState({});
    const [isChanged, setIsChanged] = useState(false);
    const [showModal, setShowModal] = useState(false);
    const [estimatedTime, setEstimatedTime] = useState(0);
    const [timeLeft, setTimeLeft] = useState(0);
    const [totalTime, setTotalTime] = useState(0);
    const [apiStatus, setApiStatus] = useState("idle");
    const [errorMessage, setErrorMessage] = useState("");
    const countdownIntervalRef = useRef(null);

    useEffect(() => {
        const fetchInterfaces = async () => {
            try {
                const response = await getAllDeviceInterfaces();
                setInterfaces(response.data);
            } catch (error) {
                message.error('Failed to fetch interfaces.');
            }
        };
        fetchInterfaces();
    }, []);

    useEffect(() => {
        if (isModalOpen && mirrorInterfaceData) {
            let mirrorType = [];
            let valueObj = {};
            try {
                mirrorType = typeof mirrorInterfaceData.MirrorType === 'string'
                    ? JSON.parse(mirrorInterfaceData.MirrorType)
                    : mirrorInterfaceData.MirrorType || [];
            } catch {
                mirrorType = [];
            }
            try {
                valueObj = typeof mirrorInterfaceData.Value === 'string'
                    ? JSON.parse(mirrorInterfaceData.Value)
                    : mirrorInterfaceData.Value || {};
            } catch {
                valueObj = {};
            }
            const initVals = {
                InterfaceToMonitorInterfaceId: mirrorInterfaceData.MonitorInterfaceId,
                InterfaceId: mirrorInterfaceData.MirrorInterfaceId,
                MirrorSetting: mirrorInterfaceData.MirrorSetting,
                MirrorType: mirrorType,
                Direction: mirrorInterfaceData.Direction || mirrorInterfaceData.MirrorSetting || '',
                DestMac: valueObj.DestMac ?? '',
                SourceMac: valueObj.SourceMac ?? '',
                DestPort: valueObj.DestPort ?? '',
                SourcePort: valueObj.SourcePort ?? '',
                DestIPv4: valueObj.DestIPv4 ?? '',
                DestIPv6: valueObj.DestIPv6 ?? '',
                SourceIPv4: valueObj.SourceIPv4 ?? '',
                SourceIPv6: valueObj.SourceIPv6 ?? '',
                Protocol: valueObj.Protocol ?? '',
            };
            form.resetFields();
            form.setFieldsValue(initVals);
            setInitialValues(initVals);
            setIsChanged(false);
        }
    }, [mirrorInterfaceData, isModalOpen, form]);

    useEffect(() => {
        return () => {
            if (countdownIntervalRef.current) {
                clearInterval(countdownIntervalRef.current);
            }
        };
    }, []);

    const mirrorTypeValues = Form.useWatch('MirrorType', form);
    const destIPv4 = Form.useWatch('DestIPv4', form);
    const destIPv6 = Form.useWatch('DestIPv6', form);
    const sourceIPv4 = Form.useWatch('SourceIPv4', form);
    const sourceIPv6 = Form.useWatch('SourceIPv6', form);

    // Change detection
    const isDeepEqual = (a, b) => JSON.stringify(a) === JSON.stringify(b);
    const handleValuesChange = (_, allValues) => {
        setIsChanged(!isDeepEqual(allValues, initialValues));
    };

    const startCountdown = (estimatedTime) => {
        setTimeLeft(estimatedTime);
        setTotalTime(estimatedTime);
        setApiStatus("processing");

        if (countdownIntervalRef.current) {
            clearInterval(countdownIntervalRef.current);
        }

        countdownIntervalRef.current = setInterval(() => {
            setTimeLeft((prevTime) => {
                if (prevTime <= 1 && apiStatus === "processing") {
                    setTotalTime((prevTotal) => prevTotal + 5);
                    return 5;
                }
                return prevTime - 1;
            });
        }, 1000);
    };

    const handleSave = async () => {
        try {
            await form.validateFields();
            setShowModal(true);
            setApiStatus("processing");

            const values = await form.getFieldsValue();
            const mirrorType = values.MirrorType || [];

            const mirroringInterface = interfaces.find(i => i.InterfaceId === values.InterfaceId);
            if (mirroringInterface && mirroringInterface.InterfaceIsMirroring && values.InterfaceId !== mirrorInterfaceData.MirrorInterfaceId) {
                setApiStatus("error");
                setErrorMessage("This mirroring interface is already in use!");
                if (countdownIntervalRef.current) {
                    clearInterval(countdownIntervalRef.current);
                }
                return;
            }

            if (mirrorType.includes("Dest IP") && !values.DestIPv4 && !values.DestIPv6) {
                setApiStatus("error");
                setErrorMessage("Please provide at least one Destination IP (IPv4 or IPv6)!");
                if (countdownIntervalRef.current) {
                    clearInterval(countdownIntervalRef.current);
                }
                return;
            }
            if (mirrorType.includes("Source IP") && !values.SourceIPv4 && !values.SourceIPv6) {
                setApiStatus("error");
                setErrorMessage("Please provide at least one Source IP (IPv4 or IPv6)!");
                if (countdownIntervalRef.current) {
                    clearInterval(countdownIntervalRef.current);
                }
                return;
            }

            const valueObj = {};
            if (mirrorType.includes("Dest Mac")) valueObj.DestMac = values.DestMac;
            if (mirrorType.includes("Source Mac")) valueObj.SourceMac = values.SourceMac;
            if (mirrorType.includes("Dest Port")) valueObj.DestPort = values.DestPort;
            if (mirrorType.includes("Source Port")) valueObj.SourcePort = values.SourcePort;
            if (mirrorType.includes("Dest IP")) {
                valueObj.DestIPv4 = values.DestIPv4;
                valueObj.DestIPv6 = values.DestIPv6;
            }
            if (mirrorType.includes("Source IP")) {
                valueObj.SourceIPv4 = values.SourceIPv4;
                valueObj.SourceIPv6 = values.SourceIPv6;
            }
            if (mirrorType.includes("Protocol")) valueObj.Protocol = values.Protocol;

            const estimatedTime = 5;
            startCountdown(estimatedTime);

            const payload = {
                MonitorInterfaceId: values.InterfaceToMonitorInterfaceId,
                MirrorInterfaceId: values.InterfaceId,
                MirrorSetting: values.Direction,
                MirrorType: mirrorType,
                Value: JSON.stringify(valueObj),
            };

            const res = await updateMirroringDeviceInterface(values.InterfaceId, payload);

            if (countdownIntervalRef.current) {
                clearInterval(countdownIntervalRef.current);
            }

            if (res.status === 200) {
                setApiStatus("success");
                setTimeout(() => {
                    setShowModal(false);
                    if (onUpdated) onUpdated();
                    setIsChanged(false);
                    onCancel();
                }, estimatedTime * 1000);
            } else {
                setApiStatus("error");
                setErrorMessage(res.data?.message || "Unknown error occurred");
            }
        } catch (err) {
            if (countdownIntervalRef.current) {
                clearInterval(countdownIntervalRef.current);
            }
            setApiStatus("error");
            setErrorMessage(err?.message || "Update failed!");
        }
    };

    return (
        <>
            <Modal
                open={showModal}
                footer={null}
                closable={apiStatus !== "processing"}
                onCancel={() => {
                    if (countdownIntervalRef.current) {
                        clearInterval(countdownIntervalRef.current);
                    }
                    setShowModal(false);
                    setApiStatus("idle");
                }}
                centered
                maskClosable={apiStatus !== "processing"}
                width={400}
            >
                <div style={{textAlign: "center", padding: 20}}>
                    {apiStatus === "processing" && (
                        <>
                            <SettingOutlined style={{fontSize: 48, color: "#1890ff"}} spin/>
                            <p style={{marginTop: 16, fontSize: 18, fontWeight: "bold"}}>
                                Saving Mirroring configuration...
                            </p>
                            <p style={{fontSize: 16, color: "#888", margin: "10px 0"}}>
                                Time remaining: {timeLeft}s
                            </p>
                            <Progress
                                percent={Math.max(0, Math.min(100, ((totalTime - timeLeft) / totalTime) * 100))}
                                status="active"
                                showInfo={false}
                                strokeColor={{
                                    from: "#108ee9",
                                    to: "#87d068",
                                }}
                                style={{margin: "15px 0"}}
                            />
                        </>
                    )}
                    {apiStatus === "success" && (
                        <>
                            <CheckCircleFilled style={{fontSize: 48, color: "#52c41a"}}/>
                            <p style={{marginTop: 16, fontSize: 18, fontWeight: "bold"}}>
                                Configuration Saved Successfully!
                            </p>
                            <p style={{fontSize: 16, color: "#888", margin: "10px 0"}}>
                                Your settings have been applied successfully.
                            </p>
                        </>
                    )}
                    {apiStatus === "error" && (
                        <>
                            <CloseCircleFilled style={{fontSize: 48, color: "#ff4d4f"}}/>
                            <p style={{marginTop: 16, fontSize: 18, fontWeight: "bold"}}>
                                Failed to Save Configuration
                            </p>
                            <p style={{fontSize: 16, color: "#888", margin: "10px 0"}}>
                                {errorMessage}
                            </p>
                            <Button
                                type="primary"
                                onClick={() => window.location.reload()}
                                style={{marginTop: 15}}
                            >
                                Try Again
                            </Button>
                        </>
                    )}
                </div>
            </Modal>

            <Modal
                width={600}
                title="Update Mirroring Interface"
                open={isModalOpen}
                footer={null}
                onCancel={onCancel}
                centered
                maskClosable={false}
            >
                <Form
                    form={form}
                    layout="vertical"
                    onValuesChange={handleValuesChange}
                >
                    <Form.Item
                        name="InterfaceToMonitorInterfaceId"
                        label="Monitoring Interface"
                        rules={[{required: true, message: 'Please select the monitoring interface!'}]}
                    >
                        <Select disabled>
                            {interfaces.map((iface) => (
                                <Select.Option key={iface.InterfaceId} value={iface.InterfaceId}>
                                    {iface.InterfaceName}
                                </Select.Option>
                            ))}
                        </Select>
                    </Form.Item>
                    <Form.Item
                        name="InterfaceId"
                        label="Mirroring Interface"
                        rules={[{required: true, message: 'Please select the mirroring interface!'}]}
                    >
                        <Select disabled>
                            {interfaces.map((iface) => (
                                <Select.Option key={iface.InterfaceId} value={iface.InterfaceId}>
                                    {iface.InterfaceName}
                                </Select.Option>
                            ))}
                        </Select>
                    </Form.Item>
                    <Form.Item
                        label="Traffic Mirroring Direction"
                        name="Direction"
                        rules={[{required: true, message: 'Please select a direction!'}]}
                    >
                        <Radio.Group style={{width: '100%'}}>
                            <Radio.Button value="Ingress" style={{width: '33.33%', textAlign: 'center'}}>
                                Ingress
                            </Radio.Button>
                            <Radio.Button value="Egress" style={{width: '33.33%', textAlign: 'center'}}>
                                Egress
                            </Radio.Button>
                            <Radio.Button value="Ingress and Egress" style={{width: '33.33%', textAlign: 'center'}}>
                                Ingress and Egress
                            </Radio.Button>
                        </Radio.Group>
                    </Form.Item>
                    <Form.Item
                        label="Type Package"
                        name="MirrorType"
                        rules={[]}
                    >
                        <Checkbox.Group style={{width: '100%'}}>
                            <Row>
                                <Col span={8}><Checkbox value="Dest Mac">Dest Mac</Checkbox></Col>
                                <Col span={8}><Checkbox value="Source Mac">Source Mac</Checkbox></Col>
                                <Col span={8}><Checkbox value="Dest IP">Dest IP</Checkbox></Col>
                                <Col span={8}><Checkbox value="Source IP">Source IP</Checkbox></Col>
                                <Col span={8}><Checkbox value="Dest Port">Dest Port</Checkbox></Col>
                                <Col span={8}><Checkbox value="Source Port">Source Port</Checkbox></Col>
                                <Col span={8}><Checkbox value="Protocol">Protocol</Checkbox></Col>
                            </Row>
                        </Checkbox.Group>
                    </Form.Item>

                    {/* Hiển thị các trường nhập động */}
                    {mirrorTypeValues?.includes('Dest Mac') && (
                        <Form.Item
                            label="Destination MAC Address"
                            name="DestMac"
                            rules={[
                                {required: true, message: 'Please enter destination MAC address!'},
                                {
                                    pattern: /^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$/,
                                    message: 'Invalid MAC address format (e.g. 00:1A:2B:3C:4D:5E)'
                                }
                            ]}
                        >
                            <Input placeholder="Enter destination MAC address"/>
                        </Form.Item>
                    )}
                    {mirrorTypeValues?.includes('Source Mac') && (
                        <Form.Item
                            label="Source MAC Address"
                            name="SourceMac"
                            rules={[
                                {required: true, message: 'Please enter source MAC address!'},
                                {
                                    pattern: /^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$/,
                                    message: 'Invalid MAC address format (e.g. 00:1A:2B:3C:4D:5E)'
                                }
                            ]}
                        >
                            <Input placeholder="Enter source MAC address"/>
                        </Form.Item>
                    )}
                    {mirrorTypeValues?.includes('Dest Port') && (
                        <Form.Item
                            label="Destination Port"
                            name="DestPort"
                            rules={[
                                {required: true, message: 'Please enter destination port!'},
                                {
                                    validator: (_, value) => {
                                        if (!value) return Promise.resolve();
                                        if (!/^\d+$/.test(value)) return Promise.reject('Port must be a number between 1 and 65535');
                                        if (Number(value) < 1 || Number(value) > 65535) return Promise.reject('Port must be between 1 and 65535');
                                        return Promise.resolve();
                                    }
                                }
                            ]}
                        >
                            <Input placeholder="Enter destination port"/>
                        </Form.Item>
                    )}
                    {mirrorTypeValues?.includes('Source Port') && (
                        <Form.Item
                            label="Source Port"
                            name="SourcePort"
                            rules={[
                                {required: true, message: 'Please enter source port!'},
                                {
                                    validator: (_, value) => {
                                        if (!value) return Promise.resolve();
                                        if (!/^\d+$/.test(value)) return Promise.reject('Port must be a number between 1 and 65535');
                                        if (Number(value) < 1 || Number(value) > 65535) return Promise.reject('Port must be between 1 and 65535');
                                        return Promise.resolve();
                                    }
                                }
                            ]}
                        >
                            <Input placeholder="Enter source port"/>
                        </Form.Item>
                    )}
                    {mirrorTypeValues?.includes('Dest IP') && (
                        <Form.Item label="Destination IP" required>
                            <Input.Group compact>
                                <Form.Item
                                    name="DestIPv4"
                                    noStyle
                                    rules={[
                                        {
                                            pattern: /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$/,
                                            message: 'Invalid IPv4 address',
                                            required: false
                                        }
                                    ]}
                                >
                                    <Input
                                        style={{width: '50%'}}
                                        placeholder="IPv4"
                                        disabled={!!destIPv6}
                                    />
                                </Form.Item>
                                <Form.Item
                                    name="DestIPv6"
                                    noStyle
                                    rules={[
                                        {
                                            pattern: /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9]))$/,
                                            message: 'Invalid IPv6 address',
                                            required: false
                                        }
                                    ]}
                                >
                                    <Input
                                        style={{width: '50%'}}
                                        placeholder="IPv6"
                                        disabled={!!destIPv4}
                                    />
                                </Form.Item>
                            </Input.Group>
                        </Form.Item>
                    )}
                    {mirrorTypeValues?.includes('Source IP') && (
                        <Form.Item label="Source IP" required>
                            <Input.Group compact>
                                <Form.Item
                                    name="SourceIPv4"
                                    noStyle
                                    rules={[
                                        {
                                            pattern: /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$/,
                                            message: 'Invalid IPv4 address',
                                            required: false
                                        }
                                    ]}
                                >
                                    <Input
                                        style={{width: '50%'}}
                                        placeholder="IPv4"
                                        disabled={!!sourceIPv6}
                                    />
                                </Form.Item>
                                <Form.Item
                                    name="SourceIPv6"
                                    noStyle
                                    rules={[
                                        {
                                            pattern: /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9]))$/,
                                            message: 'Invalid IPv6 address',
                                            required: false
                                        }
                                    ]}
                                >
                                    <Input
                                        style={{width: '50%'}}
                                        placeholder="IPv6"
                                        disabled={!!sourceIPv4}
                                    />
                                </Form.Item>
                            </Input.Group>
                        </Form.Item>
                    )}
                    {mirrorTypeValues?.includes('Protocol') && (
                        <Form.Item
                            label="Protocol"
                            name="Protocol"
                            rules={[{required: true, message: 'Please select protocol!'}]}
                        >
                            <Select
                                options={[
                                    {label: 'TCP', value: 'TCP'},
                                    {label: 'UDP', value: 'UDP'},
                                    {label: 'ICMP', value: 'ICMP'},
                                ]}
                                placeholder="Select protocol"
                            />
                        </Form.Item>
                    )}

                    <div style={{textAlign: 'center', marginTop: 24}}>
                        <Button
                            style={{
                                marginRight: 8,
                                borderColor: 'red',
                                color: 'red'
                            }}
                            onClick={onCancel}
                        >
                            Cancel
                        </Button>
                        <Button
                            type="primary"
                            icon={<SaveOutlined/>}
                            onClick={handleSave}
                            disabled={!isChanged}
                        >
                            Update Configuration
                        </Button>
                    </div>
                </Form>
            </Modal>
        </>
    );
}

MirroringUpdate.propTypes = {
    isModalOpen: PropTypes.bool.isRequired,
    onCancel: PropTypes.func.isRequired,
    mirrorInterfaceData: PropTypes.object,
    onUpdated: PropTypes.func,
};