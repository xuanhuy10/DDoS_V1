import { useEffect, useState, useRef } from "react";
import {
  Card,
  Flex,
  Space,
  message,
  Form,
  Switch,
  InputNumber,
  Button,
  Alert,
  Divider,
  Modal,
} from "antd";
import { FieldTimeOutlined, SyncOutlined } from "@ant-design/icons";

import {
  getDefenseProfileAttackTypesConfig,
  updateDefenseProfile,
} from "@/features/api/DefenseProfiles";

import "@/features/dashboard/styles/main.css";

const AttackSettings = ({ attack_info, setThreshold }) => {
  const [form] = Form.useForm();
  const [isChanged, setIsChanged] = useState(false);

  const [settings, setSettings] = useState({});
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const isFetching = useRef(false);

  const fetchSettings = async () => {
    if (!attack_info?.type) {
      setError("Invalid attack type");
      setLoading(false);
      return;
    }
    if (isFetching.current) {
      console.log(
        `Fetch already in progress for attack type: ${attack_info.type}, skipping`
      );
      return;
    }
    isFetching.current = true;

    try {
      setLoading(true);
      console.log(`Fetching settings for attack type: ${attack_info.type}`);
      const response = await getDefenseProfileAttackTypesConfig(
        attack_info.type
      );
      console.log(`Fetched settings response:`, response.data);
      if (response.data) {
        setSettings(response.data);
        form.setFieldsValue(response.data);
        setIsChanged(false);
      } else {
        setError("No active defense profile found");
      }
    } catch (error) {
      console.error(`Error fetching settings:`, error.message);
      setError(error.message || "Failed to fetch settings");
    } finally {
      setLoading(false);
      isFetching.current = false;
      console.log(`Fetch settings process completed`);
    }
  };

  const handleValidate = (rule, value) => {
    console.log("User input:", value);

    // Check if the value is strictly a whole number (integer) with no commas or special characters
    const isValidInteger = /^\d+$/.test(value.toString());

    console.log("Is valid integer:", isValidInteger);

    if (!isValidInteger) {
      return Promise.reject(
        new Error(
          "Value must be a whole number without commas or special characters!"
        )
      );
    }

    return Promise.resolve();
  };

  const handleValuesChange = (_, allValues) => {
    const hasChanges = Object.keys(allValues).some(
      (key) => allValues[key] !== settings[key]
    );

    const allValid = Object.keys(allValues).every((key) => {
      const value = allValues[key];
      return typeof value !== "number" || Number.isInteger(value);
    });

    setIsChanged(hasChanges && allValid);
  };

  const getModifiedValues = (values) => {
    const modifiedValues = {};
    Object.keys(values).forEach((key) => {
      if (values[key] !== settings[key]) {
        modifiedValues[key] = values[key];
      }
    });
    return modifiedValues;
  };

  const handleFinish = (values) => {
    Modal.confirm({
      title: "Save changes",
      content: (
        <div>
          <p>Do you want to save the changes made to this attack settings?</p>
          <p>
            <strong style={{ color: "red" }}>Note:</strong> This profile is
            currently active. Changes will be applied to the current working
            mode.
          </p>
        </div>
      ),
      onOk: async () => {
        try {
          const modifiedValues = getModifiedValues(values);
          await updateDefenseProfile(settings.DefenseProfileId, modifiedValues);
          message.success("Profile updated successfully!");
        } catch (error) {
          message.error(
            error.response?.data?.message ||
              "Failed to update profile. Please try again."
          );
        }
      },
    });
  };

  useEffect(() => {
    console.log("useEffect triggered with attack_info:", attack_info);
    if (attack_info?.type) {
      fetchSettings();
    } else {
      console.warn("attack_info.type is undefined, skipping fetchSettings");
      setLoading(false);
      setError("Attack type is not specified");
    }
  }, [attack_info?.type, form]);

  return (
    <Card
      size="small"
      loading={loading}
      title={
        <Flex justify="space-between" align="center">
          <Space>
            <span style={{ fontSize: "0.9rem" }}>
              {attack_info.title}'s Settings
            </span>
          </Space>
          <Space>
            <SyncOutlined
              onClick={fetchSettings}
              style={{ cursor: "pointer" }}
            />
          </Space>
        </Flex>
      }
    >
      <Form
        layout="vertical"
        initialValues={settings}
        colon={false}
        onFinish={handleFinish}
        form={form}
        onValuesChange={handleValuesChange}
      >
        {error && (
          <Alert
            type="error"
            message={error}
            showIcon
            style={{ marginBottom: 16 }}
          />
        )}
        <Space>
          <span>Profile: </span>
          <span style={{ fontWeight: "bold" }}>
            {settings.DefenseProfileName}
          </span>
        </Space>
        <Form.Item
          label="Attack's Detection Time"
          name="DetectionTime"
          rules={[
            { required: true, message: "Please input the Detection Time!" },
            { validator: handleValidate },
          ]}
        >
          <InputNumber
            min={1}
            step={1}
            addonBefore={<FieldTimeOutlined />}
            addonAfter="seconds"
            style={{ width: "100%" }}
          />
        </Form.Item>
        <Divider style={{ margin: "8px 0" }} />
        {attack_info.type === "synFlood" && (
          <>
            <Form.Item
              name="SYNFloodEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>SYN Flood Detection</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>

            <Form.Item
              label="SYN Threshold"
              name="SYNFloodSYNThreshold"
              rules={[
                {
                  required: true,
                  message: "Please input the SYN Flood SYN Threshold!",
                },
                { validator: handleValidate },
              ]}
            >
              <InputNumber
                min={1}
                max={65535}
                step={1}
                addonBefore={<FieldTimeOutlined />}
                addonAfter="packets/s"
                style={{ width: "100%" }}
              />
            </Form.Item>

            <Form.Item
              label="ACK Threshold"
              name="SYNFloodACKThreshold"
              rules={[
                {
                  required: true,
                  message: "Please input the SYN Flood ACK Threshold!",
                },
                { validator: handleValidate },
              ]}
            >
              <InputNumber
                min={1}
                max={65535}
                step={1}
                addonBefore={<FieldTimeOutlined />}
                addonAfter="packets/s"
                style={{ width: "100%" }}
              />
            </Form.Item>
          </>
        )}
        {attack_info.type === "udpFlood" && (
          <>
            <Form.Item
              name="UDPFloodEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>UDP Flood Detection</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
            <Form.Item
              label="UDP Flood Threshold"
              name="UDPFloodThreshold"
              rules={[
                {
                  required: true,
                  message: "Please input the UDP Flood Threshold!",
                },
                { validator: handleValidate },
              ]}
            >
              <InputNumber
                min={1}
                max={65535}
                step={1}
                addonBefore={<FieldTimeOutlined />}
                addonAfter="packets/s"
                style={{ width: "100%" }}
              />
            </Form.Item>
            <Form.Item
              label="UDP Flood Rate"
              name="UDPFloodRate"
              rules={[
                { required: true, message: "Please input the UDP Flood Rate!" },
                { validator: handleValidate },
              ]}
            >
              <InputNumber
                min={1}
                max={65535}
                step={1}
                addonBefore={<FieldTimeOutlined />}
                addonAfter="packets/s"
                style={{ width: "100%" }}
              />
            </Form.Item>
          </>
        )}
        {attack_info.type === "icmpFlood" && (
          <>
            <Form.Item
              name="ICMPFloodEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>ICMP Flood Detection</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
            <Form.Item
              label="ICMP Flood Threshold"
              name="ICMPFloodThreshold"
              rules={[
                {
                  required: true,
                  message: "Please input the ICMP Flood Threshold!",
                },
                { validator: handleValidate },
              ]}
            >
              <InputNumber
                min={1}
                max={65535}
                step={1}
                addonBefore={<FieldTimeOutlined />}
                addonAfter="packets/s"
                style={{ width: "100%" }}
              />
            </Form.Item>
            <Form.Item
              label="ICMP Flood Threshold per second"
              name="ICMPFloodRate"
              rules={[
                {
                  required: true,
                  message: "Please input the ICMP Flood Threshold per second!",
                },
                { validator: handleValidate },
              ]}
            >
              <InputNumber
                min={1}
                max={65535}
                step={1}
                addonBefore={<FieldTimeOutlined />}
                addonAfter="packets/s"
                style={{ width: "100%" }}
              />
            </Form.Item>
          </>
        )}
        {attack_info.type === "dnsFlood" && (
          <>
            <Form.Item
              name="DNSFloodEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>DNS Flood Protection</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
            <Form.Item
              label="DNS Flood Threshold"
              name="DNSFloodThreshold"
              rules={[
                {
                  required: true,
                  message: "Please input the DNS Flood Threshold!",
                },
                { validator: handleValidate },
              ]}
            >
              <InputNumber
                min={1}
                max={65535}
                step={1}
                addonBefore={<FieldTimeOutlined />}
                addonAfter="packets/s"
                style={{ width: "100%" }}
              />
            </Form.Item>
          </>
        )}
        {attack_info.type === "httpFlood" && (
          <>
            <Form.Item
              name="HTTPFloodEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>HTTP Flood Protection</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
          </>
        )}
        {attack_info.type === "land" && (
          <>
            <Form.Item
              name="LandAttackEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>LAND Attack Protection</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
          </>
        )}
        {attack_info.type === "ipsec" && (
          <>
            <Form.Item
              name="IPSecIKEEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>IPSEC IKE Flood</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
            <Form.Item
              label="IPSec IKE Threshold"
              name="IPSecIKEThreshold"
              rules={[
                {
                  required: true,
                  message: "Please input the IPSec IKE Threshold!",
                },
                { validator: handleValidate },
              ]}
            >
              <InputNumber
                min={1}
                max={65535}
                step={1}
                addonBefore={<FieldTimeOutlined />}
                addonAfter="packets/s"
                style={{ width: "100%" }}
              />
            </Form.Item>
          </>
        )}
        {attack_info.type === "tcpFrag" && (
          <>
            <Form.Item
              name="TCPFragmentEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>TCP Fragment Flood</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
          </>
        )}
        {attack_info.type === "udpFrag" && (
          <>
            <Form.Item
              name="UDPFragmentEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>UDP Fragment Flood</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
          </>
        )}
        {attack_info.type === "httpsFlood" && (
          <>
            <Form.Item
              name="HTTPSFloodEnable"
              getValueProps={(value) => ({ checked: value == 1 })}
              getValueFromEvent={(value) => (value ? 1 : 0)}
              label={<strong>HTTPs Flood Protection</strong>}
              layout="horizontal"
              style={{ marginBottom: 8, marginTop: 0 }}
              labelAlign="left"
              labelCol={{ sm: { span: 22 } }}
              wrapperCol={{ sm: { span: 2 } }}
            >
              <Switch style={{ float: "right" }} size="small" />
            </Form.Item>
          </>
        )}
        <Button
          type="primary"
          htmlType="submit"
          style={{ width: "100%" }}
          disabled={!isChanged}
        >
          Save changes
        </Button>
      </Form>
    </Card>
  );
};

export default AttackSettings;
