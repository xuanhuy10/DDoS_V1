import React, { useEffect, useState } from "react";
import {
  Button,
  Table,
  Alert,
  Modal,
  Badge,
  Space,
  message,
  Tooltip,
  Tag,
} from "antd";
import { FormOutlined, DeleteOutlined } from "@ant-design/icons";
import {
  getIpSecurity,
  deleteIpSecProfile,
} from "@/features/api/IpSecurity.jsx";
import { getAllDeviceInterfaces } from "@/features/api/DeviceInterfaces";
import "@/features/defense/interface/styles/main.css";
import IpSecEditDrawer from "./IpSecEditDrawer";

const IpSecProfile = () => {
  const [interfaceTimestamps, setInterfaceTimestamps] = useState(() => {
    const saved = localStorage.getItem("ipSecInterfaceTimestamps");
    return saved ? JSON.parse(saved) : {};
  });
  const [initLoading, setInitLoading] = useState(true);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [tableData, setData] = useState([]);
  const [editProfile, setEditProfile] = useState(null);
  const [openDrawer, setOpenDrawer] = useState(false);
  const [allInterfaces, setAllInterfaces] = useState([]);

  // Save interface timestamps to localStorage
  useEffect(() => {
    localStorage.setItem(
        "ipSecInterfaceTimestamps",
        JSON.stringify(interfaceTimestamps)
    );
  }, [interfaceTimestamps]);

  useEffect(() => {
    fetchData();
  }, []);

  // Fetch IPSec profiles and interfaces
  const fetchData = async (start = 0, append = false) => {
    try {
      setError(null);
      setInitLoading(true);

      // Fetch IPSec profiles
      const response = await getIpSecurity(start);
      const newFetched = response.data || [];

      // Fetch device interfaces
      const ifaceRes = await getAllDeviceInterfaces();
      const interfaces = ifaceRes.data || [];
      setAllInterfaces(interfaces);

      // Map backend fields to frontend
      const mappedData = newFetched.map((item) => ({
        IpSecProfileId: item.IPSecProfileId,
        IpSecProfileName: item.ProfileName,
        IpSecProfileDescription: item.ProfileDescription || "",
        ConnectionCount: item.ConnectionCount || 1,
        LocalGatewayIpAddress: item.LocalGateway,
        SubnetLocalGatewayIpAddress: item.SubnetLocalGateway,
        RemoteGatewayIpAddress: item.RemoteGateway,
        SubnetRemoteGatewayIpAddress: item.SubnetRemoteGateway,
        RemoteGatewayArray: item.RemoteGatewayArray || [item.RemoteGateway || 'N/A'],
        SubnetRemoteGatewayArray: item.SubnetRemoteGatewayArray || [item.SubnetRemoteGateway || 'N/A'],
        IKEversion: item.IKEVersion,
        IKEMode: item.Mode,
        Protocol: item.ESPAHProtocol,
        IKEReauthTime: item.IKEReauthTime,
        EncryptionAlgorithm: item.EncryptionAlgorithm,
        HashAlgorithm: item.HashAlgorithm,
        RekeyTime: item.ReKeyTime,
        IpSecProfileCreateTime: item.CreateTime,
        Enable: !!item.Enable,
      }));

      // Merge with interfaces (sửa logic find để match với LocalGateway)
      const merged = mappedData.map((profile) => {
        const interfaceData = interfaces.find(iface => iface.IpAddress === profile.LocalGatewayIpAddress);
        return {
          ...profile,
          IpSecInterface: interfaceData ? interfaceData.InterfaceName : "N/A",
        };
      });

      // Update table data
      const updatedData = append
          ? [
            ...tableData,
            ...merged.filter(
                (newItem) =>
                    !tableData.some(
                        (existing) =>
                            existing.IpSecProfileId === newItem.IpSecProfileId
                    )
            ),
          ]
          : merged;

      setData(updatedData);

      // Sync interfaceTimestamps with UsingTime (xóa vì không còn UsingTime)
      const timestamps = {};
      setInterfaceTimestamps(timestamps);
    } catch (error) {
      setError(
          error.response?.data?.message ||
          "Failed to fetch IPSec profiles or interfaces. Please try again."
      );
      console.error("fetchData error:", error);
    } finally {
      setInitLoading(false);
    }
  };

  // Handle delete profile
  const handleDelete = async (record) => {
    try {
      // Check if profile is enabled
      if (record.Enable) {
        Modal.warning({
          title: "Profile Active",
          content: `IPSec profile "${record.IpSecProfileName}" is currently active and cannot be deleted.`,
        });
        return;
      }

      Modal.confirm({
        title: `Delete ${record.IpSecProfileName}`,
        content: `Are you sure you want to delete IPSec profile ${record.IpSecProfileName}?`,
        onOk: async () => {
          try {
            await deleteIpSecProfile(record.IpSecProfileId);
            message.success("IPSec profile deleted successfully!");
            await fetchData();
          } catch (error) {
            console.error("deleteIpSecProfile error:", error);
            Modal.error({
              title: "Error",
              content:
                  error.response?.data?.message ||
                  "Failed to delete IPSec profile. Please try again.",
            });
          }
        },
      });
    } catch (error) {
      console.error("Error checking profile usage:", error);
      Modal.error({
        title: "Error",
        content: "Failed to check profile usage. Please try again.",
      });
    }
  };

  // Table columns
  const columns = [
    {
      title: "IPSec Profile Name",
      dataIndex: "IpSecProfileName",
      key: "name",
      fixed: "left",
      render: (text, record) => (
          <a
              onClick={() => {
                setEditProfile(record);
                setOpenDrawer(true);
              }}
          >
            {text}
          </a>
      ),
      sorter: (a, b) => a.IpSecProfileName.localeCompare(b.IpSecProfileName),
    },
    {
      title: "Description",
      dataIndex: "IpSecProfileDescription",
      key: "description",
    },
    {
      title: "Connection",
      dataIndex: "ConnectionCount",
      key: "connectionCount",
      render: (text) => <div style={{ textAlign: "center" }}>{text}</div>,
    },
    {
      title: "Local Gateway",
      dataIndex: "LocalGatewayIpAddress",
      key: "localGateway",
      render: (text) => <div style={{ textAlign: "center" }}>{text}</div>,
    },
    {
      title: "Subnet Local Gateway",
      dataIndex: "SubnetLocalGatewayIpAddress",
      key: "subnetLocalGateway",
      render: (text) => <div style={{ textAlign: "center" }}>{text}</div>,
    },
    {
      title: "Remote Gateway",
      key: "remoteGateway",
      render: (text, record) => {
        const gateways = record.RemoteGatewayArray || [];
        const count = record.ConnectionCount || 1;
        const displayGateways = gateways.slice(0, count).join(', ');
        return <div style={{ textAlign: "center" }}>{displayGateways || 'N/A'}</div>;
      },
    },
    {
      title: "Subnet Remote Gateway",
      key: "subnetRemoteGateway",
      render: (text, record) => {
        const subnets = record.SubnetRemoteGatewayArray || [];
        const count = record.ConnectionCount || 1;
        const displaySubnets = subnets.slice(0, count).join(', ');
        return <div style={{ textAlign: "center" }}>{displaySubnets || 'N/A'}</div>;
      },
    },
    {
      title: "IKE Version",
      dataIndex: "IKEversion",
      key: "ikeVersion",
      render: (text) => <div style={{ textAlign: "center" }}>{text}</div>,
    },
    {
      title: "IKE Mode",
      dataIndex: "IKEMode",
      key: "ikeMode",
      render: (text) => (
          <div style={{ textAlign: "center" }}>
            {text === "tunnel" ? "Tunnel" : "Transport"}
          </div>
      ),
    },
    {
      title: "Protocol",
      dataIndex: "Protocol",
      key: "protocol",
      render: (text) => (
          <div style={{ textAlign: "center" }}>{text?.toUpperCase()}</div>
      ),
    },
    {
      title: "IKE Reauth Time",
      dataIndex: "IKEReauthTime",
      key: "ikeReauthTime",
      render: (text) => <div style={{ textAlign: "center" }}>{text}</div>,
    },
    {
      title: "Encryption Algorithm",
      dataIndex: "EncryptionAlgorithm",
      key: "encryptionAlgorithm",
      render: (text) => (
          <div style={{ textAlign: "center" }}>{text?.toUpperCase()}</div>
      ),
    },
    {
      title: "Hash Algorithm",
      dataIndex: "HashAlgorithm",
      key: "hashAlgorithm",
      render: (text) => (
          <div style={{ textAlign: "center" }}>{text?.toUpperCase()}</div>
      ),
    },
    {
      title: "Re-key Time",
      dataIndex: "RekeyTime",
      key: "rekeyTime",
      render: (text) => <div style={{ textAlign: "center" }}>{text}</div>,
    },
    {
      title: "Status",
      dataIndex: "Enable",
      key: "usage",
      width: 120,
      render: (enable) => (
          <div style={{ textAlign: "center" }}>
            <Tag color={enable ? "green" : "red"}>
              {enable ? "Enabled" : "Disabled"}
            </Tag>
          </div>
      ),
      fixed: "right",
      filters: [
        { text: "Enabled", value: true },
        { text: "Disabled", value: false },
      ],
      onFilter: (value, record) => record.Enable === value,
    },
    {
      title: "Actions",
      key: "actions",
      render: (_, record) => (
          <Space>
            <Tooltip title="Edit IPSec Profile">
              <FormOutlined
                  style={{ color: "#1890ff", fontSize: 17, cursor: "pointer" }}
                  onClick={() => {
                    setEditProfile(record);
                    setOpenDrawer(true);
                  }}
              />
            </Tooltip>
            <Tooltip title="Delete IPSec Profile">
              <DeleteOutlined
                  onClick={() => handleDelete(record)}
                  style={{ color: "#ff4d4f", fontSize: 17, cursor: "pointer" }}
              />
            </Tooltip>
          </Space>
      ),
      fixed: "right",
    },
  ];

  return (
      <>
        {error && (
            <Alert
                type="error"
                message={error}
                showIcon
                style={{ marginBottom: 16 }}
                action={
                  <Button size="small" type="primary" onClick={() => fetchData()}>
                    Retry
                  </Button>
                }
            />
        )}
        <IpSecEditDrawer
            openDrawer={openDrawer}
            setOpenDrawer={setOpenDrawer}
            editData={editProfile}
            onClose={() => fetchData()}
            onRefreshData={() => fetchData(0, false)}
        />
        <Table
            columns={columns}
            dataSource={tableData}
            loading={initLoading}
            pagination={false}
            rowKey="IpSecProfileId"
            scroll={{ x: "max-content" }}
            onChange={() => fetchData(0, false)}
        />
      </>
  );
};

export default IpSecProfile;