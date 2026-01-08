import React, { useEffect, useState } from "react";
import { Link } from "react-router-dom";
import {
  Alert,
  Button,
  message,
  Modal,
  Select,
  Space,
  Table,
  Tag,
  Tooltip,
} from "antd";
import {
  DownloadOutlined,
  FormOutlined,
  MonitorOutlined,
  SettingOutlined,
} from "@ant-design/icons";
import { GoMirror } from "react-icons/go";
import ethactive from "@/assets/icons/ethactive.svg";
import ethunactive from "@/assets/icons/ethunactive.svg";
import ethattack from "@/assets/icons/ethattack.svg";
import {
  getAllDeviceInterfaces,
  updateDeviceInterface,
} from "@/features/api/DeviceInterfaces";
import { getIpSecurity } from "@/features/api/Ipsecurity";
import { getDefenseProfiles } from "@/features/api/DefenseProfiles";
import EditDrawer from "@/features/defense/interface/components/ProfileEdit";
import { socket } from "@/utils/socket";
// Function to check keys to compare
const shouldIncludeKey = (key) => {
  return ![
    "DefenseProfileId",
    "UserId",
    "DefenseProfileName",
    "DefenseProfileDescription",
    "DefenseProfileCreateTime",
    "DefenseProfileLastModified",
    "DefenseProfileUsingTime",
    "DefenseProfileStatus",
    "DefenseProfileType",
  ].includes(key);
};

// Function to calculate the number of different fields between two profiles
const calculateDifferentFields = (newProfile, currentProfile) => {
  if (newProfile && newProfile.DefenseProfileName === "DEFAULT") {
    return Object.keys(newProfile).filter((key) => shouldIncludeKey(key))
      .length;
  }

  let differentFieldsCount = 0;
  for (const key in newProfile) {
    if (newProfile.hasOwnProperty(key) && shouldIncludeKey(key)) {
      let newValue = newProfile[key];
      let currentValue = currentProfile ? currentProfile[key] : undefined;

      if (newValue === true) newValue = 1;
      if (newValue === false) newValue = 0;
      if (currentValue === true) currentValue = 1;
      if (currentValue === false) currentValue = 0;

      if (newValue !== currentValue) {
        differentFieldsCount++;
      }
    }
  }
  return differentFieldsCount;
};

