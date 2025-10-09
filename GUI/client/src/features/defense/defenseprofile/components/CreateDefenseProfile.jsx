import React, { useState, useEffect, useRef } from "react";
import {
  Row,
  Col,
  Badge,
  Divider,
  Descriptions,
  Space,
  Tabs,
  Form,
  Typography,
  Button,
  Modal,
} from "antd";
import { SaveOutlined, SettingOutlined } from "@ant-design/icons";

import "@/features/defense/defenseprofile/styles/main.css";

import {
  SettingGeneral,
  SettingNetwork,
  SettingTransport,
  SettingSession,
  SettingApplication,
} from "@/features/defense/defenseprofile/components/Protocol";
import SettingTemplates from "@/features/defense/defenseprofile/components/SettingTemplates";

import {
  insertDefenseProfile,
} from "@/features/api/DefenseProfiles";

const CreateDefenseProfile = () => {
  const [form] = Form.useForm();
  const [isNameOrDescriptionFilled, setIsNameOrDescriptionFilled] =
    useState(false);
  const [countdownActive, setCountdownActive] = useState(false);
  const [estimatedTime, setEstimatedTime] = useState(42);
  const [isModalOpen, setIsModalOpen] = useState(false);
  const countdownRef = useRef();

  const defaultTemplate = {
    DefenseProfileName: "",
    DefenseProfileDescription: "",
    DetectionTime: 1,
    DefenseMode: "Aggregate",
    SYNFloodEnable: true,
    SYNFloodSYNThreshold: 1000,
    SYNFloodACKThreshold: 1000,
    SYNFloodWhiteListTimeOut: 30,
    TCPFragmentEnable: true,
    LandAttackEnable: true,
    UDPFloodEnable: true,
    UDPFloodThreshold: 1000,
    UDPFloodRate: 1000,
    UDPFragmentEnable: true,
    IPSecIKEEnable: true,
    IPSecIKEThreshold: 1000,
    ICMPFloodEnable: true,
    ICMPFloodThreshold: 1000,
    ICMPFloodRate: 1000,
    DNSFloodEnable: true,
    DNSFloodThreshold: 1000,
    HTTPFloodEnable: true,
    HTTPSFloodEnable: true,
  };

  const name = Form.useWatch("DefenseProfileName", form);
  const description = Form.useWatch("DefenseProfileDescription", form);

  useEffect(() => {
    setIsNameOrDescriptionFilled(!!name || !!description);
  }, [name, description]);

  useEffect(() => {
    return () => {
      if (countdownRef.current) {
        clearInterval(countdownRef.current);
      }
    };
  }, []);

  function setSystemTemplates(template) {
    if (template === "default") {
      form.setFieldsValue({
        ...defaultTemplate,
      });
    }
  }

  const createConfig = (key, label, children, span = 3) => ({
    key,
    label,
    children,
    span,
  });

  //#region Profile configurations
  const detectionTime = Form.useWatch("DetectionTime", form);
  const mode = Form.useWatch("DefenseMode", form);

  const generalItems = [
    createConfig("name", "Profile name", name),
    createConfig("description", "Description", description),
    createConfig("time", "Detection time", detectionTime, 1),
    createConfig("mode", "Defense mode", mode, 2),
  ];

  const synFloodEn = Form.useWatch("SYNFloodEnable", form);
  const synFloodSYNThr = Form.useWatch("SYNFloodSYNThreshold", form);
  const synFloodACKThr = Form.useWatch("SYNFloodACKThreshold", form);
  const synFloodTimeout = Form.useWatch("SYNFloodWhiteListTimeOut", form);
  const tcpFragmentEn = Form.useWatch("TCPFragmentEnable", form);
  const landAttackEn = Form.useWatch("LandAttackEnable", form);

  const tcpConfig = [
    createConfig(
      "synflood",
      "SYN Flood",
      <Badge
        {...(synFloodEn ? { status: "success" } : { status: "error" })}
        text={synFloodEn ? "Enable" : "Disable"}
      />
    ),
    createConfig(
      "synfconf",
      "SYN Flood settings",
      <>
        SYN Packets threshold: {synFloodSYNThr}
        <br />
        ACK Packets threshold: {synFloodACKThr}
        <br />
        Whitelist duration: {synFloodTimeout}
      </>
    ),
    createConfig(
      "tcpfrag",
      "TCP Fragmentation",
      <Badge
        {...(tcpFragmentEn ? { status: "success" } : { status: "error" })}
        text={tcpFragmentEn ? "Enable" : "Disable"}
      />
    ),
    createConfig(
      "landatk",
      "Land Attack",
      <Badge
        {...(landAttackEn ? { status: "success" } : { status: "error" })}
        text={landAttackEn ? "Enable" : "Disable"}
      />
    ),
  ];

  const udpFloodEn = Form.useWatch("UDPFloodEnable", form);
  const udpFloodThr = Form.useWatch("UDPFloodThreshold", form);
  const udpFloodRate = Form.useWatch("UDPFloodRate", form);
  const udpFragmentEn = Form.useWatch("UDPFragmentEnable", form);
  const ipSecIKEEn = Form.useWatch("IPSecIKEEnable", form);
  const ipSecIKEThr = Form.useWatch("IPSecIKEThreshold", form);

  const udpConfig = [
    createConfig(
      "udpflood",
      "UDP Flood",
      <Badge
        {...(udpFloodEn ? { status: "success" } : { status: "error" })}
        text={udpFloodEn ? "Enable" : "Disable"}
      />
    ),
    createConfig(
      "udpfconf",
      "UDP Flood settings",
      <>
        UDP Packets threshold: {udpFloodThr}
        <br />
        UDP Packets threshold in 1 second: {udpFloodRate}
      </>
    ),
    createConfig(
      "udpfrag",
      "UDP Fragmentation",
      <Badge
        {...(udpFragmentEn ? { status: "success" } : { status: "error" })}
        text={udpFragmentEn ? "Enable" : "Disable"}
      />
    ),
    createConfig(
      "ipsec",
      "IPSec IKE Flood",
      <Badge
        {...(ipSecIKEEn ? { status: "success" } : { status: "error" })}
        text={ipSecIKEEn ? "Enable" : "Disable"}
      />
    ),
    createConfig(
      "udpfconf",
      "IPSec IKE settings",
      <>IPSec IKE Packets threshold: {ipSecIKEThr}</>
    ),
  ];

  const icmpFloodEn = Form.useWatch("ICMPFloodEnable", form);
  const icmpFloodThr = Form.useWatch("ICMPFloodThreshold", form);
  const icmpFloodRate = Form.useWatch("ICMPFloodRate", form);

  const icmpConfig = [
    createConfig(
      "icmpflood",
      "ICMP Flood",
      <Badge
        {...(icmpFloodEn ? { status: "success" } : { status: "error" })}
        text={icmpFloodEn ? "Enable" : "Disable"}
      />
    ),
    createConfig(
      "icmpfconf",
      "ICMP Flood settings",
      <>
        ICMP Packets threshold: {icmpFloodThr}
        <br />
        ICMP Packets threshold in 1 second: {icmpFloodRate}
      </>
    ),
  ];

  const dnsFloodEn = Form.useWatch("DNSFloodEnable", form);
  const dnsFloodThr = Form.useWatch("DNSFloodThreshold", form);

  const dnsConfig = [
    createConfig(
      "dnsflood",
      "DNS Flood",
      <Badge
        {...(dnsFloodEn ? { status: "success" } : { status: "error" })}
        text={dnsFloodEn ? "Enable" : "Disable"}
      />
    ),
    createConfig(
      "dnsfconf",
      "DNS Flood settings",
      <>DNS Packets threshold: {dnsFloodThr}</>
    ),
  ];

  const httpFloodEn = Form.useWatch("HTTPFloodEnable", form);
  const httpsFloodEn = Form.useWatch("HTTPSFloodEnable", form);

  const httpConfig = [
    createConfig(
      "httpflood",
      "HTTP Flood",
      <Badge
        {...(httpFloodEn ? { status: "success" } : { status: "error" })}
        text={httpFloodEn ? "Enable" : "Disable"}
      />
    ),
    createConfig(
      "httpsflood",
      "HTTPs Flood",
      <Badge
        {...(httpsFloodEn ? { status: "success" } : { status: "error" })}
        text={httpsFloodEn ? "Enable" : "Disable"}
      />
    ),
  ];
  //#endregion

  const renderDescriptions = (title, items) => (
    <>
      <Divider
        variant="solid"
        style={{ borderColor: "#001529", margin: "0px 0" }}
      >
        {title}
      </Divider>
      <Descriptions bordered title="" size="small" items={items} />
    </>
  );

  const items = [
    {
      key: "general",
      label: "General",
      children: <SettingGeneral />,
      forceRender: true,
    },
    {
      key: "network",
      label: "Network",
      children: <SettingNetwork />,
      forceRender: true,
    },
    {
      key: "transport",
      label: "Transport",
      children: <SettingTransport />,
      forceRender: true,
    },
    {
      key: "session",
      label: "Session",
      children: <SettingSession />,
      forceRender: true,
    },
    {
      key: "application",
      label: "Application",
      children: <SettingApplication />,
      forceRender: true,
    },
    {
      key: "default",
      label: "Default",
      children: <SettingTemplates setTemplate={setSystemTemplates} />,
      forceRender: true,
    },
  ];

  // Add utility function for error modal
  const showErrorModal = (message) => {
    Modal.error({
      title: "Error",
      content: message || "An error occurred. Please try again.",
    });
  };

  // Update handleSave function with countdown in modal
  const handleSave = () => {
    form
      .validateFields()
      .then(async (values) => {
        if (!values || Object.keys(values).length === 0) {
          showErrorModal("Invalid data. Please check again.");
          return;
        }

        setCountdownActive(true);
        setEstimatedTime(42);
        setIsModalOpen(true);

        let timeLeft = 42;
        countdownRef.current = setInterval(() => {
          timeLeft -= 1;
          setEstimatedTime(timeLeft);
          if (timeLeft <= 0) {
            clearInterval(countdownRef.current);
            setCountdownActive(false);
            setEstimatedTime(0);
            setIsModalOpen(false);
            showErrorModal("Timeout: Failed to save defense profile.");
          }
        }, 1000);

        try {
          const response = await insertDefenseProfile(values);
          if (response?.data) {
            clearInterval(countdownRef.current);
            setCountdownActive(false);
            setEstimatedTime(0);
            setIsModalOpen(false);
            Modal.success({
              title: "Success",
              content: "Defense profile created successfully.",
            });
            form.resetFields();
          } else {
            throw new Error("No data received from server.");
          }
        } catch (error) {
          clearInterval(countdownRef.current);
          setCountdownActive(false);
          setEstimatedTime(0);
          setIsModalOpen(false);
          showErrorModal(
            error.response?.data?.message || "Failed to save defense profile."
          );
        }
      })
      .catch(() => {
        showErrorModal("Please fill in all required fields.");
      });
  };

  return (
    <>
      <Row gutter={16}>
        <Col xs={24} sm={24} md={24} lg={15}>
          <Form
            form={form}
            layout="vertical"
            requiredMark={true}
            onFinish={handleSave}
            initialValues={{
              ...defaultTemplate,
            }}
          >
            <Space
              direction="vertical"
              size={"middle"}
              style={{ width: "100%" }}
            >
              <Tabs className="config-tabs" mode="horizontal" items={items} />
              <Button
                type="primary"
                htmlType="submit"
                icon={<SaveOutlined />}
                size={"large"}
                disabled={!isNameOrDescriptionFilled || countdownActive}
                loading={countdownActive}
              >
                Save defense profile
              </Button>
            </Space>
          </Form>
        </Col>
        <Col xs={24} sm={24} md={24} lg={9}>
          <div className="summarize-pane">
            <Space direction="vertical" size="small" style={{ width: "100%" }}>
              <Descriptions
                bordered
                title="Defense Profile Summary"
                size="small"
                items={generalItems}
              />
              {renderDescriptions("TCP", tcpConfig)}
              {renderDescriptions("UDP", udpConfig)}
              {renderDescriptions("ICMP", icmpConfig)}
              {renderDescriptions("DNS", dnsConfig)}
              {renderDescriptions("HTTP / HTTPs", httpConfig)}
            </Space>
          </div>
        </Col>
      </Row>
      <Modal
        title="Saving Defense Profile"
        open={isModalOpen}
        footer={null}
        closable={false}
      >
        <div
          style={{
            display: "flex",
            alignItems: "center",
            justifyContent: "center",
            marginTop: 16,
            padding: "8px",
            backgroundColor: "#f0f0f0",
            borderRadius: "4px",
          }}
        >
          <SettingOutlined
            spin
            style={{ fontSize: 20, color: "#1890ff", marginRight: 8 }}
          />
          <p style={{ color: "#888", margin: 0 }}>
            Estimated Time: {estimatedTime}s
          </p>
        </div>
      </Modal>
    </>
  );
};

export default CreateDefenseProfile;
