import React from "react";
import { Result, Button, Card, Space, Typography, List } from "antd";
import {
  CheckCircleOutlined,
  SafetyOutlined,
  SettingOutlined,
} from "@ant-design/icons";
import { Link } from "react-router-dom";

const { Title, Text, Paragraph } = Typography;

const IpSecProgress = () => {
  const profileDetails = [
    {
      title: "Certificate Status",
      description: "Server certification imported successfully",
      icon: <CheckCircleOutlined style={{ color: "#52c41a" }} />,
    },
    {
      title: "Security Configuration",
      description: "IPSec profile configured with optimal settings",
      icon: <SafetyOutlined style={{ color: "#1890ff" }} />,
    },
    {
      title: "Network Protection",
      description: "Profile activated and ready for deployment",
      icon: <SettingOutlined style={{ color: "#722ed1" }} />,
    },
  ];

  const handleCreateAnother = () => {
    // Navigate to create new profile
    window.location.reload();
  };

  const handleViewProfiles = () => {
    // Navigate to profiles list
    console.log("Navigate to profiles list");
  };

  return (
    <div style={{ padding: "40px 20px", maxWidth: "800px", margin: "0 auto" }}>
      <Result
        status="success"
        title={
          <Title level={2} style={{ color: "#52c41a", marginBottom: "8px" }}>
            IP Security Profile Created Successfully!
          </Title>
        }
        subTitle={
          <Paragraph
            style={{ fontSize: "16px", color: "#666", marginBottom: "32px" }}
          >
            Your IP Security security profile has been configured and is now
            ready to protect your network communications.
          </Paragraph>
        }
        icon={
          <div style={{ position: "relative" }}>
            <CheckCircleOutlined
              style={{ fontSize: "72px", color: "#52c41a" }}
            />
            {/* <SafetyOutlined 
              style={{ 
                position: 'absolute', 
                bottom: '-8px', 
                right: '-8px', 
                fontSize: '24px', 
                color: '#1890ff',
                backgroundColor: 'white',
                borderRadius: '50%',
                padding: '2px'
              }} 
            /> */}
          </div>
        }
        extra={
          <Space size="large" direction="vertical" style={{ width: "100%" }}>
            <Card
              title={
                <Title level={4} style={{ margin: 0, color: "#1890ff" }}>
                  <SafetyOutlined style={{ marginRight: "8px" }} />
                  Configuration Summary
                </Title>
              }
              style={{ textAlign: "left", marginBottom: "24px" }}
            >
              <List
                dataSource={profileDetails}
                renderItem={(item) => (
                  <List.Item style={{ border: "none", padding: "12px 0" }}>
                    <List.Item.Meta
                      avatar={item.icon}
                      title={<Text strong>{item.title}</Text>}
                      description={
                        <Text type="secondary">{item.description}</Text>
                      }
                    />
                  </List.Item>
                )}
              />
            </Card>

            <Space size="middle">
              <Button
                type="primary"
                size="large"
                onClick={handleCreateAnother}
                style={{
                  background:
                    "linear-gradient(135deg, #1890ff 0%, #096dd9 100%)",
                  border: "none",
                  borderRadius: "8px",
                  height: "45px",
                  paddingLeft: "24px",
                  paddingRight: "24px",
                }}
              >
                Create Another Profile
              </Button>
              <Button
                size="large"
                onClick={handleViewProfiles}
                style={{
                  borderRadius: "8px",
                  height: "45px",
                  paddingLeft: "24px",
                  paddingRight: "24px",
                }}
              >
                <Link to="/defense/interface/IpSecProfile">
                  View All Profiles
                </Link>
              </Button>
            </Space>
          </Space>
        }
      />

      <Card
        style={{
          marginTop: "32px",
          background: "linear-gradient(135deg, #f0f9ff 0%, #e0f2fe 100%)",
          border: "1px solid #91d5ff",
        }}
      >
        <Space align="center">
          <SafetyOutlined style={{ fontSize: "20px", color: "#1890ff" }} />
          <div>
            <Text strong style={{ color: "#1890ff" }}>
              Security Tip:{" "}
            </Text>
            <Text type="secondary">
              Your IP Security profile is now active. Remember to regularly
              update certificates and review security settings.
            </Text>
          </div>
        </Space>
      </Card>
    </div>
  );
};

export default IpSecProgress;
