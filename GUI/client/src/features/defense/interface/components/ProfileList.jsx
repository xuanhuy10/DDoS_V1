import React, { useEffect, useState } from "react";
import {
  Modal,
  Badge,
  Space,
  Alert,
  Button,
  Table,
  Checkbox,
  Row,
  Col,
  Tooltip,
  Tag,
} from "antd";
import {
  FormOutlined,
  DeleteOutlined,
  RightSquareOutlined,
  SettingOutlined,
} from "@ant-design/icons";
import {
  getDefenseProfiles,
  applyDefenseProfileToInterfaces,
  deleteDefenseProfile,
} from "@/features/api/DefenseProfiles";
import { getAllDeviceInterfaces } from "@/features/api/DeviceInterfaces";
import EditDrawer from "@/features/defense/interface/components/ProfileEdit";
import "@/features/defense/interface/styles/main.css";

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

const calculateDifferentFields = (newProfile, currentProfile) => {
  // Nếu newProfile có DefenseProfileName là 'DEFAULT', trả về số lượng tất cả các key cần so sánh
  if (newProfile.DefenseProfileName === "DEFAULT") {
    return Object.keys(newProfile).filter((key) => shouldIncludeKey(key))
      .length;
  }

  let differentFieldsCount = 0;
  for (const key in newProfile) {
    if (newProfile.hasOwnProperty(key) && shouldIncludeKey(key)) {
      let newValue = newProfile[key];
      let currentValue = currentProfile ? currentProfile[key] : undefined;
      // Chuẩn hóa giá trị boolean
      if (newValue === true) newValue = 1;
      if (newValue === false) newValue = 0;
      if (currentValue === true) currentValue = 1;
      if (currentValue === false) currentValue = 0;
      // So sánh giá trị
      if (newValue !== currentValue) {
        differentFieldsCount++;
      }
    }
  }
  return differentFieldsCount;
};

