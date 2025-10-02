import React, { useState, useEffect, useRef } from "react";
import { Row, Col, Divider, Descriptions, Space, Tabs, Form } from "antd";
import {
  IKESetting,
  NodeSpecSetting,
} from "@/features/defense/ipSetting/setting/IpSec";
import "../styles/main.css";

const IpSecProfileSetting = ({ onValidationChange }) => {
  const [form] = Form.useForm();
  const [isNameOrDescriptionFilled, setIsNameOrDescriptionFilled] =
    useState(false);
  const renderCount = useRef(0);
  renderCount.current += 1;

  const defaultTemplate = {
    IpSecProfileName: "IpSec Profile Num 0",
    IpSecProfileDescription: "Sample description",
    ConnectionCount: 1,
    LocalGatewayIPv4: "1.1.1.1",
    RemoteGatewayIPv4: "1.1.1.1",
    SubnetLocalGatewayIPv4: "255.255.255.0",
    SubnetRemoteGatewayIPv4: "255.255.255.0",
    IKE2Version: "v2",
    IKEMode: "tunnel",
    ESPAHProtocol: "ESP",
    IKEReauthTime: "3600",
    HashAlgorithm: "sha256",
    EncryptionAlgorithm: "aes-256",
    RekeyTime: "86400",
  };

  // Watch only name and description
  const name = Form.useWatch("IpSecProfileName", form);
  const description = Form.useWatch("IpSecProfileDescription", form);
  const connectionCount = Form.useWatch("ConnectionCount", form);

  useEffect(() => {
    setIsNameOrDescriptionFilled(!!name?.trim() || !!description?.trim());
  }, [name, description]);

  const createConfig = (key, label, children, span = 1) => ({
    key,
    label,
    children,
    span,
  });

  const localGateway =
    form.getFieldValue("LocalGatewayIPv4") ||
    form.getFieldValue("LocalGatewayIPv6");

  const subnetLocalGateway =
    form.getFieldValue("SubnetLocalGatewayIPv4") ||
    form.getFieldValue("SubnetLocalGatewayIPv6");

  const remoteGateway =
    form.getFieldValue("RemoteGatewayIPv4") ||
    form.getFieldValue("RemoteGatewayIPv6");

  const subnetRemoteGateway =
    form.getFieldValue("SubnetRemoteGatewayIPv4") ||
    form.getFieldValue("SubnetRemoteGatewayIPv6");

  const generalItems = [
    createConfig("name", "Profile name", name),
    createConfig("description", "Description", description),

    createConfig("connectionCount", "Connection ", connectionCount),
    createConfig("local", "Local Gateway", localGateway),
    createConfig("subnet-local", "Subnet Local Gateway", subnetLocalGateway),
    ...(connectionCount > 1
      ? Array.from({ length: connectionCount }, (_, index) => [
          createConfig(
            `remote-${index}`,
            `Remote Gateway ${index + 1}`,
            form.getFieldValue(`RemoteGatewayIPv4_${index}`) ||
              form.getFieldValue(`RemoteGatewayIPv6_${index}`)
          ),
          createConfig(
            `subnet-remote-${index}`,
            `Subnet Remote Gateway ${index + 1}`,
            form.getFieldValue(`SubnetRemoteGatewayIPv4_${index}`) ||
              form.getFieldValue(`SubnetRemoteGatewayIPv6_${index}`)
          ),
        ]).flat()
      : [
          createConfig("remote", "Remote Gateway", remoteGateway),
          createConfig(
            "subnet-remote",
            "Subnet Remote Gateway",
            subnetRemoteGateway
          ),
        ]),
  ];

  const ikeConfig = [
    createConfig("version", "IKE2 Version", form.getFieldValue("IKE2Version")),
    createConfig(
      "mode",
      "Mode",
      form.getFieldValue("IKEMode") === "tunnel"
        ? "Tunnel Mode"
        : "Transport Mode"
    ),
    createConfig(
      "protocol",
      "ESP/AH Protocol",
      form.getFieldValue("ESPAHProtocol")
    ),
    createConfig(
      "reauth",
      "IKE Reauth time (optional)",
      form.getFieldValue("IKEReauthTime")
    ),
    createConfig(
      "enc",
      "Encryption Algorithm",
      form.getFieldValue("EncryptionAlgorithm")?.toUpperCase()
    ),
    ...(form.getFieldValue("ESPAHProtocol") === "ESP"
      ? [
          createConfig(
            "hash",
            "Hash Algorithm",
            form.getFieldValue("HashAlgorithm")?.toUpperCase()
          ),
        ]
      : []),
    createConfig(
      "rekey",
      "Re-key Time (seconds)",
      form.getFieldValue("RekeyTime")
    ),
  ];

  const renderDescriptions = (title, items) => (
    <>
      <Divider
        variant="solid"
        style={{ borderColor: "#001529", margin: "0px 0" }}
      >
        {title}
      </Divider>
      <Descriptions bordered title="" size="small" items={items} column={1} />
    </>
  );

  const items = [
    {
      key: "node",
      label: connectionCount === 1 ? "Net12 Settings" : "Net13 Settings",
      children: <NodeSpecSetting />,
      forceRender: true,
    },
    {
      key: "ike",
      label: "IKE Settings",
      children: <IKESetting />,
      forceRender: true,
    },
  ];

  return (
    <Row gutter={[16]}>
      <Col xs={24} sm={24} md={24} lg={15}>
        <Form
          form={form}
          layout="vertical"
          requiredMark={true}
          initialValues={defaultTemplate}
          onValuesChange={(_, values) => {
            onValidationChange(true, values);
          }}
        >
          <Space direction="vertical" size="middle" style={{ width: "100%" }}>
            <Tabs
              className="config-tabs"
              mode="horizontal"
              items={items}
              destroyInactiveTabPane={false}
            />
          </Space>
        </Form>
      </Col>
      <Col xs={24} sm={24} md={24} lg={9}>
        <div className="summarize-pane" style={{ marginTop: "3.8rem" }}>
          <Space direction="vertical" size="small" style={{ width: "100%" }}>
            <Descriptions
              bordered
              title="IPSec Profile Summary"
              size="small"
              items={generalItems}
              column={1}
              style={{ width: "100%", tableLayout: "fixed" }}
            />
            {renderDescriptions("IKE Configuration", ikeConfig)}
          </Space>
        </div>
      </Col>
    </Row>
  );
};

export default IpSecProfileSetting;
