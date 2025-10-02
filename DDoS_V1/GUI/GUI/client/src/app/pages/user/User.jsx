import { useEffect, useState } from "react";
import { Link } from "react-router-dom";

import {
  EditOutlined,
  LockOutlined,
  MailOutlined,
  UserOutlined,
  SettingOutlined,
  CheckOutlined,
  CloseOutlined,
  ProfileOutlined,
} from "@ant-design/icons";
import {
  Button,
  Col,
  Form,
  Descriptions,
  Input,
  Row,
  message,
  Space,
  Card,
  Flex,
  Typography,
  Divider,
  Switch,
  Tag,
  Modal,
} from "antd";

const { Title } = Typography;

import "@/features/users/styles/main.css";
import { getLoggedInUser, updateUser } from "@/features/api/Users";
import { profileTimeFormatter } from "@/lib/formatter";
import Logo from "@/assets/logo.svg";

export default function Profile() {
  const [form] = Form.useForm();

  const [disabled, setDisabled] = useState(true);
  const [userData, setUserData] = useState(null);

  const [isEnabled, setIsEnabled] = useState(false);

  const toggle = () => {
    setDisabled(!disabled);
  };

  const handleSwitchChange = (checked) => {
    setIsEnabled(checked);
  };

  const fetchUserData = async () => {
    try {
      const response = await getLoggedInUser();
      setUserData(response.data);
      form.setFieldsValue({
        UserFullName: response.data?.UserFullName ?? "N/A",
        Email: response.data?.Email ?? "N/A",
        NotifyDDoSAttackDetect: response.data?.NotifyDDoSAttackDetect === 1,
        NotifyDDoSAttackEnd: response.data?.NotifyDDoSAttackEnd === 1,
        NotifyDiskExceeds: response.data?.NotifyDiskExceeds === 1,
        NotifyNetworkAnomalyDetect:
          response.data?.NotifyNetworkAnomalyDetect === 1,
      });
    } catch (error) {
      console.error("Failed to fetch user data: ", error);
    }
  };

  const generateItems = (profiles) => {
    if (!profiles || profiles.length === 0) return [];
    return profiles
      .map((profile, index) => [
        {
          key: `name-${index}`,
          label: <ProfileOutlined />,
          labelStyle: { display: "none" },
          children: (
            <p>
              Profile{" "}
              <span>
                <strong>{profile.Name || "N/A"}</strong>
              </span>
            </p>
          ),
          span: 3,
        },
        {
          key: `time-${index}`,
          label: "Usage",
          children: profileTimeFormatter(profile.UsingTime) || "N/A",
          span: 3,
        },
      ])
      .flat();
  };

  const items = generateItems(userData?.listSettingProfile);

  const handleUpdateProfile = async (values) => {
    try {
      const submitValues = { ...values };
      if (!isEnabled) {
        delete submitValues.CurrentPassword;
        delete submitValues.Password;
        delete submitValues.ConfirmPassword;
      }

      Modal.confirm({
        title: "Confirm",
        content: "Are you sure you want to save the changes?",
        onOk: async () => {
          try {
            const response = await updateUser(userData.UserId, submitValues);
            message.success(response.message);
          } catch (error) {
            message.error(
              error.response?.data?.message ||
                "Failed to update user profile. Please try again."
            );
          } finally {
            setDisabled(true);
            setIsEnabled(false); // Reset switch to off after save
            // Reset password fields
            form.resetFields([
              "CurrentPassword",
              "Password",
              "ConfirmPassword",
            ]);
            await fetchUserData();
          }
        },
        onCancel: () => {
          message.info("Changes have not been saved.");
          // Reset password fields when cancel
          form.resetFields(["CurrentPassword", "Password", "ConfirmPassword"]);
        },
      });
    } catch (error) {
      message.error(
        error.response?.data?.message ||
          "Failed to update user profile. Please try again."
      );
      // Reset password fields on error
      form.resetFields(["CurrentPassword", "Password", "ConfirmPassword"]);
    }
  };

  useEffect(() => {
    fetchUserData();
  }, []);

  //Chỉnh sửa avt thành logo
  return (
    <Form
      layout="vertical"
      colon={false}
      requiredMark={false}
      form={form}
      onFinish={handleUpdateProfile}
    >
      <Row style={{ marginTop: "20px" }} gutter={[16, 16]}>
        <Col span={3} offset={1}>
          <Flex
            vertical
            style={{ width: "100%" }}
            align="center"
            justify="center"
            gap={20}
          >
            {/* <Avatar shape="square" size={128} src={Logo} /> */}
            <div
              style={{
                width: 128,
                height: 128,
                display: "flex",
                alignItems: "center",
                justifyContent: "center",
              }}
            >
              <img
                src={Logo}
                alt="Logo"
                style={{ maxWidth: "100%", maxHeight: "100%" }}
              />
            </div>
            <Space direction="vertical" align="center" size={10}>
              <Tag
                style={{ marginInlineEnd: 0 }}
                color={userData?.Role === "admin" ? "red" : "green"}
              >
                {userData?.Role}
              </Tag>
              <Title style={{ marginBottom: 5 }} level={3}>
                {userData?.UserFullName ?? "N/A"}
              </Title>
              <p style={{ textAlign: "center" }}>
                Member Since <br /> {userData?.CreateTime ?? "N/A"}
              </p>
            </Space>
          </Flex>
        </Col>

        <Col span={12}>
          <Flex vertical size="middle" style={{ width: "100%" }} gap={20}>
            <Card>
              <Flex justify="space-between" align="middle">
                <Title level={4}>Profile Info</Title>
                <Button
                  type="primary"
                  icon={disabled ? <EditOutlined /> : <CloseOutlined />}
                  onClick={toggle}
                  danger={!disabled}
                >
                  {disabled ? "Edit" : "Cancel"}
                </Button>
              </Flex>
              <Form.Item
                label={
                  <span style={{ fontSize: "15px", fontWeight: 500 }}>
                    Full Name
                  </span>
                }
                layout="vertical"
                name="UserFullName"
                rules={[
                  { required: true, message: "Please enter a full name" },
                ]}
              >
                <Input
                  size="large"
                  disabled={disabled}
                  prefix={<UserOutlined />}
                />
              </Form.Item>
              <Divider style={{ borderColor: "#001529" }} />

              <Title level={4}>Notification</Title>
              <Form.Item
                label={
                  <span style={{ fontSize: "15px", fontWeight: 500 }}>
                    Email
                  </span>
                }
                layout="vertical"
                name="Email"
                rules={[
                  { required: true, message: "Please enter an email address" },
                  {
                    type: "email",
                    message: "Please enter a valid email address",
                  },
                ]}
              >
                <Input disabled={disabled} prefix={<MailOutlined />} />
              </Form.Item>

              <Title level={5}>Event</Title>
              <Form.Item
                label="Network anomaly detected"
                name="NotifyNetworkAnomalyDetect"
                valuePropName="checked"
                layout="horizontal"
                style={{ marginBottom: 5 }}
                labelAlign="left"
                labelCol={{ sm: { span: 22 } }}
                wrapperCol={{ sm: { span: 2 } }}
              >
                <Switch
                  size="small"
                  disabled={disabled}
                  style={{ float: "right" }}
                />
              </Form.Item>

              <Form.Item
                label="DDoS attack detected"
                name="NotifyDDoSAttackDetect"
                valuePropName="checked"
                layout="horizontal"
                style={{ marginBottom: 5 }}
                labelAlign="left"
                labelCol={{ sm: { span: 22 } }}
                wrapperCol={{ sm: { span: 2 } }}
              >
                <Switch
                  size="small"
                  disabled={disabled} // Switches: only disabled by global disabled
                  style={{ float: "right" }}
                />
              </Form.Item>

              <Form.Item
                label="DDoS attack end"
                valuePropName="checked"
                name="NotifyDDoSAttackEnd"
                layout="horizontal"
                style={{ marginBottom: 5 }}
                labelAlign="left"
                labelCol={{ sm: { span: 22 } }}
                wrapperCol={{ sm: { span: 2 } }}
              >
                <Switch
                  size="small"
                  disabled={disabled} // Switches: only disabled by global disabled
                  style={{ float: "right" }}
                />
              </Form.Item>

              <Form.Item
                label="Disk usage exceeds threshold"
                name="NotifyDiskExceeds"
                valuePropName="checked"
                layout="horizontal"
                style={{ marginBottom: 5 }}
                labelAlign="left"
                labelCol={{ sm: { span: 22 } }}
                wrapperCol={{ sm: { span: 2 } }}
              >
                <Switch
                  size="small"
                  disabled={disabled} // Switches: only disabled by global disabled
                  style={{ float: "right" }}
                />
              </Form.Item>

              <Divider style={{ borderColor: "#001529" }} />
              <Flex justify="space-between" align="middle">
                <Title level={4}>Update password</Title>
                <Switch
                  onChange={handleSwitchChange}
                  checked={isEnabled}
                  checkedChildren="Enabled"
                  unCheckedChildren="Disabled"
                  disabled={disabled}
                />
              </Flex>
              <Form.Item
                label={
                  <span style={{ fontSize: "15px", fontWeight: 400 }}>
                    Current password
                  </span>
                }
                layout="vertical"
                name="CurrentPassword"
                rules={[
                  {
                    required: isEnabled,
                    message: "Please enter your current password",
                  },
                ]}
              >
                <Input.Password
                  prefix={<LockOutlined />}
                  disabled={disabled || !isEnabled}
                />
              </Form.Item>
              <Form.Item
                label={
                  <span style={{ fontSize: "15px", fontWeight: 400 }}>
                    New password
                  </span>
                }
                layout="vertical"
                name="Password"
                rules={[
                  {
                    required: isEnabled,
                    message: "Please enter a new password",
                  },
                  {
                    min: 8,
                    message: "Password must be at least 8 characters",
                  },
                ]}
              >
                <Input.Password
                  prefix={<LockOutlined />}
                  disabled={disabled || !isEnabled}
                />
              </Form.Item>
              <Form.Item
                label={
                  <span style={{ fontSize: "15px", fontWeight: 400 }}>
                    Re-type new password
                  </span>
                }
                layout="vertical"
                name="ConfirmPassword"
                dependencies={["Password"]}
                rules={[
                  {
                    required: isEnabled, // Rule only applies if switch is on
                    message: "Please re-type the new password",
                  },
                  ({ getFieldValue }) => ({
                    validator(_, value) {
                      if (
                        !isEnabled ||
                        !value ||
                        getFieldValue("Password") === value
                      ) {
                        return Promise.resolve();
                      }
                      return Promise.reject(
                        new Error("The passwords do not match")
                      );
                    },
                  }),
                ]}
              >
                <Input.Password
                  prefix={<LockOutlined />}
                  disabled={disabled || !isEnabled}
                />
              </Form.Item>

              <Button
                htmlType="submit"
                size="large"
                icon={<CheckOutlined />}
                color="green"
                variant="solid"
                disabled={disabled}
                style={{ width: "100%" }}
              >
                Save
              </Button>
            </Card>
          </Flex>
        </Col>

        <Col span={7} gap={20}>
          <Card>
            <Flex justify="space-between" align="middle">
              <Title level={5}>
                {userData?.UserFullName}'s setting profile
              </Title>
              <Space>
                <Link to="/defense/interface/defense-profile-list">
                  <SettingOutlined className="icon" />
                </Link>
              </Space>
            </Flex>
            <Descriptions bordered size="small" items={items} />
          </Card>
        </Col>
      </Row>
    </Form>
  );
}
