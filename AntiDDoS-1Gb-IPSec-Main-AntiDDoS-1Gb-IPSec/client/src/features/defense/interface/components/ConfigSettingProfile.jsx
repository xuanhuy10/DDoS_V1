import React, { useState, useEffect } from "react";
import { DeleteOutlined, PlusOutlined, SaveOutlined } from "@ant-design/icons";
import {
  Table,
  Button,
  Space,
  Modal,
  Form,
  Input,
  message,
  Badge,
  Tooltip,
  Alert,
  Tag,
  Tabs,
} from "antd";
import { MonitorOutlined } from "@ant-design/icons";
import { getAllDeviceInterfaces } from "@/features/api/DeviceInterfaces";
import { getDefenseProfiles } from "@/features/api/DefenseProfiles";
import { getIpSecurity } from "@/features/api/IpSecurity";
import { socket } from "@/utils/socket";
import TabPane from "antd/es/tabs/TabPane";

const ConfigSettingProfile = () => {
  const [profiles, setProfiles] = useState([]);
  const [isModalVisible, setIsModalVisible] = useState(false);
  const [form] = Form.useForm();
  const [editingId, setEditingId] = useState(null);
  const [isLoading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [interfaces, setInterfaces] = useState([]);
  const [managementProfiles, setManagementProfiles] = useState([]);
  const [ipsecProfiles, setIpsecProfiles] = useState([]);
  const [interfaceStatusData, setInterfaceStatusData] = useState(null);
  const [interfaceOneSecData, setInterfaceOneSecData] = useState(null);
  const [expandedRowKeys, setExpandedRowKeys] = useState([]);

  useEffect(() => {
    setProfiles([
      {
        id: 1,
        name: "Default Profile",
        ddos: "Default DDoS configuration",
        ipsec: "Default IPSec configuration",
        status: true,
      },
      {
        id: 2,
        name: "Custom Profile",
        ddos: "Custom DDoS configuration",
        ipsec: "Custom IPSec configuration",
        status: false,
      },
    ]);
  }, []);

  // Fetch interfaces, management profiles, and IPSec profiles
  const fetchData = async () => {
    try {
      setError(null);
      setLoading(true);
      const [interfaceResponse, managementProfileResponse, ipsecResponse] =
        await Promise.all([
          getAllDeviceInterfaces(),
          getDefenseProfiles(),
          getIpSecurity(),
        ]);
      const rawInterfaces = interfaceResponse.data;
      setInterfaces(rawInterfaces);
      setManagementProfiles(managementProfileResponse.data);
      setIpsecProfiles(ipsecResponse.data || []);
    } catch (error) {
      console.error("fetchData error:", error);
      setError("Unable to fetch data, please try again later.");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  // Socket handler for real-time status data
  const handleInterfaceData = (status, oneSec) => {
    setInterfaceStatusData(status);
    setInterfaceOneSecData(oneSec);
  };

  // Listen to socket for interface status updates
  useEffect(() => {
    socket.on("interface", handleInterfaceData);
    return () => {
      socket.off("interface", handleInterfaceData);
    };
  }, []);

  // Columns for the main profiles table
  const columns = [
    Table.SELECTION_COLUMN,
    {
      title: "Name",
      dataIndex: "name",
      key: "name",
      width: "15%",
    },
    {
      title: "DDoS",
      dataIndex: "ddos",
      key: "ddos",
      width: "30%",
    },
    Table.EXPAND_COLUMN,
    {
      title: "IPSec",
      dataIndex: "ipsec",
      key: "ipsec",
      width: "25%",
    },
    Table.EXPAND_COLUMN,
    {
      title: "Actions",
      key: "actions",
      render: (_, record) => {
        const status = !!record.status;
        return (
          <Space>
            <Tooltip
              title={status ? "Apply this profile" : "Profile is inactive"}
            >
              <Button
                type="primary"
                icon={<MonitorOutlined />}
                onClick={() => handleApply(record)}
              ></Button>
            </Tooltip>
            <Tooltip title="Save profile">
              <Button
                icon={<SaveOutlined />}
                onClick={() => handleEdit(record)}
                disabled={!status}
              ></Button>
            </Tooltip>
            <Tooltip title="Delete profile">
              <Button
                danger
                icon={<DeleteOutlined />}
                onClick={() => handleDelete(record.id)}
                disabled={status}
              ></Button>
            </Tooltip>
            <Badge>
              <span>
                {status ? (
                  <Tag color="green">Active</Tag>
                ) : (
                  <Tag color="red">Inactive</Tag>
                )}
              </span>
            </Badge>
          </Space>
        );
      },
    },
  ];

  // Sub-columns for DDoS expandable
  const ddosInterfaceCols = [
    {
      title: "Interface",
      dataIndex: "InterfaceName",
      key: "interface",
      render: (text) => {
        return <span>{text}</span>;
      },
    },
    {
      title: "Interface Zone",
      dataIndex: "InterfaceType",
      key: "interfaceType",
      align: "center",
      render: (text) => <Tag>{text || "inside"}</Tag>,
    },
    {
      title: "Protection Mode",
      dataIndex: "InterfaceProtectionMode",
      align: "center",
      key: "protectionMode",
      render: (text) => <Tag>{text || "Port"}</Tag>,
    },
    {
      title: "Management Profile",
      key: "defenseProfile",
      align: "center",
      render: (text, record) => {
        const profile = managementProfiles.find(
          (p) => p.DefenseProfileId === record.DefenseProfileId
        );
        return <Tag>{profile ? profile.DefenseProfileName : "None"}</Tag>;
      },
    },
    {
      title: "Tag",
      key: "tag",
      align: "center",
      render: (text, record) => (
        <Space>
          {record.InterfaceIsMirroring === 1 && (
            <Tag color="blue">Mirroring</Tag>
          )}
          {record.InterfaceIsMonitoring === 1 && (
            <Tag color="green">Monitoring</Tag>
          )}
        </Space>
      ),
    },
  ];

  // IPSec columns for expand
  const ipsecCols = [
    {
      title: "IPSec Profile Name",
      dataIndex: "ProfileName",
      key: "name",
      width: 150,
    },
    {
      title: "Connection",
      dataIndex: "ConnectionCount",
      key: "connectionCount",
      align: "center",
      width: 100,
      render: (text) => <Tag>{text || 1}</Tag>,
    },
    {
      title: "Local Gateway",
      dataIndex: "LocalGateway",
      key: "localGateway",
      align: "center",
      width: 150,
    },
    {
      title: "Subnet Local Gateway",
      dataIndex: "SubnetLocalGateway",
      key: "subnetLocalGateway",
      align: "center",
      width: 180,
    },
    {
      title: "Remote Gateway",
      dataIndex: "RemoteGateway",
      key: "remoteGateway",
      align: "center",
      width: 150,
    },
    {
      title: "Subnet Remote Gateway",
      dataIndex: "SubnetRemoteGateway",
      key: "subnetRemoteGateway",
      align: "center",
      width: 180,
    },
    {
      title: "IKE Version",
      dataIndex: "IKEVersion",
      align: "center",
      key: "ikeVersion",
      width: 120,
    },
    {
      title: "IKE Mode",
      dataIndex: "Mode",
      align: "center",
      key: "ikeMode",
      width: 120,
      render: (text) => <Tag>{text || "N/A"}</Tag>,
    },
    {
      title: "Protocol",
      dataIndex: "ESPAHProtocol",
      key: "protocol",
      align: "center",
      width: 120,
      render: (text) => <Tag>{text?.toUpperCase()}</Tag>,
    },
    {
      title: "IKE Reauth Time",
      dataIndex: "IKEReauthTime",
      align: "center",
      key: "ikeReauthTime",
      width: 150,
    },
    {
      title: "Encryption Algorithm",
      dataIndex: "EncryptionAlgorithm",
      align: "center",
      key: "encryptionAlgorithm",
      width: 150,
      render: (text) => <Tag>{text?.toUpperCase()}</Tag>,
    },
    {
      title: "Hash Algorithm",
      dataIndex: "HashAlgorithm",
      align: "center",
      key: "hashAlgorithm",
      width: 150,
      render: (text) => <Tag>{text?.toUpperCase()}</Tag>,
    },
    {
      title: "Re-key Time",
      dataIndex: "ReKeyTime",
      align: "center",
      key: "rekeyTime",
      width: 120,
    },
    {
      title: "Status",
      dataIndex: "Enable",
      align: "center",
      key: "status",
      width: 100,
      render: (enable) => (
        <Tag color={enable ? "green" : "red"}>
          {enable ? "Enabled" : "Disabled"}
        </Tag>
      ),
    },
  ];

  const handleEdit = (record) => {
    setEditingId(record.id);
    form.setFieldsValue(record);
    setIsModalVisible(true);
  };

  const handleApply = (record) => {
    Modal.confirm({
      title: "Apply Configuration",
      content: `Are you sure you want to apply the "${record.name}" profile?`,
      onOk: () => {
        // Here you would typically make an API call to apply the configuration
        message.success(`Profile "${record.name}" applied successfully`);
        setProfiles(
          profiles.map((profile) => ({
            ...profile,
            status: profile.id === record.id,
          }))
        );
      },
    });
  };

  const handleDelete = (id) => {
    Modal.confirm({
      title: "Delete Profile",
      content: "Are you sure you want to delete this profile?",
      onOk: () => {
        setProfiles(profiles.filter((profile) => profile.id !== id));
        message.success("Profile deleted successfully");
      },
    });
  };

  const handleAdd = () => {
    setEditingId(null);
    form.resetFields();
    setIsModalVisible(true);
  };

  const handleModalOk = () => {
    form
      .validateFields()
      .then((values) => {
        if (editingId) {
          setProfiles(
            profiles.map((profile) =>
              profile.id === editingId ? { ...profile, ...values } : profile
            )
          );
          message.success("Profile updated successfully");
        } else {
          const newProfile = {
            ...values,
            id: Math.max(...profiles.map((p) => p.id)) + 1,
          };
          setProfiles([...profiles, newProfile]);
          message.success("Profile added successfully");
        }
        setIsModalVisible(false);
      })
      .catch((info) => {
        console.log("Validate Failed:", info);
      });
  };

  const onExpandedRowsChange = (expandedRows) => {
    setExpandedRowKeys(expandedRows);
  };

  const expandedRowRender = (record) => (
    <div>
      <Tabs defaultActiveKey="ddos">
        <TabPane tab="DDoS Configuration" key="ddos">
          {error && (
            <Alert message="Error" description={error} type="error" showIcon />
          )}
          <Table
            dataSource={interfaces.map((iface) => ({
              ...iface,
              key: iface.InterfaceId,
            }))}
            columns={ddosInterfaceCols}
            loading={isLoading}
            pagination={false}
            rowKey={(record) => record.key}
            size="small"
          />
        </TabPane>
        <TabPane tab="IPSec Configuration" key="ipsec">
          {error && (
            <Alert message="Error" description={error} type="error" showIcon />
          )}
          <div style={{ overflowX: "auto" }}>
            <Table
              dataSource={ipsecProfiles.map((profile) => ({
                ...profile,
                key: profile.IPSecProfileId,
              }))}
              columns={ipsecCols}
              loading={isLoading}
              pagination={false}
              scroll={{ y: true, x: true }}
              rowKey={(record) => record.key}
              size="small"
            />
          </div>
        </TabPane>
      </Tabs>
    </div>
  );

  return (
    <>
      <Space
        style={{
          width: "100%",
          marginBottom: 16,
          justifyContent: "flex-end",
        }}
      >
        <Button type="primary" icon={<PlusOutlined />} onClick={handleAdd}>
          Add Profile
        </Button>
        <Button>Save All</Button>
      </Space>
      <Table
        rowKey="id"
        columns={columns}
        dataSource={profiles}
        expandable={{
          expandedRowRender,
          expandedRowKeys,
          onExpandedRowsChange,
          rowExpandable: (record) => true,
        }}
      />
      <Modal
        title={editingId ? "Save Profile" : "Add Profile"}
        open={isModalVisible}
        onOk={handleModalOk}
        onCancel={() => setIsModalVisible(false)}
      >
        <Form form={form} layout="vertical">
          <Form.Item
            name="name"
            label="Unique Config Name"
            rules={[{ required: true }]}
          >
            <Input />
          </Form.Item>
        </Form>
      </Modal>
    </>
  );
};

export default ConfigSettingProfile;
