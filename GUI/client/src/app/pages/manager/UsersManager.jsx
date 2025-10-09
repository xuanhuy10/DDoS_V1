import React, { useEffect, useState } from "react";
import {
  Modal,
  Table,
  Input,
  Tag,
  Button,
  Alert,
  Form,
  Checkbox,
  Card,
  Space,
  Flex,
  message,
  Collapse,
} from "antd";
import {
  DeleteOutlined,
  EditOutlined,
  PlusSquareOutlined,
  MailOutlined,
  LockOutlined,
  UserAddOutlined,
  InfoCircleOutlined,
  ClockCircleOutlined,
} from "@ant-design/icons";
import { FiUsers } from "react-icons/fi";

import PageTitle from "@/components/common/PageTitle";

import {
  getAllUsers,
  insertUser,
  updateUser,
  deleteUser,
} from "@/features/api/Users";

import "@/features/manager/users/styles/main.css";

export default function UserManager() {
  const [form] = Form.useForm();

  const [refresh, setRefresh] = useState(false);
  const [users, setUsers] = useState([]);
  const [error, setError] = useState(null);
  const [addUserModal, setAddUserModal] = useState(false);
  const [editingUser, setEditingUser] = useState(null);
  const [isLoading, setIsLoading] = useState(false);
  const [isUpdating, setIsUpdating] = useState(false);
  const [formChanged, setFormChanged] = useState(false);
  const [initialValues, setInitialValues] = useState({});

  useEffect(() => {
    fetchUsers();
  }, [refresh]);

  const fetchUsers = async () => {
    setIsLoading(true);
    setError(null);
    try {
      const response = await getAllUsers();
      setUsers(response.data);
    } catch (error) {
      setError("Failed to fetch users. Please try again later.");
    } finally {
      setIsLoading(false);
    }
  };

  const columns = [
    {
      title: "Username",
      dataIndex: "Username",
      key: "username",
    },
    {
      title: "Full Name",
      dataIndex: "UserFullName",
      key: "name",
    },
    {
      title: "Role",
      dataIndex: "Role",
      key: "role",
      render: (role) => (
        <Tag color={role === "admin" ? "red" : "green"}>{role}</Tag>
      ),
    },
    {
      title: "Email",
      dataIndex: "Email",
      key: "email",
      render: (_, record) => (
        <a href={`mailto:${record.Email}`}>{record.Email}</a>
      ),
    },
    {
      title: "Last Login",
      dataIndex: "LastLogin",
      key: "lastLogin",
      render: (text) =>
        text === null ? (
          <Tag icon={<ClockCircleOutlined />} color="warning">
            Never Logged In
          </Tag>
        ) : (
          text
        ),
    },
    {
      title: "Created Date",
      dataIndex: "CreateTime",
      key: "createdDate",
    },
    {
      title: "Action",
      key: "action",
      render: (_, record) => (
        <Space>
          <Button
            type="dashed"
            icon={<EditOutlined />}
            onClick={() => handleEdit(record)}
          />
          {record.Role !== "admin" && (
            <Button
              danger
              icon={<DeleteOutlined />}
              onClick={() => handleDelete(record)}
            />
          )}
        </Space>
      ),
    },
  ];

  const handleNewUserBtn = () => {
    try {
      if (users.length === 6) {
        message.error(
          "Maximum number of users reached, please delete a user to add a new one"
        );
      } else {
        form.resetFields();
        setEditingUser(null);

        const defaultValues = {
          NotifyDDoSAttackDetect: true,
          NotifyDDoSAttackEnd: true,
          NotifyNetworkAnomalyDetect: true,
          NotifyDiskExceeds: true,
        };

        setInitialValues(defaultValues);

        form.setFieldsValue(defaultValues);
        setFormChanged(false);
        setAddUserModal(true);
      }
    } catch (error) {
      message.error(
        error.response?.data?.message ||
          "Failed to add new user. Please try again later."
      );
    }
  };

  const CreateUserOrUpdate = async (values) => {
    try {
      setIsUpdating(true);
      if (editingUser) {
        await updateUser(editingUser.UserId, values);
        message.success("User updated successfully!");
      } else {
        await insertUser(values);
        message.success("User created successfully!");
      }
      setAddUserModal(false);
      form.resetFields();
      setFormChanged(false);
      setRefresh(!refresh);
    } catch (error) {
      message.error(
        error.response?.data?.message ||
          "Failed to save user. Please try again."
      );
    } finally {
      setIsUpdating(false);
    }
  };

  const handleEdit = (record) => {
    setEditingUser(record);

    const userValues = {
      ...record,
      NotifyDDoSAttackDetect: record.NotifyDDoSAttackDetect === 1,
      NotifyDDoSAttackEnd: record.NotifyDDoSAttackEnd === 1,
      NotifyNetworkAnomalyDetect: record.NotifyNetworkAnomalyDetect === 1,
      NotifyDiskExceeds: record.NotifyDiskExceeds === 1,
    };

    setInitialValues(userValues);
    form.setFieldsValue(userValues);
    setFormChanged(false);
    setAddUserModal(true);
  };

  const handleDelete = async (record) => {
    try {
      if (record.Role === "admin") {
        message.error("Cannot delete an admin user");
      } else {
        Modal.confirm({
          title: "Delete User",
          content: `Are you sure you want to delete user ${record.Username}?`,
          onOk: async () => {
            try {
              await deleteUser(record.UserId);
              message.success("User deleted successfully!");
              setRefresh(!refresh);
            } catch (error) {
              message.error(
                error.response?.data?.message ||
                  "Failed to delete user. Please try again."
              );
            }
          },
        });
      }
    } catch (error) {
      message.error(
        error.response?.data?.message ||
          "Failed to delete user. Please try again."
      );
    }
  };

  const checkFormChanged = (changedValues, allValues) => {
    if (!editingUser) {
      // Trường hợp tạo mới user
      const defaultValues = {
        NotifyDDoSAttackDetect: true,
        NotifyDDoSAttackEnd: true,
        NotifyNetworkAnomalyDetect: true,
        NotifyDiskExceeds: true,
      };

      const isDefault =
        (!allValues.Username || allValues.Username === "") &&
        (!allValues.UserFullName || allValues.UserFullName === "") &&
        (!allValues.Email || allValues.Email === "") &&
        (!allValues.Password || allValues.Password === "") &&
        (!allValues.ConfirmPassword || allValues.ConfirmPassword === "") &&
        allValues.NotifyDDoSAttackDetect ===
          defaultValues.NotifyDDoSAttackDetect &&
        allValues.NotifyDDoSAttackEnd === defaultValues.NotifyDDoSAttackEnd &&
        allValues.NotifyNetworkAnomalyDetect ===
          defaultValues.NotifyNetworkAnomalyDetect &&
        allValues.NotifyDiskExceeds === defaultValues.NotifyDiskExceeds;

      setFormChanged(!isDefault);
    } else {
      // Trường hợp chỉnh sửa user
      const keysToCheck = [
        "Username",
        "UserFullName",
        "Email",
        "NotifyDDoSAttackDetect",
        "NotifyDDoSAttackEnd",
        "NotifyNetworkAnomalyDetect",
        "NotifyDiskExceeds",
        "Password",
        "ConfirmPassword",
      ];

      const changed = keysToCheck.some((key) => {
        if (
          (key === "Password" || key === "ConfirmPassword") &&
          !allValues[key]
        ) {
          // Nếu password hoặc confirm password chưa nhập thì bỏ qua
          return false;
        }

        const initVal = initialValues[key];
        const currVal = allValues[key];

        // So sánh boolean chính xác cho checkbox
        if (typeof initVal === "boolean" || typeof currVal === "boolean") {
          return Boolean(initVal) !== Boolean(currVal);
        }

        // So sánh kiểu string cho các trường còn lại
        return String(initVal || "") !== String(currVal || "");
      });

      setFormChanged(changed);
    }
  };

  const handleModalCancel = () => {
    setAddUserModal(false);
    setFormChanged(false);
    form.resetFields();
  };

  return (
    <div style={{ padding: "5px 15px" }}>
      <PageTitle
        title="User managers"
        description="Manage user accounts and roles for the device"
      />
      <Card>
        <Flex
          justify="space-between"
          align="center"
          style={{ marginBottom: 16, width: "100%" }}
        >
          <Space className="UserManagerHeader" align="center">
            <FiUsers />
            <p>
              <span style={{ color: users.length >= 6 ? "red" : "inherit" }}>
                {users.length ?? 0} / 6
              </span>{" "}
              Users
            </p>
          </Space>
          <Space>
            <Button
              color="green"
              variant="solid"
              icon={<PlusSquareOutlined />}
              onClick={handleNewUserBtn}
            >
              New User
            </Button>
          </Space>
        </Flex>
        {error && (
          <Alert
            message={error}
            type="error"
            showIcon
            style={{ marginBottom: 16 }}
          />
        )}
        <Table
          dataSource={users}
          columns={columns}
          loading={isLoading}
          scroll={{ x: 768 }}
          rowKey={(record) => record.UserId}
          pagination={false}
        />
        <Modal
          className="add-user-modal"
          title={editingUser ? "Edit User" : "Add New User"}
          open={addUserModal}
          okText={editingUser ? "Save" : "Add"}
          confirmLoading={isUpdating}
          cancelText="Cancel"
          okButtonProps={{ disabled: !formChanged }}
          onCancel={handleModalCancel}
          onOk={() =>
            form
              .validateFields()
              .then(CreateUserOrUpdate)
              .catch((info) => {
                console.error("Validation Failed:", info);
              })
          }
        >
          <Form
            layout="vertical"
            name="addUserForm"
            form={form}
            onValuesChange={checkFormChanged}
            requiredMark={editingUser ? false : true}
          >
            <Form.Item
              label="Username"
              name="Username"
              rules={[
                { required: true, message: "Please enter a username" },
                {
                  min: 5,
                  max: 7,
                  message: "Username must be between 5 and 7 characters",
                },
                {
                  pattern: /^[A-Za-z0-9]+$/,
                  message:
                    "Username must not contain Vietnamese characters or special symbols",
                },
              ]}
            >
              <Input
                prefix={
                  <UserAddOutlined style={{ color: "rgba(0,0,0,.25)" }} />
                }
                disabled={!!editingUser}
              />
            </Form.Item>
            <Form.Item
              label="Full Name"
              name="UserFullName"
              rules={[{ required: true, message: "Please enter a full name" }]}
            >
              <Input
                prefix={
                  <InfoCircleOutlined style={{ color: "rgba(0,0,0,.25)" }} />
                }
              />
            </Form.Item>
            <Form.Item
              label="Email"
              name="Email"
              rules={[
                { required: true, message: "Please enter an email address" },
                {
                  type: "email",
                  message: "Please enter a valid email address",
                },
              ]}
            >
              <Input
                prefix={<MailOutlined style={{ color: "rgba(0,0,0,.25)" }} />}
              />
            </Form.Item>

            <Collapse
              className="notification-settings"
              ghost
              items={[
                {
                  key: "1",
                  label: "Notification events",
                  forceRender: true,
                  children: (
                    <>
                      <Form.Item
                        name="NotifyDDoSAttackDetect"
                        valuePropName="checked"
                      >
                        <Checkbox>DDoS attack detected</Checkbox>
                      </Form.Item>
                      <Form.Item
                        name="NotifyDDoSAttackEnd"
                        valuePropName="checked"
                      >
                        <Checkbox>DDoS attack end</Checkbox>
                      </Form.Item>
                      <Form.Item
                        name="NotifyNetworkAnomalyDetect"
                        valuePropName="checked"
                      >
                        <Checkbox>Network anomaly detected</Checkbox>
                      </Form.Item>
                      <Form.Item
                        name="NotifyDiskExceeds"
                        valuePropName="checked"
                      >
                        <Checkbox>Disk usage exceeds</Checkbox>
                      </Form.Item>
                    </>
                  ),
                },
              ]}
            />
            {editingUser && (
              <Form.Item
                label="Current Password"
                name="CurrentPassword"
                rules={[
                  {
                    required: true,
                    message: "Please enter your current password",
                  },
                  // Có thể thêm validator gọi API kiểm tra mật khẩu cũ ở đây nếu muốn kiểm tra realtime
                ]}
              >
                <Input.Password
                  prefix={<LockOutlined style={{ color: "rgba(0,0,0,.25)" }} />}
                />
              </Form.Item>
            )}
            {editingUser ? (
              <>
                <Form.Item
                  label="New Password"
                  name="Password"
                  rules={[
                    ({ getFieldValue }) => ({
                      validator(_, value) {
                        if (!value || value.length >= 8) {
                          return Promise.resolve();
                        }
                        return Promise.reject(
                          new Error("Password must be at least 8 characters")
                        );
                      },
                    }),
                  ]}
                >
                  <Input.Password
                    prefix={
                      <LockOutlined style={{ color: "rgba(0,0,0,.25)" }} />
                    }
                  />
                </Form.Item>
                <Form.Item
                  label="Repeat New Password"
                  name="ConfirmPassword"
                  dependencies={["Password"]}
                  rules={[
                    ({ getFieldValue }) => ({
                      validator(_, value) {
                        if (!value || getFieldValue("Password") === value) {
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
                    prefix={
                      <LockOutlined style={{ color: "rgba(0,0,0,.25)" }} />
                    }
                  />
                </Form.Item>
              </>
            ) : (
              <>
                <Form.Item
                  label="Password"
                  name="Password"
                  rules={[
                    { required: true, message: "Please enter a password" },
                    {
                      min: 8,
                      message: "Password must be at least 8 characters",
                    },
                  ]}
                >
                  <Input.Password
                    prefix={
                      <LockOutlined style={{ color: "rgba(0,0,0,.25)" }} />
                    }
                  />
                </Form.Item>
                <Form.Item
                  label="Repeat Password"
                  name="ConfirmPassword"
                  dependencies={["Password"]}
                  rules={[
                    { required: true, message: "Please repeat the password" },
                    {
                      min: 8,
                      message: "Password must be at least 8 characters",
                    },
                    ({ getFieldValue }) => ({
                      validator(_, value) {
                        if (!value || getFieldValue("Password") === value) {
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
                    prefix={
                      <LockOutlined style={{ color: "rgba(0,0,0,.25)" }} />
                    }
                  />
                </Form.Item>
              </>
            )}
          </Form>
        </Modal>
      </Card>
    </div>
  );
}
