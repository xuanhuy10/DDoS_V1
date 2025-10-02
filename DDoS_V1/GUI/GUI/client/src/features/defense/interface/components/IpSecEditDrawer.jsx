import React, { useEffect, useState } from "react";
import {
  CloseOutlined,
  SaveOutlined,
  CaretRightOutlined,
  ClockCircleOutlined,
  SettingOutlined,
} from "@ant-design/icons";
import {
  Button,
  Form,
  message,
  Badge,
  Modal,
  Collapse,
  InputNumber,
  Radio,
  Select,
  Switch,
} from "antd";
import { Flex, Drawer, Input, theme, Space } from "antd";
import {
  updateIpSecProfile,
  getIpSecurity,
} from "@/features/api/IpSecurity.jsx";
import "@/features/defense/interface/styles/main.css";

const IpSecEditDrawer = ({
  openDrawer,
  setOpenDrawer,
  editData,
  onRefreshData,
}) => {
  const [form] = Form.useForm();
  const { token } = theme.useToken();
  const [isChanged, setIsChanged] = useState(false);
  const [initialValues, setInitialValues] = useState({});
  const [isCertificationValid, setIsCertificationValid] = useState(false);
  const [isEnabled, setIsEnabled] = useState(false);
  const [modalVisible, setModalVisible] = useState(false);
  const [modalLoading, setModalLoading] = useState(false);
  const [countdownActive, setCountdownActive] = useState(false);
  const [estimatedTime, setEstimatedTime] = useState(0);
  const [existingProfiles, setExistingProfiles] = useState(null);
  const [profilesLoading, setProfilesLoading] = useState(false);
  const [errorMessage, setErrorMessage] = useState("");

  const localGatewayIPv4 = Form.useWatch("LocalGatewayIPv4", form);
  const localGatewayIPv6 = Form.useWatch("LocalGatewayIPv6", form);
  const connectionCount = Form.useWatch("ConnectionCount", form);

  // Regex patterns
  const ipv4Pattern =
    /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
  const ipv6Pattern =
    /^(([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4})?){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$/;
  const ipv4SubnetPattern = new RegExp(
    ipv4Pattern.source + "(\\/(3[0-2]|[1-2]?[0-9]))?$"
  );
  const ipv6SubnetPattern = new RegExp(
    ipv6Pattern.source + "(\\/([0-9]|[1-9][0-9]|1[0-1][0-9]|12[0-8]))?$"
  );

  // Map encryption algorithms to IP types
  const ipTypeMap = {
    "aes-192": null,
    "aes-256": null,
    "aes-128": null,
    "3des": "IPv4",
    des: "IPv4",
  };
  const isIPv4Only = ipTypeMap[editData?.EncryptionAlgorithm] === "IPv4";
  const isIPv6Only = ipTypeMap[editData?.EncryptionAlgorithm] === "IPv6";

  const panelStyle = {
    marginBottom: 12,
    borderRadius: token.borderRadiusLG,
    border: "1px solid " + token.colorBorder,
    boxShadow: token.shadowLG,
  };

  // Load existing profiles when opening drawer so validator can check uniqueness
  useEffect(() => {
    if (!openDrawer) return;
    let cancelled = false;
    const loadProfiles = async () => {
      setProfilesLoading(true);
      try {
        const res = await getIpSecurity();
        // API might return data in res.data or res.data.data
        const list = (res && res.data && (res.data.data ?? res.data)) || [];
        if (!cancelled) setExistingProfiles(Array.isArray(list) ? list : []);
      } catch (err) {
        console.error(
          "Failed to load existing profiles for uniqueness check:",
          err
        );
        if (!cancelled) setExistingProfiles([]);
      } finally {
        if (!cancelled) setProfilesLoading(false);
      }
    };
    loadProfiles();
    return () => {
      cancelled = true;
    };
  }, [openDrawer]);

  useEffect(() => {
    if (editData && openDrawer) {
      form.resetFields();
      const formData = {
        IpSecProfileName: editData.IpSecProfileName || null,
        IpSecProfileDescription: editData.IpSecProfileDescription || null,
        IKE2Version: editData.IKEversion || null,  // Thay IKEVersion bằng IKE2Version
        ConnectionCount: editData.ConnectionCount || 1,
        IKEMode: editData.IKEMode || null,  // Thay Mode bằng IKEMode
        ESPAHProtocol: editData.Protocol || null,  // Thay Protocol bằng ESPAHProtocol
        IKEReauthTime: editData.IKEReauthTime || null,
        EncryptionAlgorithm: editData.EncryptionAlgorithm || null,
        HashAlgorithm: editData.HashAlgorithm || null,
        RekeyTime: editData.RekeyTime || null,  // Thay ReKeyTime bằng RekeyTime
        LocalGatewayIPv4: editData.LocalGatewayIpAddress?.match(ipv4Pattern) ? editData.LocalGatewayIpAddress : null,
        LocalGatewayIPv6: editData.LocalGatewayIpAddress && !editData.LocalGatewayIpAddress.match(ipv4Pattern) ? editData.LocalGatewayIpAddress : null,
        SubnetLocalGatewayIPv4: editData.SubnetLocalGatewayIpAddress?.match(ipv4Pattern) ? editData.SubnetLocalGatewayIpAddress : null,
        SubnetLocalGatewayIPv6: editData.SubnetLocalGatewayIpAddress && !editData.SubnetLocalGatewayIpAddress.match(ipv4Pattern) ? editData.SubnetLocalGatewayIpAddress : null,
        Enable: editData.Enable ? 1 : 0,
      };

      // Xử lý remote gateways với indexed fields (split bằng ';')
      const cc = editData.ConnectionCount || 1;
      const remoteIps = (editData.RemoteGatewayIpAddress || "").split(';').map(s => s.trim()).filter(s => s);
      const subnetRemotes = (editData.SubnetRemoteGatewayIpAddress || "").split(';').map(s => s.trim()).filter(s => s);
      for (let i = 0; i < cc; i++) {
        const gw = remoteIps[i] || "";
        const sub = subnetRemotes[i] || "";
        const isGwIPv4 = gw && ipv4Pattern.test(gw);
        const isSubIPv4 = sub && ipv4Pattern.test(sub);
        if (cc === 1) {
          formData.RemoteGatewayIPv4 = isGwIPv4 ? gw : null;
          formData.RemoteGatewayIPv6 = !isGwIPv4 ? gw : null;
          formData.SubnetRemoteGatewayIPv4 = isSubIPv4 ? sub : null;
          formData.SubnetRemoteGatewayIPv6 = !isSubIPv4 ? sub : null;
        } else {
          formData[`RemoteGatewayIPv4_${i}`] = isGwIPv4 ? gw : null;
          formData[`RemoteGatewayIPv6_${i}`] = !isGwIPv4 ? gw : null;
          formData[`SubnetRemoteGatewayIPv4_${i}`] = isSubIPv4 ? sub : null;
          formData[`SubnetRemoteGatewayIPv6_${i}`] = !isSubIPv4 ? sub : null;
        }
      }
      const remoteGatewaysInit = Array.from({ length: cc }, (_, i) => {
        const gw = remoteIps[i] || "";
        const sub = subnetRemotes[i] || "";
        const isGwIPv4 = gw && ipv4Pattern.test(gw);
        const isSubIPv4 = sub && ipv4Pattern.test(sub);
        return {
          gatewayIPv4: isGwIPv4 ? gw : undefined,
          gatewayIPv6: !isGwIPv4 ? gw : undefined,
          subnetIPv4: isSubIPv4 ? sub : undefined,
          subnetIPv6: !isSubIPv4 ? sub : undefined,
        };
      });
      form.setFieldsValue({ ...formData, remoteGateways: remoteGatewaysInit });  // Giữ cho UI

      const fullInitial = { ...formData, remoteGateways: remoteGatewaysInit };
      setInitialValues(fullInitial);
      setIsChanged(false);
      setIsCertificationValid(true);
      setIsEnabled(!!editData.Enable);
      console.log("Form data initialized:", fullInitial);
    }
  }, [editData, openDrawer, form]);

  // Sync remoteGateways length with connectionCount
  useEffect(() => {
    if (connectionCount === undefined || connectionCount === null) return;
    const currentList = form.getFieldValue("remoteGateways") || [];
    let newList = [...currentList];
    while (newList.length < connectionCount) {
      newList.push({});
    }
    while (newList.length > connectionCount) {
      newList.pop();
    }
    if (newList.length !== currentList.length) {
      form.setFieldsValue({ remoteGateways: newList });
    }
  }, [connectionCount, form]);

  useEffect(() => {
    if (localGatewayIPv4) form.setFieldsValue({ LocalGatewayIPv6: undefined });
    if (localGatewayIPv6) form.setFieldsValue({ LocalGatewayIPv4: undefined });
  }, [localGatewayIPv4, localGatewayIPv6, form]);

  const handleSwitchChange = (checked) => {
    setIsEnabled(checked);
    form.setFieldsValue({ Enable: checked });
    setIsChanged(true);
  };

  const handleValuesChange = (_, allValues) => {
    // Compare against initialValues (which we set when opening the drawer)
    const hasChanges = Object.keys(allValues).some((key) => {
      const newVal = allValues[key] === undefined ? null : allValues[key];
      const initVal =
        (initialValues && initialValues[key]) === undefined
          ? null
          : initialValues[key];
      return newVal !== initVal;
    });
    setIsChanged(hasChanges);
  };

  const getModifiedValues = (formData) => {
    const modifiedValues = {};
    const ipSecFields = [
      "ProfileName", "ProfileDescription", "ConnectionCount",
      "LocalGateway", "SubnetLocalGateway", "RemoteGateway", "SubnetRemoteGateway",
      "IKEVersion", "Mode", "ESPAHProtocol", "IKEReauthTime",
      "EncryptionAlgorithm", "HashAlgorithm", "RekeyTime", "Enable",
    ];

    // Map normalizedEditData với tên backend (từ editData)
    const normalizedEditData = {
      ProfileName: editData.IpSecProfileName || null,
      ProfileDescription: editData.IpSecProfileDescription || null,
      ConnectionCount: editData.ConnectionCount || 1,
      LocalGateway: editData.LocalGatewayIpAddress || null,
      SubnetLocalGateway: editData.SubnetLocalGatewayIpAddress || null,
      RemoteGateway: editData.RemoteGatewayIpAddress || null,
      SubnetRemoteGateway: editData.SubnetRemoteGatewayIpAddress || null,
      IKEVersion: editData.IKEversion || null,  // Map từ IKEversion
      Mode: editData.IKEMode || null,
      ESPAHProtocol: editData.Protocol || null,  // Map từ Protocol
      IKEReauthTime: editData.IKEReauthTime || null,
      EncryptionAlgorithm: editData.EncryptionAlgorithm || null,
      HashAlgorithm: editData.HashAlgorithm || null,
      RekeyTime: editData.RekeyTime || null,  // Map từ RekeyTime
      Enable: editData.Enable ? 1 : 0,
    };

    let changedFieldsCount = 0;
    ipSecFields.forEach((key) => {
      const formValue = formData[key] === undefined ? null : formData[key];
      const editValue = normalizedEditData[key];
      if (formValue !== editValue) {
        modifiedValues[key] = formValue;
        changedFieldsCount++;
        console.log(`Field changed: ${key}, Old: ${editValue}, New: ${formValue}`);
      }
    });
    console.log("Changed fields count:", changedFieldsCount);
    return { modifiedValues, changedFieldsCount };
  };

  const handleFinish = async (values) => {
    const connectionCount = values.ConnectionCount || 1;
    const remoteGatewaysList = values.remoteGateways || [];  // Từ Form.List UI

    // Map từ array sang indexed cho đồng bộ
    const indexedValues = { ...values };
    remoteGatewaysList.forEach((item, index) => {
      if (connectionCount > 1) {
        indexedValues[`RemoteGatewayIPv4_${index}`] = item.gatewayIPv4 || null;
        indexedValues[`RemoteGatewayIPv6_${index}`] = item.gatewayIPv6 || null;
        indexedValues[`SubnetRemoteGatewayIPv4_${index}`] = item.subnetIPv4 || null;
        indexedValues[`SubnetRemoteGatewayIPv6_${index}`] = item.subnetIPv6 || null;
      } else {
        indexedValues.RemoteGatewayIPv4 = item.gatewayIPv4 || null;
        indexedValues.RemoteGatewayIPv6 = item.gatewayIPv6 || null;
        indexedValues.SubnetRemoteGatewayIPv4 = item.subnetIPv4 || null;
        indexedValues.SubnetRemoteGatewayIPv6 = item.subnetIPv6 || null;
      }
    });

    const profileData = {
      ProfileName: indexedValues.IpSecProfileName,
      ProfileDescription: indexedValues.IpSecProfileDescription,
      LocalGateway: indexedValues.LocalGatewayIPv4 || indexedValues.LocalGatewayIPv6 || editData.LocalGatewayIpAddress || "",
      SubnetLocalGateway: indexedValues.SubnetLocalGatewayIPv4 || indexedValues.SubnetLocalGatewayIPv6 || editData.SubnetLocalGatewayIpAddress || "",
      RemoteGateway: connectionCount === 1
          ? (indexedValues.RemoteGatewayIPv4 || indexedValues.RemoteGatewayIPv6)
          : Array.from({ length: connectionCount }, (_, index) =>
              indexedValues[`RemoteGatewayIPv4_${index}`] || indexedValues[`RemoteGatewayIPv6_${index}`]
          ).filter(Boolean).join(';'),  // Join bằng ';'
      SubnetRemoteGateway: connectionCount === 1
          ? (indexedValues.SubnetRemoteGatewayIPv4 || indexedValues.SubnetRemoteGatewayIPv6)
          : Array.from({ length: connectionCount }, (_, index) =>
              indexedValues[`SubnetRemoteGatewayIPv4_${index}`] || indexedValues[`SubnetRemoteGatewayIPv6_${index}`]
          ).filter(Boolean).join(';'),  // Join bằng ';'
      IKEVersion: indexedValues.IKE2Version,  // Tên đúng
      Mode: indexedValues.IKEMode,  // Tên đúng
      ESPAHProtocol: indexedValues.ESPAHProtocol,  // Tên đúng
      IKEReauthTime: indexedValues.IKEReauthTime ? parseInt(indexedValues.IKEReauthTime, 10) : null,
      EncryptionAlgorithm: indexedValues.EncryptionAlgorithm,
      HashAlgorithm: indexedValues.HashAlgorithm,
      RekeyTime: indexedValues.RekeyTime ? parseInt(indexedValues.RekeyTime, 10) : null,  // Tên đúng, parseInt
      Enable: isEnabled ? 1 : 0,
      ConnectionCount: connectionCount,
    };

    const { modifiedValues, changedFieldsCount } = getModifiedValues(profileData);  // Cập nhật hàm này ở bước 3

    console.log("profileData gửi backend:", profileData);

    setModalVisible(true);
    setEstimatedTime(Math.max(changedFieldsCount * 2, 2));
    setCountdownActive(true);
    startCountdownAndUpdate(modifiedValues, profileData);  // Truyền profileData đầy đủ
  };

  const startCountdownAndUpdate = async (modifiedValues, profileData) => {
    setModalLoading(true);
    const countdown = setInterval(() => {
      setEstimatedTime((prev) => {
        if (prev <= 0) return 5;
        return prev - 1;
      });
    }, 1000);

    try {
      // Map nếu backend expect tên khác (dựa trên create, gửi trực tiếp profileData)
      const payload = {
        ...modifiedValues,
        ...profileData,  // Merge để đầy đủ
      };
      const updateResponse = await updateIpSecProfile(editData.IpSecProfileId, payload);

      clearInterval(countdown);
      setCountdownActive(false);
      setEstimatedTime(0);
      setModalLoading(false);
      setModalVisible(false);

      if (updateResponse.status === 200 && updateResponse.data.success) {
        await onRefreshData();
        setInitialValues(profileData);
        setIsChanged(false);
        message.success("IPSec profile updated successfully");
        setOpenDrawer(false);
      } else {
        throw new Error(updateResponse.data.message || "Failed to update IPSec profile");
      }
    } catch (error) {
      clearInterval(countdown);
      setCountdownActive(false);
      setEstimatedTime(0);
      setModalLoading(false);
      setErrorMessage(error.response?.data?.message || error.message || "Unknown error occurred");
      setModalVisible(true);  // Giữ modal để hiển thị lỗi
    }
  };

  const localGatewayValidator = {
    validator: async () => {
      const ipv4 = form.getFieldValue("LocalGatewayIPv4");
      const ipv6 = form.getFieldValue("LocalGatewayIPv6");
      if (!ipv4 && !ipv6) {
        return Promise.reject(
          new Error("Please set your local gateway IP address!")
        );
      }
      if (isIPv4Only && !ipv4) {
        return Promise.reject(
          new Error("IPv4 address required for this encryption algorithm!")
        );
      }
      if (isIPv6Only && !ipv6) {
        return Promise.reject(
          new Error("IPv6 address required for this encryption algorithm!")
        );
      }
      return Promise.resolve();
    },
  };

  return (
    <Drawer
      title={
        <Flex align="center" justify="space-between" gap="small">
          <span>
            Edit IPSec Profile:{" "}
            <strong>{editData?.IpSecProfileName || "Unknown"}</strong>
          </span>
          <Space>
            <Badge
              status={
                editData?.IpSecProfileStatus === "Active"
                  ? "processing"
                  : "default"
              }
            />
            <Badge
              status={
                editData?.CertificationStatus === "Valid" ? "success" : "error"
              }
              text={`Cert: ${editData?.CertificationStatus || "Unknown"}`}
            />
            <Switch
              onChange={handleSwitchChange}
              checked={isEnabled}
              checkedChildren="Enabled"
              unCheckedChildren="Disabled"
            />
          </Space>
        </Flex>
      }
      placement="right"
      closable={true}
      onClose={() => setOpenDrawer(false)}
      open={openDrawer}
      width={800}
      className="drawer"
    >
      <Form
        layout="vertical"
        colon={false}
        requiredMark={false}
        form={form}
        onFinish={handleFinish}
        onValuesChange={handleValuesChange}
      >
        <div style={{ padding: "0" }}>
          <Flex vertical gap="middle">
            <Collapse
              style={{ backgroundColor: "white" }}
              bordered={false}
              expandIconPosition="end"
              expandIcon={({ isActive }) => (
                <CaretRightOutlined rotate={isActive ? 90 : 0} />
              )}
              size="small"
              defaultActiveKey={["general", "ike"]}
              items={[
                {
                  key: "general",
                  label: <span className="collapse-title">Node Settings</span>,
                  style: panelStyle,
                  children: (
                    <>
                      <Form.Item
                        name="IpSecProfileName"
                        label="Profile Name"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please set your IPSec profile name!",
                          },
                          {
                            // async validator using cached existingProfiles
                            validator: async (_, value) => {
                              if (!value) return Promise.resolve();
                              const name = value.trim().toLowerCase();

                              // If profiles are still loading, attempt to fetch synchronously
                              if (
                                existingProfiles === null &&
                                !profilesLoading
                              ) {
                                try {
                                  const res = await getIpSecurity();
                                  const list =
                                    (res &&
                                      res.data &&
                                      (res.data.data ?? res.data)) ||
                                    [];
                                  setExistingProfiles(
                                    Array.isArray(list) ? list : []
                                  );
                                } catch (err) {
                                  console.error(
                                    "Error fetching profiles inside validator:",
                                    err
                                  );
                                  // Fail open: allow validation (avoid blocking user) if fetch fails
                                  return Promise.resolve();
                                }
                              }

                              const listToCheck = existingProfiles || [];

                              const exists = listToCheck.some((item) => {
                                const itemName = (item.IpSecProfileName || "")
                                  .trim()
                                  .toLowerCase();
                                const sameName = itemName === name;
                                const sameId =
                                  item.IpSecProfileId ===
                                  editData?.IpSecProfileId;
                                return sameName && !sameId; // duplicate with different record -> invalid
                              });

                              if (exists) {
                                return Promise.reject(
                                  new Error(
                                    "Profile name already exists, please choose another one!"
                                  )
                                );
                              }

                              return Promise.resolve();
                            },
                          },
                        ]}
                      >
                        <Input placeholder="Enter profile name" />
                      </Form.Item>

                      <Form.Item
                        name="IpSecProfileDescription"
                        label="Profile Description"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message:
                              "Please provide a description for your IPSec profile!",
                          },
                        ]}
                      >
                        <Input.TextArea placeholder="Enter profile description" />
                      </Form.Item>

                      <Form.Item
                        name="ConnectionCount"
                        label="Connection Count"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please set connection count!",
                          },
                        ]}
                      >
                        <InputNumber
                          min={1}
                          max={10}
                          style={{ width: "100%" }}
                        />
                      </Form.Item>

                      <Form.Item
                        name="LocalGateway"
                        label="Local Gateway IP Address"
                        rules={[localGatewayValidator]}
                      >
                        <Input.Group compact>
                          <Form.Item
                            name="LocalGatewayIPv4"
                            noStyle
                            rules={[
                              {
                                pattern: ipv4Pattern,
                                message: "Invalid IPv4 address",
                              },
                            ]}
                          >
                            <Input
                              style={{ width: "50%" }}
                              placeholder="IPv4 address"
                              disabled={isIPv6Only || !!localGatewayIPv6}
                            />
                          </Form.Item>
                          <Form.Item
                            name="SubnetLocalGatewayIPv4"
                            noStyle
                            rules={[
                              {
                                pattern: ipv4SubnetPattern,
                                message:
                                  "Invalid IPv4 subnet (e.g., 10.0.0.0/24)",
                              },
                            ]}
                          >
                            <Input
                              style={{ width: "50%" }}
                              placeholder="Subnet Local Gateway IPv4"
                              disabled={!!localGatewayIPv6}
                            />
                          </Form.Item>
                        </Input.Group>
                        <Input.Group compact>
                          <Form.Item
                            name="LocalGatewayIPv6"
                            noStyle
                            rules={[
                              {
                                pattern: ipv6Pattern,
                                message: "Invalid IPv6 address",
                              },
                            ]}
                          >
                            <Input
                              style={{ width: "50%" }}
                              placeholder="IPv6 address"
                              disabled={isIPv4Only || !!localGatewayIPv4}
                            />
                          </Form.Item>
                          <Form.Item
                            name="SubnetLocalGatewayIPv6"
                            noStyle
                            rules={[
                              {
                                pattern: ipv6SubnetPattern,
                                message:
                                  "Invalid IPv6 subnet (e.g., 2001:db8::/32)",
                              },
                            ]}
                          >
                            <Input
                              style={{ width: "50%" }}
                              placeholder="Subnet Local Gateway IPv6"
                              disabled={!!localGatewayIPv4}
                            />
                          </Form.Item>
                        </Input.Group>
                      </Form.Item>

                      <Form.List name="remoteGateways">
                        {(fields, { add, remove }) => (
                          <>
                            {fields.map(
                              ({ key, name, ...restField }, index) => (
                                <Form.Item
                                  key={key}
                                  label={`Remote Gateway IP Address #${
                                    index + 1
                                  }`}
                                  required
                                  rules={[
                                    {
                                      validator: () => {
                                        const g4 = form.getFieldValue([
                                          "remoteGateways",
                                          index,
                                          "gatewayIPv4",
                                        ]);
                                        const g6 = form.getFieldValue([
                                          "remoteGateways",
                                          index,
                                          "gatewayIPv6",
                                        ]);
                                        if (!g4 && !g6) {
                                          return Promise.reject(
                                            new Error(
                                              `Please set your remote gateway IP address #${
                                                index + 1
                                              }!`
                                            )
                                          );
                                        }
                                        if (isIPv4Only && !g4) {
                                          return Promise.reject(
                                            new Error(
                                              "IPv4 address required for this encryption algorithm!"
                                            )
                                          );
                                        }
                                        if (isIPv6Only && !g6) {
                                          return Promise.reject(
                                            new Error(
                                              "IPv6 address required for this encryption algorithm!"
                                            )
                                          );
                                        }
                                        return Promise.resolve();
                                      },
                                    },
                                  ]}
                                >
                                  <Input.Group compact>
                                    <Form.Item
                                      {...restField}
                                      name={[name, "gatewayIPv4"]}
                                      noStyle
                                      rules={[
                                        {
                                          pattern: ipv4Pattern,
                                          message: "Invalid IPv4 address",
                                        },
                                      ]}
                                    >
                                      <Input
                                        style={{ width: "50%" }}
                                        placeholder="IPv4 address"
                                        disabled={
                                          isIPv6Only ||
                                          !!form.getFieldValue([
                                            "remoteGateways",
                                            index,
                                            "gatewayIPv6",
                                          ])
                                        }
                                      />
                                    </Form.Item>
                                    <Form.Item
                                      {...restField}
                                      name={[name, "subnetIPv4"]}
                                      noStyle
                                      rules={[
                                        {
                                          pattern: ipv4SubnetPattern,
                                          message:
                                            "Invalid IPv4 subnet (e.g., 10.0.0.0/24)",
                                        },
                                      ]}
                                    >
                                      <Input
                                        style={{ width: "50%" }}
                                        placeholder="Subnet Remote Gateway IPv4"
                                        disabled={
                                          isIPv6Only ||
                                          !!form.getFieldValue([
                                            "remoteGateways",
                                            index,
                                            "gatewayIPv6",
                                          ])
                                        }
                                      />
                                    </Form.Item>
                                  </Input.Group>

                                  <Input.Group compact>
                                    <Form.Item
                                      {...restField}
                                      name={[name, "gatewayIPv6"]}
                                      noStyle
                                      rules={[
                                        {
                                          pattern: ipv6Pattern,
                                          message: "Invalid IPv6 address",
                                        },
                                      ]}
                                    >
                                      <Input
                                        style={{ width: "50%" }}
                                        placeholder="IPv6 address"
                                        disabled={
                                          isIPv4Only ||
                                          !!form.getFieldValue([
                                            "remoteGateways",
                                            index,
                                            "gatewayIPv4",
                                          ])
                                        }
                                      />
                                    </Form.Item>
                                    <Form.Item
                                      {...restField}
                                      name={[name, "subnetIPv6"]}
                                      noStyle
                                      rules={[
                                        {
                                          pattern: ipv6SubnetPattern,
                                          message:
                                            "Invalid IPv6 subnet (e.g., 2001:db8::/32)",
                                        },
                                      ]}
                                    >
                                      <Input
                                        style={{ width: "50%" }}
                                        placeholder="Subnet Remote Gateway IPv6"
                                        disabled={
                                          isIPv4Only ||
                                          !!form.getFieldValue([
                                            "remoteGateways",
                                            index,
                                            "gatewayIPv4",
                                          ])
                                        }
                                      />
                                    </Form.Item>
                                  </Input.Group>
                                </Form.Item>
                              )
                            )}
                          </>
                        )}
                      </Form.List>
                    </>
                  ),
                },
                {
                  key: "ike",
                  label: <span className="collapse-title">IKE Settings</span>,
                  style: panelStyle,
                  children: (
                    <>
                      <Form.Item
                        label="IKE Version"
                        name="IKEVersion"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please set IKE version!",
                          },
                        ]}
                      >
                        <Input placeholder="Enter IKE version" />
                      </Form.Item>
                      <Form.Item
                        label="IKE Mode"
                        name="Mode"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please choose IKE mode!",
                          },
                        ]}
                      >
                        <Radio.Group>
                          <Radio value="tunnel">Tunnel Mode</Radio>
                          <Radio value="transport">Transport Mode</Radio>
                        </Radio.Group>
                      </Form.Item>
                      <Form.Item
                        label="Protocol"
                        name="ESPAHProtocol"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please choose protocol!",
                          },
                        ]}
                      >
                        <Radio.Group>
                          <Radio value="ESP">ESP Protocol</Radio>
                          <Radio value="AH">AH Protocol</Radio>
                        </Radio.Group>
                      </Form.Item>
                      <Form.Item
                        label="IKE Reauth Time"
                        name="IKEReauthTime"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please set IKE reauth time!",
                          },
                        ]}
                      >
                        <InputNumber
                          min={1}
                          max={200000}
                          addonAfter="s"
                          style={{ width: "100%" }}
                          prefix={<ClockCircleOutlined />}
                        />
                      </Form.Item>
                      <Form.Item
                        label="Encryption Algorithm"
                        name="EncryptionAlgorithm"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please choose encryption algorithm!",
                          },
                        ]}
                      >
                        <Select placeholder="Select encryption algorithm">
                          <Select.Option value="aes-192">AES-192</Select.Option>
                          <Select.Option value="aes-256">AES-256</Select.Option>
                          <Select.Option value="aes-128">AES-128</Select.Option>
                          <Select.Option value="3des">3DES</Select.Option>
                          <Select.Option value="des">DES</Select.Option>
                        </Select>
                      </Form.Item>
                      <Form.Item
                        label="Hash Algorithm"
                        name="HashAlgorithm"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please choose hash algorithm!",
                          },
                        ]}
                      >
                        <Select placeholder="Select hash algorithm">
                          <Select.Option value="md5">MD5</Select.Option>
                          <Select.Option value="sha1">SHA-1</Select.Option>
                          <Select.Option value="sha256">SHA-256</Select.Option>
                          <Select.Option value="sha384">SHA-384</Select.Option>
                          <Select.Option value="sha512">SHA-512</Select.Option>
                        </Select>
                      </Form.Item>
                      <Form.Item
                        label="Re-key Time"
                        name="ReKeyTime"
                        layout="horizontal"
                        labelAlign="left"
                        labelCol={{ sm: { span: 12 } }}
                        wrapperCol={{ sm: { span: 12 } }}
                        rules={[
                          {
                            required: true,
                            message: "Please set re-key time!",
                          },
                        ]}
                      >
                        <InputNumber
                          min={1}
                          max={200000}
                          addonAfter="s"
                          style={{ width: "100%" }}
                          prefix={<ClockCircleOutlined />}
                        />
                      </Form.Item>
                    </>
                  ),
                },
              ]}
            />
          </Flex>
          <Flex
            justify="space-between"
            align="center"
            style={{
              marginTop: "32px",
              paddingTop: "24px",
              borderTop: "1px solid #f0f0f0",
            }}
          >
            <div>
              <Button
                type="default"
                icon={<CloseOutlined />}
                onClick={() => setOpenDrawer(false)}
                style={{ marginRight: 8 }}
              >
                Cancel
              </Button>
              <Button
                type="primary"
                htmlType="submit"
                icon={<SaveOutlined />}
                disabled={!isChanged || !isCertificationValid}
                style={{ backgroundColor: "#52c41a", borderColor: "#52c41a" }}
              >
                Save Changes
              </Button>
            </div>
          </Flex>
        </div>
      </Form>

      <Modal
        title="Save Changes"
        open={modalVisible}
        onCancel={() => {
          setModalVisible(false);
          setCountdownActive(false);
          setEstimatedTime(0);
          setErrorMessage(""); // Xóa thông điệp lỗi khi đóng modal
          message.info("Update cancelled");
        }}
        cancelText="Cancel"
        footer={[
          <Button
            key="cancel"
            onClick={() => {
              setModalVisible(false);
              setCountdownActive(false);
              setEstimatedTime(0);
              setErrorMessage(""); // Xóa thông điệp lỗi khi đóng modal
              message.info("Update cancelled");
            }}
          >
            Cancel
          </Button>,
        ]}
      >
        {modalLoading ? (
          <>
            <p>Updating IPSec profile...</p>
            {editData?.IpSecProfileStatus === "Active" && (
              <p>
                <strong style={{ color: "red" }}>Note:</strong> This profile is
                currently active. Changes are being applied.
              </p>
            )}
            {!isCertificationValid && (
              <p>
                <strong style={{ color: "orange" }}>Warning:</strong>{" "}
                Certificate files are invalid or missing. Please upload valid
                certificates.
              </p>
            )}
            {countdownActive && (
              <div
                style={{
                  display: "flex",
                  alignItems: "center",
                  justifyContent: "center",
                  marginTop: 16,
                }}
              >
                <SettingOutlined
                  spin
                  style={{ fontSize: 20, color: "#1890ff", marginRight: 8 }}
                />
                <p style={{ color: "#888", margin: 0 }}>
                  Estimated time remaining: {estimatedTime}s
                </p>
              </div>
            )}
          </>
        ) : (
          <>
            <p style={{ color: "#ff4d4f", fontWeight: "bold" }}>
              Failed to update IPSec profile
            </p>
            <p style={{ color: "#888" }}>{errorMessage}</p>{" "}
            {/* Hiển thị thông điệp lỗi từ backend */}
            {editData?.IpSecProfileStatus === "Active" && (
              <p>
                <strong style={{ color: "red" }}>Note:</strong> This profile is
                currently active. Changes are being applied.
              </p>
            )}
            {!isCertificationValid && (
              <p>
                <strong style={{ color: "orange" }}>Warning:</strong>{" "}
                Certificate files are invalid or missing. Please upload valid
                certificates.
              </p>
            )}
          </>
        )}
      </Modal>
    </Drawer>
  );
};

export default IpSecEditDrawer;
