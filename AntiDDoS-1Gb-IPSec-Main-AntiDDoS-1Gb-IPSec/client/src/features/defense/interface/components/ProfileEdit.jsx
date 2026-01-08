import { useEffect, useState } from 'react';
import { Button, Form, Divider, message, Badge } from 'antd';
import { Flex, Drawer, Collapse, Input, InputNumber, Switch, Radio, theme, Space, Modal } from 'antd';
import { ClockCircleOutlined, CheckOutlined, CloseOutlined, SaveOutlined, CaretRightOutlined } from '@ant-design/icons';
import { updateDefenseProfile } from '@/features/api/DefenseProfiles';
import '@/features/defense/interface/styles/main.css';

const EditDrawer = ({ openDrawer, setOpenDrawer, editData, onUpdateSuccess }) => {
    const [form] = Form.useForm();
    const { token } = theme.useToken();
    const [isChanged, setIsChanged] = useState(false);
    const [initialValues, setInitialValues] = useState({});

    const panelStyle = {
        marginBottom: 12,
        borderRadius: token.borderRadiusLG,
        border: '1px solid ' + token.colorBorder,
        boxShadow: token.shadowLG,
    };

    useEffect(() => {
        if (openDrawer && editData) {
            // Reset form and set fields with editData to ensure fresh state
            form.resetFields();
            form.setFieldsValue(editData);
            setInitialValues(editData);
            setIsChanged(false);
        }
    }, [editData, openDrawer, form]);

    const handleValuesChange = (_, allValues) => {
        const hasChanges = Object.keys(allValues).some(
            (key) => allValues[key] !== initialValues[key]
        );
        setIsChanged(hasChanges);
    };

    const getModifiedValues = (values) => {
        const modifiedValues = {};
        Object.keys(values).forEach((key) => {
            if (values[key] !== initialValues[key]) {
                modifiedValues[key] = values[key];
            }
        });
        return modifiedValues;
    };

    const handleFinish = async (values) => {
        Modal.confirm({
            title: 'Save changes',
            content: (
                <div>
                    <p>Do you want to save the changes made to this setting profile?</p>
                    {editData?.DefenseProfileStatus === 'Active' && (
                        <p>
                            <strong style={{ color: 'red' }}>Note:</strong> This profile is currently active. Changes will be applied to the current working mode.
                        </p>
                    )}
                </div>
            ),
            onOk: async () => {
                try {
                    const modifiedValues = getModifiedValues(values);
                    const response = await updateDefenseProfile(editData.DefenseProfileId, modifiedValues);
                    message.success('Profile updated successfully!');
                    // Notify parent component with updated data
                    if (onUpdateSuccess) {
                        onUpdateSuccess({ ...editData, ...modifiedValues });
                    }
                    // Optional: Uncomment the line below to force a page reload
                    // window.location.reload();
                    setOpenDrawer(false);
                } catch (error) {
                    message.error(error.response?.data?.message || 'Failed to update profile. Please try again later.');
                }
            },
        });
    };

    return (
        <Drawer
            title={
                <Flex align="center" justify='space-between' gap='small'>
                    <span>
                        Edit Profile: <strong>{editData?.Name || editData?.DefenseProfileName}</strong>
                    </span>
                    <Space>
                        <Badge status={editData?.DefenseProfileStatus === 'Active' ? 'processing' : 'default'} />
                    </Space>
                </Flex>
            }
            placement="right"
            closable={true}
            onClose={() => setOpenDrawer(false)}
            open={openDrawer}
            width={500}
            className='drawer'
        >
            <Form
                layout='vertical'
                colon={false}
                requiredMark={false}
                form={form}
                onFinish={handleFinish}
                onValuesChange={handleValuesChange}
            >
                <Form.Item
                    name="DefenseProfileName"
                    label={<span className='collapse-title'>Profile Name</span>}
                    rules={[{ required: true, message: 'Please set your profile name!' }]}
                >
                    <Input />
                </Form.Item>
                <Form.Item
                    name="DefenseProfileDescription"
                    label={<span className='collapse-title'>Profile Description</span>}
                    rules={[{ required: true, message: 'Please give a description for your profile!' }]}
                >
                    <Input.TextArea />
                </Form.Item>
                <Flex vertical gap="middle">
                    <Collapse
                        style={{ backgroundColor: 'white' }}
                        bordered={false}
                        expandIconPosition='end'
                        expandIcon={({ isActive }) => <CaretRightOutlined rotate={isActive ? 90 : 0} />}
                        size='small'
                        items={[
                            {
                                key: 'general',
                                label: <span className='collapse-title'>General Settings</span>,
                                style: panelStyle,
                                children: (
                                    <>
                                        <Form.Item
                                            label="Attack detection times"
                                            name="DetectionTime"
                                            layout='horizontal'
                                            rules={[{ required: true, message: 'Please set attack detection times!' }]}
                                        >
                                            <InputNumber min={1} max={20} addonAfter="s" style={{ width: '100%' }} prefix={<ClockCircleOutlined />} />
                                        </Form.Item>
                                        <Form.Item
                                            label="Defense mode"
                                            name="DefenseMode"
                                            layout='horizontal'
                                            rules={[{ required: true, message: 'Please choose defense mode!' }]}
                                        >
                                            <Radio.Group>
                                                <Radio value="aggregate">Aggregate</Radio>
                                                <Radio value="classified">Classified</Radio>
                                            </Radio.Group>
                                        </Form.Item>
                                    </>
                                ),
                            },
                            {
                                key: 'TCP',
                                label: <span className='collapse-title'>TCP Attacks</span>,
                                style: panelStyle,
                                children: (
                                    <>
                                        <Form.Item
                                            label={<strong>SYN Flood</strong>}
                                            name="SYNFloodEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            rules={[{ required: true }]}
                                            layout="horizontal"
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size="small"
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                        <Form.Item
                                            label="SYN Threshold"
                                            name="SYNFloodSYNThreshold"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set SYN Threshold!' }]}
                                        >
                                            <InputNumber min={1} max={65535} addonAfter="packets/s" style={{ width: '100%' }} />
                                        </Form.Item>
                                        <Form.Item
                                            label="ACK Threshold"
                                            name="SYNFloodACKThreshold"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set ACK Threshold!' }]}
                                        >
                                            <InputNumber min={1} max={65535} addonAfter="packets/s" style={{ width: '100%' }} />
                                        </Form.Item>
                                        <Form.Item
                                            label="Timeout"
                                            name="SYNFloodWhiteListTimeOut"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set Timeout!' }]}
                                        >
                                            <InputNumber min={1} max={200} addonAfter="s" style={{ width: '100%' }} prefix={<ClockCircleOutlined />} />
                                        </Form.Item>
                                        <Divider />
                                        <Form.Item
                                            rules={[{ required: true }]}
                                            label={<strong>TCP Fragment</strong>}
                                            name="TCPFragmentEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                        <Divider />
                                        <Form.Item
                                            rules={[{ required: true }]}
                                            label={<strong>Land Attack</strong>}
                                            name="LandAttackEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                    </>
                                )
                            },
                            {
                                key: 'UDP',
                                label: <span className='collapse-title'>UDP Attacks</span>,
                                style: panelStyle,
                                children: (
                                    <>
                                        <Form.Item
                                            rules={[{ required: true }]}
                                            label={<strong>UDP Flood</strong>}
                                            name="UDPFloodEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                        <Form.Item
                                            label="UDP Threshold"
                                            name="UDPFloodThreshold"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set UDP Threshold!' }]}
                                        >
                                            <InputNumber min={1} max={65535} addonAfter="packets/s" style={{ width: '100%' }} />
                                        </Form.Item>
                                        <Form.Item
                                            label="UDP Threshold Rate"
                                            name="UDPFloodRate"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set UDP Threshold!' }]}
                                        >
                                            <InputNumber min={1} max={65535} addonAfter="packets/s" style={{ width: '100%' }} />
                                        </Form.Item>
                                        <Divider />
                                        <Form.Item
                                            rules={[{ required: true }]}
                                            label={<strong>UDP Fragment</strong>}
                                            name="UDPFragmentEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                        <Divider />
                                        <Form.Item
                                            rules={[{ required: true }]}
                                            label={<strong>IPSEC IKE Flood</strong>}
                                            name="IPSecIKEEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                        <Form.Item
                                            label="IPSEC IKE Threshold"
                                            name="IPSecIKEThreshold"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set IPSEC Threshold!' }]}
                                        >
                                            <InputNumber min={1} max={65535} addonAfter="packets/s" style={{ width: '100%' }} />
                                        </Form.Item>
                                    </>
                                )
                            },
                            {
                                key: 'ICMP',
                                label: <span className='collapse-title'>ICMP Attacks</span>,
                                style: panelStyle,
                                children: (
                                    <>
                                        <Form.Item
                                            rules={[{ required: true }]}
                                            label={<strong>ICMP Flood</strong>}
                                            name="ICMPFloodEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                        <Form.Item
                                            label="ICMP Threshold"
                                            name="ICMPFloodThreshold"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set ICMP Threshold!' }]}
                                        >
                                            <InputNumber min={1} max={65535} addonAfter="packets/s" style={{ width: '100%' }} />
                                        </Form.Item>
                                        <Form.Item
                                            label="ICMP Threshold Rate"
                                            name="ICMPFloodRate"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set ICMP Threshold!' }]}
                                        >
                                            <InputNumber min={1} max={65535} addonAfter="packets/s" style={{ width: '100%' }} />
                                        </Form.Item>
                                    </>
                                )
                            },
                            {
                                key: 'DNS',
                                label: <span className='collapse-title'>DNS Attacks</span>,
                                style: panelStyle,
                                children: (
                                    <>
                                        <Form.Item
                                            rules={[{ required: true }]}
                                            label={<strong>DNS Flood</strong>}
                                            name="DNSFloodEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                        <Form.Item
                                            label="DNS Threshold"
                                            name="DNSFloodThreshold"
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 12 } }}
                                            wrapperCol={{ sm: { span: 12 } }}
                                            rules={[{ required: true, message: 'Please set DNS Threshold!' }]}
                                        >
                                            <InputNumber min={1} max={65535} addonAfter="packets/s" style={{ width: '100%' }} />
                                        </Form.Item>
                                    </>
                                )
                            },
                            {
                                key: 'HTTP',
                                label: <span className='collapse-title'>HTTP/HTTPs Attacks</span>,
                                style: panelStyle,
                                children: (
                                    <>
                                        <Form.Item
                                            label={<strong>HTTP Flood</strong>}
                                            name="HTTPFloodEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                        <Divider />
                                        <Form.Item
                                            label={<strong>HTTPs Flood</strong>}
                                            name="HTTPSFloodEnable"
                                            getValueProps={(value) => ({ checked: value === 1 })}
                                            getValueFromEvent={(value) => (value ? 1 : 0)}
                                            layout='horizontal'
                                            style={{ marginBottom: 5 }}
                                            labelAlign="left"
                                            labelCol={{ sm: { span: 21 } }}
                                            wrapperCol={{ sm: { span: 3 } }}
                                        >
                                            <Switch
                                                style={{ float: 'right' }}
                                                size='small'
                                                checkedChildren={<CheckOutlined />}
                                                unCheckedChildren={<CloseOutlined />}
                                            />
                                        </Form.Item>
                                    </>
                                ),
                            },
                        ]}
                    />
                    {editData?.DefenseProfileType !== 'SystemProfile' && (
                        <Flex align="center" style={{ width: '100%' }} justify="end" gap={10}>
                            <Button
                                type="default"
                                icon={<CloseOutlined />}
                                onClick={() => setOpenDrawer(false)}
                                variant="solid"
                                color="red"
                            >
                                Cancel
                            </Button>
                            <Button
                                type="primary"
                                htmlType="submit"
                                icon={<SaveOutlined />}
                                variant="solid"
                                color="green"
                                disabled={!isChanged}
                            >
                                Save changes
                            </Button>
                        </Flex>
                    )}
                </Flex>
            </Form>
        </Drawer>
    );
};

export default EditDrawer;