export default function PortInterface() {
  const [isLoading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [editProfile, setEditProfile] = useState(null);
  const [openDrawer, setOpenDrawer] = useState(false);
  const [interfaces, setInterfaces] = useState([]);
  const [managementProfiles, setManagementProfiles] = useState([]);
  const [formChanges, setFormChanges] = useState({});
  const [originalInterfaces, setOriginalInterfaces] = useState({});
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [modalStep, setModalStep] = useState("confirm");
  const [estimatedTime, setEstimatedTime] = useState(0);
  const [timeLeft, setTimeLeft] = useState(0);
  const [countdownActive, setCountdownActive] = useState(false);
  const [currentInterfaceId, setCurrentInterfaceId] = useState(null);
  const [updateData, setUpdateData] = useState(null);
  const [ipSecProfiles, setIpSecProfiles] = useState([]);
  const [interfaceStatusData, setInterfaceStatusData] = useState(null);
  const [interfaceOneSecData, setInterfaceOneSecData] = useState(null);

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

  // Function getPortStatus (di chuyển vào component để access state)
  const getPortStatus = (port) => {
    const statusInfo = interfaceStatusData?.interface?.[port];
    const trafficInfo = interfaceOneSecData?.interface?.[port];

    if (!statusInfo || !trafficInfo) {
      return { status: "Unactive", icon: ethunactive, isUnderAttack: false };
    }

    let statusText = "Unactive";
    let isUnderAttack = false;

    if (statusInfo.status === 1) {
      // Interface is active
      if (trafficInfo.attack.bits > 0) {
        // There's attack traffic
        isUnderAttack = true;
        if (trafficInfo.bypass.bits > 0) {
          statusText = "Processing"; // Has both attack and bypass traffic
        } else {
          statusText = "Under Attack"; // Only attack traffic, no bypass
        }
      } else {
        // No attack traffic
        if (trafficInfo.bypass.bits > 0) {
          statusText = "Active"; // Only bypass traffic (normal operation)
        } else {
          statusText = "Active"; // Interface is up but no traffic
        }
      }
    }

    // Determine icon based on status
    let icon;
    if (statusInfo.status === 1) {
      icon = isUnderAttack ? ethattack : ethactive;
    } else {
      icon = ethunactive;
    }

    return { status: statusText, icon, isUnderAttack };
  };

  // Fetch interfaces and management profiles
  const fetchInterfaces = async () => {
    try {
      setError(null);
      setLoading(true);

      const interfaceResponse = await getAllDeviceInterfaces();
      const rawInterfaces = interfaceResponse.data;

      setInterfaces(rawInterfaces);
      setOriginalInterfaces(
        rawInterfaces.reduce((acc, iface) => {
          acc[iface.InterfaceId] = {
            InterfaceType: iface.InterfaceType || "inside",
            InterfaceProtectionMode: iface.InterfaceProtectionMode || "Port",
            DefenseProfileId: iface.DefenseProfileId || null,
          };
          return acc;
        }, {})
      );

      const managementProfileResponse = await getDefenseProfiles();
      setManagementProfiles(managementProfileResponse.data);

      const ipSecProfileResponse = await getIpSecurity();
      setIpSecProfiles(ipSecProfileResponse.data);
    } catch (error) {
      console.error(" fetchInterfaces error:", error);
      setError("Unable to fetch data, please try again later.");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchInterfaces();
  }, []);

  useEffect(() => {
    if (countdownActive && timeLeft > 0) {
      const timer = setInterval(() => {
        setTimeLeft((prev) => {
          if (prev <= 1) {
            return 5; // Khi đếm về 0, reset về 5
          }
          return prev - 1;
        });
      }, 1000);
      return () => clearInterval(timer);
    }
  }, [countdownActive, timeLeft]);

  // Handle field changes
  const onFieldChange = (interfaceId, field, value) => {
    const updatedInterfaces = interfaces.map((item) =>
      item.InterfaceId === interfaceId ? { ...item, [field]: value } : item
    );
    setInterfaces(updatedInterfaces);

    const updated = updatedInterfaces.find(
      (i) => i.InterfaceId === interfaceId
    );
    const original = originalInterfaces[interfaceId];

    const hasChanged =
      updated.InterfaceType !== original.InterfaceType ||
      updated.InterfaceProtectionMode !== original.InterfaceProtectionMode ||
      updated.DefenseProfileId !== original.DefenseProfileId;

    setFormChanges((prev) => ({
      ...prev,
      [interfaceId]: hasChanged,
    }));
  };

  // Trigger update flow
  const handleUpdateInterface = (interfaceId, data) => {
    let totalDifferentFields = 0;
    const original = originalInterfaces[interfaceId];

    if (data.InterfaceProtectionMode !== original.InterfaceProtectionMode) {
      totalDifferentFields += 1;
    }

    if (data.InterfaceType !== original.InterfaceType) {
      totalDifferentFields += 1;
    }

    if (data.DefenseProfileId !== original.DefenseProfileId) {
      const newProfile = managementProfiles.find(
        (p) => p.DefenseProfileId === data.DefenseProfileId
      );
      const currentProfile = managementProfiles.find(
        (p) => p.DefenseProfileId === original.DefenseProfileId
      );
      totalDifferentFields += calculateDifferentFields(
        newProfile,
        currentProfile
      );
    }

    const totalProcessingTime = totalDifferentFields * 2;
    setEstimatedTime(totalProcessingTime);
    setTimeLeft(totalProcessingTime);

    setCurrentInterfaceId(interfaceId);
    setUpdateData(data);
    setModalStep("confirm");
    setIsModalOpen(true);
  };

  const resetUpdateButtons = () => {
    setFormChanges({});
    setOriginalInterfaces((prev) => {
      const updated = { ...prev };
      interfaces.forEach((iface) => {
        updated[iface.InterfaceId] = {
          InterfaceType: iface.InterfaceType || "inside",
          InterfaceProtectionMode: iface.InterfaceProtectionMode || "Port",
          DefenseProfileId: iface.DefenseProfileId || null,
        };
      });
      return updated;
    });
  };

  const confirmUpdate = async () => {
    setModalStep("countdown");
    setCountdownActive(true);

    let apiSuccess = true;
    let apiResponse = null;

    try {
      const iface = interfaces.find(
        (i) => i.InterfaceId === currentInterfaceId
      );
      if (!iface || !iface.InterfaceName) {
        throw new Error("Interface does not exist or is missing InterfaceName");
      }

      const payload = {
        ports: [
          {
            InterfaceId: currentInterfaceId,
            InterfaceName: iface.InterfaceName,
            InterfaceType:
              updateData.InterfaceType || iface.InterfaceType || "inside",
            InterfaceProtectionMode:
              updateData.InterfaceProtectionMode ||
              iface.InterfaceProtectionMode ||
              "Port",
            DefenseProfileId: updateData.DefenseProfileId || null,
          },
        ],
      };

      console.log(" Sending payload:", payload);

      try {
        const response = await updateDeviceInterface(
          currentInterfaceId,
          payload
        );
        apiResponse = response;
        if (response.status !== 200) {
          apiSuccess = false;
          throw new Error(
            `Failed to update interface: Status ${response.status}`
          );
        }
      } catch (error) {
        apiSuccess = false;
        apiResponse = error;
        console.error("❌ API error:", error);
      }

      setCountdownActive(false);
      setIsModalOpen(false);
      setModalStep("confirm");

      if (apiSuccess) {
        message.success("Interface updated successfully!");
      } else {
        message.error(apiResponse?.message || "Failed to update interface!");
      }

      setFormChanges((prev) => ({
        ...prev,
        [currentInterfaceId]: false,
      }));
      setOriginalInterfaces((prev) => ({
        ...prev,
        [currentInterfaceId]: { ...updateData },
      }));
    } catch (error) {
      console.error("❌ Error in confirmUpdate:", error);
      setCountdownActive(false);
      setIsModalOpen(false);
      setModalStep("confirm");
      message.error(error.message || "Failed to update interface!");
      fetchInterfaces();
      setFormChanges({});
    }
  };

  const cancelUpdate = () => {
    setIsModalOpen(false);
    setModalStep("confirm");
    setCountdownActive(false);
  };

  const handleUpdateSuccess = (updatedProfile) => {
    setManagementProfiles((prev) =>
      prev.map((profile) =>
        profile.DefenseProfileId === updatedProfile.DefenseProfileId
          ? { ...profile, ...updatedProfile }
          : profile
      )
    );
    setEditProfile((prev) =>
      prev && prev.DefenseProfileId === updatedProfile.DefenseProfileId
        ? { ...prev, ...updatedProfile }
        : prev
    );
  };

  // Table columns
  const interface_cols = [
    {
      title: "Interface",
      dataIndex: "InterfaceName",
      key: "interface",
      render: (text, record) => {
        const { status: statusText, icon, isUnderAttack } = getPortStatus(text);

        let color;
        if (isUnderAttack) {
          color = "#ff4d4f";
        } else if (statusText === "Active" || statusText === "Processing") {
          color = "#52c41a";
        } else {
          color = "#bfbfbf";
        }

        return (
          <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
            <Link
              style={{
                display: "flex",
                alignItems: "center",
                color,
                fontWeight: "bold",
                transition: "color 0.3s ease",
                textDecoration: "none",
              }}
            >
              <img
                src={icon}
                alt="interface state"
                style={{
                  width: 30,
                  marginRight: 8,
                  filter: isUnderAttack
                    ? "brightness(1) saturate(1.2)"
                    : statusText === "Unactive"
                    ? "brightness(0.7)"
                    : "brightness(1)",
                }}
              />
              {text}
            </Link>
          </div>
        );
      },
    },
    {
      title: "Interface Zone",
      key: "interfaceType",
      align: "center",
      render: (text, record) => (
        <Select
          value={record.formValues.InterfaceType}
          onChange={(value) =>
            onFieldChange(record.InterfaceId, "InterfaceType", value)
          }
          style={{
            width: "100%",
            borderRadius: "6px",
            borderColor: "#d9d9d9",
            boxShadow: "0 2px 4px rgba(0,0,0,0.05)",
          }}
          size="middle"
        >
          <Select.Option value="inside">Inside</Select.Option>
          <Select.Option value="outside">Outside</Select.Option>
        </Select>
      ),
    },
    {
      title: "Protection Mode",
      key: "status",
      align: "center",
      render: (text, record) => (
        <Select
          value={record.formValues.InterfaceProtectionMode}
          onChange={(value) =>
            onFieldChange(record.InterfaceId, "InterfaceProtectionMode", value)
          }
          style={{
            width: "100%",
            borderRadius: "6px",
            borderColor: "#d9d9d9",
            boxShadow: "0 2px 4px rgba(0,0,0,0.05)",
          }}
          size="middle"
        >
          <Select.Option value="Port">Port</Select.Option>
          <Select.Option value="IP">IP</Select.Option>
        </Select>
      ),
    },
    {
      title: "Management Profile",
      key: "defenseProfile",
      align: "center",
      render: (text, record) => (
        <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
          <Select
            value={record.formValues.DefenseProfileId}
            onChange={(value) =>
              onFieldChange(record.InterfaceId, "DefenseProfileId", value)
            }
            style={{
              flexGrow: 1,
              borderRadius: "6px",
              borderColor: "#d9d9d9",
              boxShadow: "0 2px 4px rgba(0,0,0,0.05)",
            }}
            size="middle"
          >
            {managementProfiles.map((profile) => (
              <Select.Option
                key={profile.DefenseProfileId}
                value={profile.DefenseProfileId}
              >
                {profile.DefenseProfileName}
              </Select.Option>
            ))}
          </Select>
          <Tooltip title="Edit Profile">
            <FormOutlined
              style={{
                color: "#1890ff",
                fontSize: 17,
                cursor: "pointer",
                transition: "color 0.3s ease",
                padding: "4px",
                borderRadius: "4px",
              }}
              onMouseEnter={(e) => {
                e.target.style.color = "#40a9ff";
              }}
              onMouseLeave={(e) => {
                e.target.style.color = "#1890ff";
              }}
              onClick={() => {
                const profile = managementProfiles.find(
                  (p) =>
                    p.DefenseProfileId === record.formValues.DefenseProfileId
                );
                setEditProfile(profile || record);
                setOpenDrawer(true);
              }}
            />
          </Tooltip>
        </div>
      ),
    },
    {
      title: "Mirror",
      key: "mirror",
      align: "center",
      render: (text, record) => (
        <Space>
          {record.InterfaceIsMirroring === 1 && (
            <Tooltip title="Mirroring">
              <GoMirror
                style={{
                  color: "#108ee9",
                  fontSize: 17,
                  cursor: "default",
                }}
              />
            </Tooltip>
          )}
          {record.InterfaceIsMonitoring === 1 && (
            <Tooltip title="Monitoring">
              <MonitorOutlined
                style={{
                  color: "#87d068",
                  fontSize: 17,
                  cursor: "default",
                }}
              />
            </Tooltip>
          )}
        </Space>
      ),
    },
    {
      title: "Tag",
      dataIndex: "Tag",
      key: "tag",
      align: "center",
      render: (text, record) => (
        <Space>
          {record.InterfaceIsMirroring === 1 && (
            <Tag
              color="#108ee9"
              style={{
                borderRadius: "9px",
                fontSize: "12px",
                padding: "4px 8px",
              }}
            >
              Mirroring
            </Tag>
          )}
          {record.InterfaceIsMonitoring === 1 && (
            <Tag
              color="#87d068"
              style={{
                borderRadius: "9px",
                fontSize: "12px",
                padding: "4px 8px",
              }}
            >
              Monitoring
            </Tag>
          )}
        </Space>
      ),
    },
    {
      title: "Options",
      key: "action",
      align: "center",
      render: (_, record) => (
        <Button
          icon={<DownloadOutlined />}
          type="primary"
          disabled={!formChanges[record.InterfaceId]}
          onClick={() => {
            const updatedData = {
              InterfaceType: record.formValues.InterfaceType,
              InterfaceProtectionMode:
                record.formValues.InterfaceProtectionMode,
              DefenseProfileId: record.formValues.DefenseProfileId,
            };
            handleUpdateInterface(record.InterfaceId, updatedData);
          }}
          style={{
            borderRadius: "9px",
            backgroundColor: formChanges[record.InterfaceId]
              ? "#1890ff"
              : "#f5f5f5",
            borderColor: formChanges[record.InterfaceId]
              ? "#1890ff"
              : "#d9d9d9",
            color: formChanges[record.InterfaceId] ? "#fff" : "#bfbfbf",
            transition: "all 0.3s ease",
            boxShadow: formChanges[record.InterfaceId]
              ? "0 2px 8px rgba(24, 144, 255, 0.2)"
              : "none",
          }}
          size="middle"
        >
          Update
        </Button>
      ),
    },
  ];

  return (
    <>
      <Modal
        open={isModalOpen}
        footer={null}
        closable={false}
        centered
        maskClosable={false}
        width={400}
      >
        {modalStep === "confirm" ? (
          <div style={{ textAlign: "center", padding: 20 }}>
            <p style={{ fontSize: 18, marginBottom: 16 }}>
              Are you sure you want to update this interface?
            </p>
            <Space>
              <Button danger onClick={cancelUpdate}>
                Cancel
              </Button>
              <Button type="primary" onClick={confirmUpdate}>
                Yes, Update
              </Button>
            </Space>
          </div>
        ) : (
          <div style={{ textAlign: "center", padding: 20 }}>
            <SettingOutlined style={{ fontSize: 48, color: "#1890ff" }} spin />
            <p style={{ marginTop: 16, fontSize: 18 }}>
              Updating configuration...
            </p>
            <p style={{ fontSize: 16, color: "#888" }}>
              Estimated time: {timeLeft}s
            </p>
          </div>
        )}
      </Modal>

      {error && (
        <Alert
          type="error"
          message={error}
          showIcon
          style={{ marginBottom: 16 }}
          action={
            <Button size="small" type="primary" onClick={fetchInterfaces}>
              Try Again
            </Button>
          }
        />
      )}
      <EditDrawer
        openDrawer={openDrawer}
        setOpenDrawer={setOpenDrawer}
        editData={editProfile}
        onUpdateSuccess={handleUpdateSuccess}
      />
      <Table
        columns={interface_cols}
        dataSource={interfaces.map((iface) => ({
          ...iface,
          key: iface.InterfaceId,
          formValues: {
            InterfaceType: iface.InterfaceType || "inside",
            InterfaceProtectionMode: iface.InterfaceProtectionMode || "Port",
            DefenseProfileId: iface.DefenseProfileId || null,
          },
        }))}
        loading={isLoading}
        pagination={false}
        rowKey={(record) => record.key}
      />
    </>
  );
}