const ProfileList = () => {
  const [initLoading, setInitLoading] = useState(true);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [offset, setOffset] = useState(0);
  const [hasMore, setHasMore] = useState(true);
  const [tableData, setData] = useState([]);
  const [editProfile, setEditProfile] = useState(null);
  const [openDrawer, setOpenDrawer] = useState(false);
  const [isPortModalOpen, setIsPortModalOpen] = useState(false);
  const [selectedProfile, setSelectedProfile] = useState(null);
  const [selectedPorts, setSelectedPorts] = useState([]);
  const [originalPorts, setOriginalPorts] = useState([]);
  const [interfaceList, setInterfaceList] = useState([]);
  const [allInterfaces, setAllInterfaces] = useState([]);
  const [estimatedTime, setEstimatedTime] = useState(0);
  const [countdownActive, setCountdownActive] = useState(false);

  const fetchData = async (start = 0, append = false) => {
    try {
      setError(null);
      const response = await getDefenseProfiles(start);
      const newFetched = response.data;
      const ifaceRes = await getAllDeviceInterfaces();
      setAllInterfaces(ifaceRes.data || []);

      const merged = append
        ? [
            ...tableData,
            ...newFetched.filter(
              (newItem) =>
                !tableData.some(
                  (existing) =>
                    existing.DefenseProfileId === newItem.DefenseProfileId
                )
            ),
          ]
        : newFetched;

      setData(merged);
      setInitLoading(false);
      setHasMore(newFetched.length > 0);
      setOffset(append ? start + newFetched.length : newFetched.length);
    } catch (error) {
      setError("Unable to fetch profiles : " + error);
      setInitLoading(false);
    }
  };

  const handleDrawerClose = () => {
    setOpenDrawer(false);
    fetchData();
  };

  const handleApply = async (record) => {
    try {
      setSelectedProfile(record);
      const res = await getAllDeviceInterfaces();
      const interfaces = res.data || [];
      setInterfaceList(interfaces);

      const currentlyAppliedInterfaces = interfaces
        .filter((i) => i.DefenseProfileId === record.DefenseProfileId)
        .map((i) => i.InterfaceId);

      const validCurrentInterfaces = currentlyAppliedInterfaces.filter(
        (interfaceId) =>
          interfaces.some((iface) => iface.InterfaceId === interfaceId)
      );

      setOriginalPorts(validCurrentInterfaces);
      setSelectedPorts(validCurrentInterfaces);
      setIsPortModalOpen(true);
    } catch (error) {
      Modal.error({
        title: "Error",
        content: "Unable to apply profile",
      });
    }
  };

  const handlePortModalOk = async () => {
    let apiSuccess = true;
    let countdown = null;
    try {
      setLoading(true);
      setCountdownActive(true);

      // Validate selectedPorts
      const validSelectedPorts = selectedPorts.filter((portId) => {
        const iface = interfaceList.find((i) => i.InterfaceId === portId);
        return !!iface;
      });

      // Calculate total different fields
      let totalDifferentFields = 0;
      for (const portId of validSelectedPorts) {
        const iface = interfaceList.find((i) => i.InterfaceId === portId);
        if (
          iface &&
          iface.DefenseProfileId &&
          iface.DefenseProfileId !== selectedProfile.DefenseProfileId
        ) {
          const currentProfile = tableData.find(
            (p) => p.DefenseProfileId === iface.DefenseProfileId
          );
          if (currentProfile) {
            totalDifferentFields += calculateDifferentFields(
              selectedProfile,
              currentProfile
            );
          } else {
            totalDifferentFields += Object.keys(selectedProfile).filter((key) =>
              shouldIncludeKey(key)
            ).length;
          }
        } else {
          if (
            !iface.DefenseProfileId ||
            iface.DefenseProfileId !== selectedProfile.DefenseProfileId
          ) {
            totalDifferentFields += Object.keys(selectedProfile).filter((key) =>
              shouldIncludeKey(key)
            ).length;
          }
        }
      }

      // Calculate total processing time
      const totalProcessingTime = totalDifferentFields * 2;
      setEstimatedTime(totalProcessingTime);

      let timeLeft = totalProcessingTime;
      countdown = setInterval(() => {
        timeLeft -= 1;
        setEstimatedTime(timeLeft);
        if (timeLeft <= 0) {
          // clearInterval(countdown);
          // setCountdownActive(false);
          // setIsPortModalOpen(false);
          // fetchData();
          // if (apiSuccess) {
          //   Modal.success({
          //     title: "Success",
          //     content: "Defense profile updated successfully!",
          //   });
          // } else {
          //   Modal.error({
          //     title: "Error",
          //     content: "Failed to update defense profile.",
          //   });
          // }
          timeLeft = 6;
        }
      }, 1000);

      // Check for invalid ports
      if (validSelectedPorts.length !== selectedPorts.length) {
        const invalidCount = selectedPorts.length - validSelectedPorts.length;
        Modal.warning({
          title: "Warning",
          content: `${invalidCount} interfaces are no longer available and will be skipped`,
        });
      }

      const toAdd = validSelectedPorts.filter(
        (p) => !originalPorts.includes(p)
      );
      const toRemove = originalPorts.filter(
        (p) => !validSelectedPorts.includes(p)
      );

      const currentDate = new Date();
      const formattedDate = `${currentDate.getFullYear()}/${(
        currentDate.getMonth() + 1
      )
        .toString()
        .padStart(2, "0")}/${currentDate
        .getDate()
        .toString()
        .padStart(2, "0")} ${currentDate
        .getHours()
        .toString()
        .padStart(2, "0")}:${currentDate
        .getMinutes()
        .toString()
        .padStart(2, "0")}:${currentDate
        .getSeconds()
        .toString()
        .padStart(2, "0")}`;

      // Send applyDefenseProfileToInterfaces request
      let applyResponse = null;
      if (toAdd.length > 0) {
        applyResponse = await applyDefenseProfileToInterfaces(
          selectedProfile.DefenseProfileId,
          toAdd.map((portId) => {
            const iface = interfaceList.find((i) => i.InterfaceId === portId);
            if (!iface || !iface.InterfaceId) {
              throw new Error(`InterfaceId not found for interface ${portId}`);
            }
            return {
              name: iface.InterfaceName,
              date: formattedDate,
            };
          })
        );
        if (applyResponse.status !== 200) {
          throw new Error(
            `Failed to apply defense profile: Status ${applyResponse.status}`
          );
        } else {
          clearInterval(countdown);
          setCountdownActive(false);
          setEstimatedTime(0);
          setIsPortModalOpen(false);
          await fetchData();
          Modal.success({
            title: "Success",
            content: "Defense profile updated successfully!",
          });
          return;
        }
      }

      if (toRemove.length > 0) {
        applyResponse = await applyDefenseProfileToInterfaces(
          null,
          toRemove.map((portId) => {
            const iface = interfaceList.find((i) => i.InterfaceId === portId);
            if (!iface || !iface.InterfaceId) {
              throw new Error(
                `InterfaceId not found for interface to remove ${portId}`
              );
            }
            return {
              name: iface.InterfaceName,
              date: formattedDate,
            };
          })
        );
        if (applyResponse.status !== 200) {
          throw new Error(
            `Failed to remove defense profile: Status ${applyResponse.status}`
          );
        } else {
          // ✅ Dừng countdown ngay khi có response thành công
          clearInterval(countdown);
          setCountdownActive(false);
          setEstimatedTime(0);
          setIsPortModalOpen(false);
          await fetchData();
          Modal.success({
            title: "Success",
            content: "Defense profile removed successfully!",
          });
          return; // Thoát luôn
        }
      }

      setSelectedPorts([]);
      setOriginalPorts([]);
      setSelectedProfile(null);
    } catch (error) {
      apiSuccess = false;
      clearInterval(countdown);
      setCountdownActive(false);
      setEstimatedTime(0);
      setIsPortModalOpen(false);
      setSelectedPorts([]);
      setOriginalPorts([]);
      setSelectedProfile(null);
      Modal.error({
        title: "Error",
        content: error.message || "Failed to update defense profile",
      });
    } finally {
      setLoading(false);
    }
  };

  const arraysEqual = (arr1 = [], arr2 = []) => {
    if (arr1.length !== arr2.length) return false;
    const sorted1 = [...arr1].sort();
    const sorted2 = [...arr2].sort();
    return sorted1.every((val, index) => val === sorted2[index]);
  };

  const handleDelete = async (record) => {
    try {
      const freshInterfaceRes = await getAllDeviceInterfaces();
      const freshInterfaces = freshInterfaceRes.data || [];

      const isInUse = freshInterfaces.some(
        (iface) =>
          String(iface.DefenseProfileId) === String(record.DefenseProfileId)
      );

      if (isInUse) {
        const assignedInterfaces = freshInterfaces
          .filter(
            (iface) =>
              String(iface.DefenseProfileId) === String(record.DefenseProfileId)
          )
          .map((iface) => iface.InterfaceName);

        Modal.warning({
          title: "Profile in use",
          content: `Profile "${
            record.DefenseProfileName
          }" is currently assigned to the following interfaces and cannot be deleted: ${assignedInterfaces.join(
            ", "
          )}`,
        });
        return;
      }

      Modal.confirm({
        title: `Delete ${record.DefenseProfileName}`,
        content: `Are you sure you want to delete profile ${record.DefenseProfileName}?`,
        onOk: async () => {
          try {
            await deleteDefenseProfile(record.DefenseProfileId);
            Modal.success({
              title: "Success",
              content: "Defense profile deleted successfully!",
            });
            await fetchData();
          } catch (error) {
            Modal.error({
              title: "Error",
              content:
                error.response?.data?.message ||
                "Cannot delete defense profile. Please try again.",
            });
          }
        },
      });
    } catch (error) {
      Modal.error({
        title: "Error",
        content: "Unable to check profile usage status. Please try again.",
      });
    }
  };

  useEffect(() => {
    fetchData();
  }, []);

  const columns = [
    {
      title: "Profile Name",
      dataIndex: "DefenseProfileName",
      key: "name",
      fixed: "left",
      render: (text, record) => (
        <a
          onClick={() => {
            setEditProfile(record);
            setOpenDrawer(true);
          }}
          style={{
            textAlign: "center",
            color: "#1890ff",
            fontWeight: "bold",
            textDecoration: "none",
            transition: "color 0.3s ease",
            ":hover": {
              backgroundColor: "#e6f7ff",
            },
          }}
        >
          {text}
        </a>
      ),
      sorter: (a, b) =>
        a.DefenseProfileName.localeCompare(b.DefenseProfileName),
      sortDirections: ["descend", "ascend"],
    },
    {
      title: "Description",
      dataIndex: "DefenseProfileDescription",
      key: "description",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#666",
            fontSize: "14px",
            padding: "8px",
          }}
        >
          {text || "No description"}
        </div>
      ),
    },
    {
      title: "Detection Time (s)",
      dataIndex: "DetectionTime",
      key: "detectionTime",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            fontWeight: "bold",
            color: "#1890ff",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "Mode",
      dataIndex: "DefenseMode",
      key: "mode",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Tag
            color={text === "aggregate" ? "blue" : "purple"}
            style={{
              borderRadius: "12px",
              fontWeight: "bold",
              padding: "4px 8px",
            }}
          >
            {text === "aggregate" ? "Aggregate" : "Classified"}
          </Tag>
        </div>
      ),
    },
    {
      title: "ICMP Flood",
      dataIndex: "ICMPFloodEnable",
      key: "icmpFlood",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "ICMP Flood Threshold (pps)",
      dataIndex: "ICMPFloodThreshold",
      key: "icmpFloodThr",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#ff6b35",
            fontWeight: "bold",
            backgroundColor: "#fff2e8",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "ICMP Flood Rate (pps)",
      dataIndex: "ICMPFloodRate",
      key: "icmpFloodRate",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#ff6b35",
            fontWeight: "bold",
            backgroundColor: "#fff2e8",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "SYN Flood",
      dataIndex: "SYNFloodEnable",
      key: "synFlood",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "SYN Threshold (pps)",
      dataIndex: "SYNFloodSYNThreshold",
      key: "synFloodSynThr",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#722ed1",
            fontWeight: "bold",
            backgroundColor: "#f3e8ff",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "ACK Threshold (pps)",
      dataIndex: "SYNFloodACKThreshold",
      key: "synFloodAckThr",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#722ed1",
            fontWeight: "bold",
            backgroundColor: "#f3e8ff",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "Whitelist Timeout (s)",
      dataIndex: "SYNFloodWhiteListTimeOut",
      key: "synFloodWhitelistTimeout",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#722ed1",
            fontWeight: "bold",
            backgroundColor: "#f3e8ff",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "UDP Flood",
      dataIndex: "UDPFloodEnable",
      key: "udpFlood",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "UDP Flood Threshold (pps)",
      dataIndex: "UDPFloodThreshold",
      key: "udpFloodThr",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#fa8c16",
            fontWeight: "bold",
            backgroundColor: "#fff7e6",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "UDP Flood Rate (pps)",
      dataIndex: "UDPFloodRate",
      key: "udpFloodRate",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#fa8c16",
            fontWeight: "bold",
            backgroundColor: "#fff7e6",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "DNS Flood",
      dataIndex: "DNSFloodEnable",
      key: "dnsFlood",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "DNS Flood Threshold (pps)",
      dataIndex: "DNSFloodThreshold",
      key: "dnsFloodThr",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#fa541c",
            fontWeight: "bold",
            backgroundColor: "#fff2f0",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "Land Attack",
      dataIndex: "LandAttackEnable",
      key: "landAttack",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "IPSec IKE",
      dataIndex: "IPSecIKEEnable",
      key: "ipsecIke",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "IPSec IKE Threshold (pps)",
      dataIndex: "IPSecIKEThreshold",
      key: "ipsecIkeThr",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#eb2f96",
            fontWeight: "bold",
            backgroundColor: "#fff0f6",
            borderRadius: "6px",
            padding: "4px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "TCP Fragment Attack",
      dataIndex: "TCPFragmentEnable",
      key: "tcpFragment",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "UDP Fragment Attack",
      dataIndex: "UDPFragmentEnable",
      key: "udpFragment",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "HTTP Flood",
      dataIndex: "HTTPFloodEnable",
      key: "httpFlood",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "HTTPS Flood",
      dataIndex: "HTTPSFloodEnable",
      key: "httpsFlood",
      render: (text) => (
        <div style={{ textAlign: "center" }}>
          <Badge
            status={text ? "success" : "default"}
            color={text ? "#52c41a" : "#d9d9d9"}
            text={text ? "Enabled" : "Disabled"}
            style={{
              fontSize: "12px",
              borderRadius: "10px",
              padding: "2px 6px",
            }}
          />
        </div>
      ),
    },
    {
      title: "Created Date",
      dataIndex: "DefenseProfileCreateTime",
      key: "createTime",
      sorter: (a, b) =>
        a.DefenseProfileCreateTime.localeCompare(b.DefenseProfileCreateTime),
      sortDirections: ["descend", "ascend"],
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#595959",
            fontSize: "12px",
            backgroundColor: "#f5f5f5",
            borderRadius: "4px",
            padding: "4px 8px",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "Usage (YYYY/MM/DD HH:MM:SS)",
      dataIndex: "DefenseProfileUsingTime",
      key: "usingTime",
      width: 200,
      render: (text, record) => {
        // Lọc các interface đang sử dụng profile này
        const activeInterfaces = allInterfaces.filter(
          (iface) =>
            String(iface.DefenseProfileId) === String(record.DefenseProfileId)
        );

        if (activeInterfaces.length > 0) {
          // Lấy danh sách tên các cổng
          const formatInterfaceWithTimestamp = (interfaces) => {
            let usageData = [];
            try {
              usageData = JSON.parse(record.DefenseProfileUsingTime || "[]");
            } catch (error) {
              usageData = [];
            }

            const timestampedInterfaces = interfaces.map((iface) => {
              const usageItem = usageData.find(
                (item) => item.name === iface.InterfaceName
              );
              return {
                timestamp: usageItem?.date || "Unknown time",
                interfaceName: iface.InterfaceName,
              };
            });

            const groupedByTimestamp = timestampedInterfaces.reduce(
              (acc, { timestamp, interfaceName }) => {
                if (!acc[timestamp]) {
                  acc[timestamp] = [];
                }
                acc[timestamp].push(interfaceName);
                return acc;
              },
              {}
            );

            return Object.keys(groupedByTimestamp)
              .sort((a, b) => {
                if (a === "Unknown time") return 1;
                if (b === "Unknown time") return -1;
                const timestampRegex =
                  /^\d{4}\/\d{2}\/\d{2} \d{2}:\d{2}:\d{2}$/;
                if (!timestampRegex.test(a) || !timestampRegex.test(b)) {
                  return a.localeCompare(b);
                }
                const dateA = new Date(
                  a.replace(
                    /(\d{4})\/(\d{2})\/(\d{2}) (\d{2}):(\d{2}):(\d{2})/,
                    "$1-$2-$3T$4:$5:$6"
                  )
                );
                const dateB = new Date(
                  b.replace(
                    /(\d{4})\/(\d{2})\/(\d{2}) (\d{2}):(\d{2}):(\d{2})/,
                    "$1-$2-$3T$4:$5:$6"
                  )
                );
                return dateB - dateA;
              })
              .map(
                (timestamp) =>
                  `(${timestamp} -> ${groupedByTimestamp[timestamp].join(
                    ", "
                  )})`
              )
              .join(", ");
          };

          const formattedDisplay =
            formatInterfaceWithTimestamp(activeInterfaces);

          return (
            <div
              className="usage-scroll-container"
              style={{
                overflowY: "auto",
                backgroundColor: "#f6ffed",
                borderRadius: "6px",
                padding: "8px",
              }}
            >
              <div className="usage-scroll-inner">
                <Badge status="success" />
                <span
                  style={{
                    marginLeft: 8,
                    color: "#52c41a",
                    fontWeight: "bold",
                  }}
                >
                  {activeInterfaces.length} active ports: {formattedDisplay}
                </span>
              </div>
            </div>
          );
        }

        return (
          <div
            style={{
              textAlign: "center",
              backgroundColor: "#f5f5f5",
              borderRadius: "6px",
              padding: "8px",
              color: "#bfbfbf",
            }}
          >
            <Badge status="default" />
            <span style={{ marginLeft: 8 }}>Not in use</span>
          </div>
        );
      },
      fixed: "right",
      filters: [
        { text: "In use", value: "Active" },
        { text: "Not in use", value: "Inactive" },
      ],
      onFilter: (value, record) => {
        const used = allInterfaces.some(
          (iface) =>
            String(iface.DefenseProfileId) === String(record.DefenseProfileId)
        );
        return value === "Active" ? used : !used;
      },
    },
    {
      title: "Options",
      key: "actions",
      render: (text, record) => (
        <Space style={{ justifyContent: "center" }}>
          {record.DefenseProfileName !== "DEFAULT" && (
            <>
              <Tooltip title="Edit Profile">
                <FormOutlined
                  style={{
                    color: "#1890ff",
                    fontSize: 17,
                    cursor: "pointer",
                    padding: "4px",
                    borderRadius: "4px",
                    transition: "all 0.3s ease",
                    ":hover": {
                      backgroundColor: "#e6f7ff",
                      transform: "scale(1.1)",
                    },
                  }}
                  onClick={() => {
                    setEditProfile(record);
                    setOpenDrawer(true);
                  }}
                />
              </Tooltip>
              <Tooltip title="Delete Profile">
                <DeleteOutlined
                  onClick={() => handleDelete(record)}
                  style={{
                    color: "#ff4d4f",
                    fontSize: 17,
                    cursor: "pointer",
                    padding: "4px",
                    borderRadius: "4px",
                    transition: "all 0.3s ease",
                    ":hover": {
                      backgroundColor: "#fff2f0",
                      transform: "scale(1.1)",
                    },
                  }}
                />
              </Tooltip>
            </>
          )}
          <Tooltip title="Apply to Interfaces">
            <RightSquareOutlined
              onClick={() => handleApply(record)}
              style={{
                color: "#52c41a",
                fontSize: 17,
                cursor: "pointer",
                padding: "4px",
                borderRadius: "4px",
                transition: "all 0.3s ease",
                ":hover": {
                  backgroundColor: "#f6ffed",
                  transform: "scale(1.1)",
                },
              }}
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
              Thử lại
            </Button>
          }
        />
      )}
      <EditDrawer
        openDrawer={openDrawer}
        setOpenDrawer={handleDrawerClose}
        editData={editProfile}
      />
      <Table
        columns={columns}
        dataSource={tableData}
        loading={initLoading}
        pagination={false}
        rowKey={(record) => record.DefenseProfileId}
        scroll={{ x: "max-content" }}
        bordered
        size="middle"
        rowClassName={(record, index) =>
          index % 2 === 0 ? "table-row-even" : "table-row-odd"
        }
        tableLayout="fixed"
        components={{
          header: {
            cell: (props) => (
              <th
                {...props}
                style={{
                  ...props.style,
                  backgroundColor: "#f0f2f5",
                  color: "#262626",
                  fontWeight: "bold",
                  borderBottom: "2px solid #d9d9d9",
                  padding: "16px 12px",
                  textAlign: "center",
                  borderRight: "1px solid #e8e8e8",
                  borderTop: "1px solid #e8e8e8",
                }}
              />
            ),
          },
          body: {
            cell: (props) => (
              <td
                {...props}
                style={{
                  ...props.style,
                  padding: "12px",
                  textAlign: "center",
                  borderRight: "1px solid #f0f0f0",
                  borderBottom: "1px solid #f0f0f0",
                  backgroundColor:
                    props.index % 2 === 0 ? "#fafafa" : "#ffffff",
                }}
              />
            ),
          },
        }}
      />
      <Modal
        title={`Select interfaces for ${
          selectedProfile?.DefenseProfileName || "Profile"
        }`}
        open={isPortModalOpen}
        onOk={handlePortModalOk}
        onCancel={() => {
          setIsPortModalOpen(false);
          setSelectedPorts([]);
          setOriginalPorts([]);
          setSelectedProfile(null);
        }}
        okText="Apply"
        okButtonProps={{
          disabled:
            arraysEqual(originalPorts, selectedPorts) || countdownActive,
          loading: loading,
        }}
        cancelButtonProps={{ disabled: loading }}
      >
        <div>
          <b>Select interfaces to apply:</b>
          {interfaceList.length === 0 && (
            <Alert
              message="No interfaces available"
              description="There are no network interfaces to configure."
              type="warning"
              style={{ marginTop: 12 }}
            />
          )}
          {interfaceList.length > 0 && (
            <>
              <div
                style={{
                  marginTop: 8,
                  marginBottom: 12,
                  fontSize: "12px",
                  color: "#666",
                }}
              >
                Selected: {selectedPorts.length} out of {interfaceList.length}{" "}
                available interfaces
                {originalPorts.length !== selectedPorts.length && (
                  <span style={{ color: "#1890ff", marginLeft: 8 }}>
                    (Pending changes)
                  </span>
                )}
              </div>
              <Checkbox.Group
                style={{
                  display: "flex",
                  flexDirection: "column",
                  marginTop: 12,
                }}
                value={selectedPorts}
                onChange={(val) => setSelectedPorts(val)}
                disabled={countdownActive}
              >
                <Row gutter={[16, 16]}>
                  {interfaceList.map((iface) => {
                    const isOriginallyAssigned = originalPorts.includes(
                      iface.InterfaceId
                    );
                    return (
                      <Col
                        span={6}
                        align="middle"
                        key={iface.InterfaceId}
                        style={{ marginBottom: 8 }}
                      >
                        <Checkbox
                          value={iface.InterfaceId}
                          disabled={isOriginallyAssigned}
                          style={{
                            opacity: isOriginallyAssigned ? 0.7 : 1,
                            cursor: isOriginallyAssigned
                              ? "not-allowed"
                              : "pointer",
                          }}
                        >
                          <Tooltip
                            title={
                              isOriginallyAssigned
                                ? `Assigned to this profile - Status: ${
                                    iface.InterfaceStatus || "Unknown"
                                  }`
                                : `Status: ${
                                    iface.InterfaceStatus || "Unknown"
                                  }`
                            }
                          >
                            {iface.InterfaceName}
                            {isOriginallyAssigned && (
                              <Badge
                                status="processing"
                                style={{ marginLeft: 4 }}
                              />
                            )}
                            {!isOriginallyAssigned && iface.InterfaceStatus && (
                              <Badge
                                status={
                                  iface.InterfaceStatus === "up"
                                    ? "success"
                                    : "default"
                                }
                                style={{ marginLeft: 4 }}
                              />
                            )}
                          </Tooltip>
                        </Checkbox>
                      </Col>
                    );
                  })}
                </Row>
              </Checkbox.Group>
            </>
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
                {" "}
                Estimated Time: {estimatedTime}s
              </p>
            </div>
          )}
        </div>
      </Modal>
    </>
  );
};

export default ProfileList;
