import React, { useEffect, useState } from "react";
import PropTypes from "prop-types";
import { Modal, Form, Input, Select, message } from "antd";
import { SettingOutlined } from "@ant-design/icons";

import {
  insertProtectedAddresses,
  insertBlockedAddresses,
  insertVPNAllowedAddresses,
  insertHTTPBlockedAddresses,
} from "@/features/api/NetworkAddresses";
import { getAllDeviceInterfaces } from "@/features/api/DeviceInterfaces";

export const AddressInsert = ({
  type,
  onCancel,
  isModalOpen,
  onCreate,
  isImportLoading,
  estimatedTime,
  countdown,
}) => {
  const [form] = Form.useForm();
  const [interfaces, setInterfaces] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [addClicked, setAddClicked] = useState(false);

  useEffect(() => {
    if (isModalOpen) {
      form.resetFields();
      setAddClicked(false);
    }
  }, [isModalOpen, form]);

  useEffect(() => {
    const fetchInterfaces = async () => {
      try {
        const response = await getAllDeviceInterfaces();
        setInterfaces(response.data);
      } catch (error) {
        message.error("Failed to fetch interfaces. Please try again later.");
      }
    };
    fetchInterfaces();
  }, []);

  const handleCreate = async (values) => {
    setIsLoading(true);
    try {
      switch (type) {
        case "protected":
          await insertProtectedAddresses(values);
          break;
        case "blocked":
          await insertBlockedAddresses(values);
          break;
        case "vpn_white":
          await insertVPNAllowedAddresses(values);
          break;
        case "http_black":
          await insertHTTPBlockedAddresses(values);
          break;
        default:
          throw new Error("Invalid type provided!");
      }
      message.success("Address created successfully!");
      form.resetFields();
      onCancel();
    } catch (error) {
      message.error(
        error.response?.data?.message ||
          "Failed to create address. Please try again."
      );
      onCancel();
    } finally {
      setIsLoading(false);
      setAddClicked(false);
    }
  };

  return (
    <Modal
      title="Add a new IP Address"
      okText="Add"
      open={isModalOpen}
      cancelText="Cancel"
      confirmLoading={isImportLoading || isLoading}
      onCancel={onCancel}
      onOk={() => {
        if (isImportLoading || isLoading || addClicked) return;
        setAddClicked(true);
        form
          .validateFields()
          .then((values) => {
            let portName = null;
            if (
              (type === "protected" || type === "blocked") &&
              values.InterfaceId &&
              interfaces.length > 0
            ) {
              const selected = interfaces.find(
                (i) => i.InterfaceId === values.InterfaceId
              );
              portName = selected ? selected.InterfaceName : null;
            }
            onCreate({
              ...values,
              Port: portName,
            });
          })
          .catch(() => {
            message.error("Please fill in all required fields!");
            setAddClicked(false);
          });
      }}
      okButtonProps={{ disabled: isImportLoading || isLoading || addClicked }}
      style={{ top: 20 }}
      width={500}
    >
      <Form
        form={form}
        layout="vertical"
        name="form_in_modal"
        initialValues={{ modifier: "public" }}
      >
        <Form.Item
          name="AddressVersion"
          label="Version"
          rules={[{ required: true, message: "Please select the ip version!" }]}
        >
          <Select
            style={{
              width: "100%",
              borderRadius: "6px",
              borderColor: "#d9d9d9",
              boxShadow: "0 2px 4px rgba(0,0,0,0.05)",
            }}
          >
            <Select.Option value="IPv4">IPv4</Select.Option>
            <Select.Option value="IPv6">IPv6</Select.Option>
          </Select>
        </Form.Item>

        {type === "protected" || type === "blocked" ? (
          <Form.Item
            name="InterfaceId"
            label="Attach to Interface"
            rules={[
              { required: true, message: "Please select the address type!" },
            ]}
          >
            <Select
              style={{
                width: "100%",
                borderRadius: "6px",
                borderColor: "#d9d9d9",
                boxShadow: "0 2px 4px rgba(0,0,0,0.05)",
              }}
            >
              {interfaces.map((iface) => (
                <Select.Option
                  key={iface.InterfaceId}
                  value={iface.InterfaceId}
                >
                  {iface.InterfaceName}
                </Select.Option>
              ))}
            </Select>
          </Form.Item>
        ) : null}

        <Form.Item
          name="Address"
          label="IP Address"
          dependencies={["AddressVersion"]}
          rules={[
            { required: true, message: "Please input the IP address!" },
            ({ getFieldValue }) => ({
              validator(_, value) {
                const ipVersion = getFieldValue("AddressVersion");
                const ipv4Pattern =
                  /^(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.((25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\.){2}(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)$/;
                const ipv6Pattern =
                  /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9]))$/;

                if (!ipVersion)
                  return Promise.reject(
                    new Error("Please select the IP version!")
                  );
                if (ipVersion === "IPv4" && !ipv4Pattern.test(value)) {
                  return Promise.reject(
                    new Error("Please enter a valid IPv4 address!")
                  );
                }
                if (ipVersion === "IPv6" && !ipv6Pattern.test(value)) {
                  return Promise.reject(
                    new Error("Please enter a valid IPv6 address!")
                  );
                }
                return Promise.resolve();
              },
            }),
          ]}
        >
          <Input
            onChange={() => setAddClicked(false)}
            style={{
              borderRadius: "6px",
              borderColor: "#d9d9d9",
              boxShadow: "0 2px 4px rgba(0,0,0,0.05)",
            }}
          />
        </Form.Item>
        {isImportLoading && estimatedTime > 0 && (
          <div
            style={{
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
              marginTop: 16,
              marginBottom: 16,
              backgroundColor: "#f6ffed",
              borderRadius: "6px",
              padding: "12px",
              borderLeft: "3px solid #52c41a",
            }}
          >
            <SettingOutlined
              spin={isImportLoading}
              style={{ fontSize: 20, color: "#52c41a", marginRight: 8 }}
            />
            <p style={{ color: "#52c41a", margin: 0, fontWeight: "bold" }}>
              {countdown > 0 && <span> Estimated Time: {countdown}s</span>}
            </p>
          </div>
        )}
      </Form>
    </Modal>
  );
};

AddressInsert.propTypes = {
  onCreate: PropTypes.func.isRequired,
  onCancel: PropTypes.func.isRequired,
  isModalOpen: PropTypes.bool.isRequired,
  isImportLoading: PropTypes.bool,
  estimatedTime: PropTypes.number,
  countdown: PropTypes.number,
};

export default AddressInsert;
