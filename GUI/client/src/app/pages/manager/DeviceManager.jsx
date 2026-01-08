import React, { useEffect, useState } from "react";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
  Tabs,
  Spin,
  Space,
  Flex,
  Typography,
  Badge,
  Card,
  Button,
  Modal,
  message,
} from "antd";
const { Text } = Typography;

import {
  faHardDrive,
  faMemory,
  faMicrochip,
  faThermometerHalf,
} from "@fortawesome/free-solid-svg-icons";

import PageTitle from "@/components/common/PageTitle";
import DiskChart from "@/features/manager/devices/components/DiskChart";
import CpuBarChart from "@/features/manager/devices/components/CpuBarChart";
import MemoryChart from "@/features/manager/devices/components/MemoryChart";
import SmallAreaChart from "@/features/manager/devices/components/SmallAreaChart";
import SettingFeature from "@/features/manager/devices/components/SettingFeature";
import TemperatureChart from "@/features/manager/devices/components/TemperatureChart";

import {
  getDeviceResourceUsage,
  resetSystem,
} from "@/features/api/DeviceSettings";

// Helper function to format time difference into "days : hours : minutes : seconds"
const formatUptime = (lastReset) => {
  if (!lastReset || lastReset === "Never") return "Never";
  const now = new Date();
  const lastResetDate = new Date(lastReset);
  const diffMs = now - lastResetDate;
  if (diffMs < 0) return "0 seconds";

  const seconds = Math.floor((diffMs / 1000) % 60);
  const minutes = Math.floor((diffMs / (1000 * 60)) % 60);
  const hours = Math.floor((diffMs / (1000 * 60 * 60)) % 24);
  const days = Math.floor(diffMs / (1000 * 60 * 60 * 24));

  let uptimeParts = [];
  if (days > 0) uptimeParts.push(`${days} day${days !== 1 ? "s" : ""}`);
  if (hours > 0) uptimeParts.push(`${hours} hour${hours !== 1 ? "s" : ""}`);
  if (minutes > 0)
    uptimeParts.push(`${minutes} minute${minutes !== 1 ? "s" : ""}`);
  if (seconds > 0 || uptimeParts.length === 0) {
    uptimeParts.push(`${seconds} second${seconds !== 1 ? "s" : ""}`);
  }

  return uptimeParts.join(" : ");
};

export default function DeviceManager() {
  const [systemData, setSystemData] = useState(null);
  const [loading, setLoading] = useState(false);
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [lastReset, setLastReset] = useState(() => {
    return localStorage.getItem("lastReset") || "Never";
  });
  const [uptime, setUptime] = useState(() => formatUptime(lastReset));

  const fetchData = async () => {
    try {
      setLoading(true);
      const response = await getDeviceResourceUsage();
      const data = response.data;
      if (!data?.performanceData) {
        throw new Error("Invalid API response structure");
      }
      setSystemData({
        totalMemory: data.performanceData.memory?.totalMemory
          ? parseFloat(data.performanceData.memory.totalMemory)
          : 0,
        heapDisplay: data.performanceData.memory?.heapUsed
          ? parseFloat(data.performanceData.memory.heapUsed)
          : 0,
        diskPercentageDisplay: data.performanceData.storageInfo?.usedPercentage
          ? parseFloat(data.performanceData.storageInfo.usedPercentage)
          : 0,
        cpuPercentShow: data.performanceData.cpu?.overallUsage
          ? parseFloat(data.performanceData.cpu.overallUsage)
          : 0,
        cpuUsage: data.performanceData.cpu?.utilization || [],
        diskStorageInfo: data.performanceData.storageInfo || {},
        nCPUs: {
          model: data.performanceData.cpu?.model || "Unknown",
          cores: data.performanceData.cpu?.cores
            ? `${data.performanceData.cpu.cores} cores`
            : "Unknown",
          speed: data.performanceData.cpu?.speed || "Unknown",
        },
        memoryUsage: {
          freeMemory: data.performanceData.memory?.freeMemory || "0",
          heapUsed: data.performanceData.memory?.heapUsed || "0",
          totalMemory: data.performanceData.memory?.totalMemory || "0",
          committedMemory: data.performanceData.memory?.committed || "0",
          cachedMemory: data.performanceData.memory?.cached || "0",
        },
        diskUsage: {
          freeDisk: data.performanceData.storageInfo?.free || "0",
          usedDisk: data.performanceData.storageInfo?.used || "0",
          totalDisk: data.performanceData.storageInfo?.total || "0",
          diskType: data.performanceData.storageInfo?.type || "Unknown",
        },
        temperature: {
          value: data.performanceData.temperature?.value || 0,
          unit: data.performanceData.temperature?.unit || "°C",
        },
      });
    } catch (error) {
      console.error("Failed to fetch system data:", error);
      message.error("Failed to fetch system data. Please try again.");
    } finally {
      setLoading(false);
    }
  };

  const heapUsed = systemData?.memoryUsage?.heapUsed || "0";
  const totalMemory = systemData?.memoryUsage?.totalMemory || "0";
  const freeMemory = systemData?.memoryUsage?.freeMemory || "0";
  const diskUsed = systemData?.diskUsage?.usedDisk || "0";
  const diskTotal = systemData?.diskUsage?.totalDisk || "0";
  const diskFree = systemData?.diskUsage?.freeDisk || "0";
  const diskUsedPercentage = Number(
    systemData?.diskStorageInfo?.usedPercentage || 0
  );
  const memoryUsedPercentage =
    systemData?.heapDisplay && systemData?.totalMemory
      ? Number(
          ((systemData.heapDisplay / systemData.totalMemory) * 100).toFixed(2)
        )
      : 0;
  const temperatureValue = systemData?.temperature?.value || 0;
  const temperatureUnit = systemData?.temperature?.unit || "°C";

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 5000);
    // Update uptime every second
    const uptimeInterval = setInterval(() => {
      setUptime(formatUptime(lastReset));
    }, 1000);
    return () => {
      clearInterval(interval);
      clearInterval(uptimeInterval);
    };
  }, [lastReset]);

  const statusColor = (value) =>
    value > 80 ? "red" : value > 50 ? "orange" : "green";

  const items = [
    {
      key: "CPU",
      label: (
        <Flex align="center" gap={10}>
          {typeof systemData?.cpuPercentShow === "number" ? (
            <SmallAreaChart data={systemData?.cpuPercentShow} />
          ) : (
            <Spin size="small" />
          )}
          <Space direction="vertical">
            <Flex align="center" gap={5}>
              <FontAwesomeIcon icon={faMicrochip} aria-label="CPU icon" />
              <Text strong>CPU</Text>
              <Badge color={statusColor(systemData?.cpuPercentShow)} />
            </Flex>
            <Flex align="center" gap={5}>
              <Text>
                {systemData?.cpuPercentShow ? (
                  `${systemData.cpuPercentShow.toFixed(2)}%`
                ) : (
                  <Spin size="small" />
                )}{" "}
                | {systemData?.nCPUs.cores || <Spin size="small" />}
              </Text>
            </Flex>
          </Space>
        </Flex>
      ),
      children: (
        <div style={{ padding: "20px", marginTop: "20px" }}>
          <CpuBarChart data={systemData} />
        </div>
      ),
    },
    {
      key: "MEMORY",
      label: (
        <Flex align="center" gap={10}>
          {typeof memoryUsedPercentage === "number" ? (
            <SmallAreaChart data={memoryUsedPercentage} />
          ) : (
            <Spin size="small" />
          )}
          <Space direction="vertical">
            <Flex align="center" gap={5}>
              <FontAwesomeIcon icon={faMemory} aria-label="Memory icon" />
              <Text strong>Memory</Text>
              <Badge color={statusColor(memoryUsedPercentage)} />
            </Flex>
            <Flex align="center" gap={5}>
              <Text>
                {systemData?.heapDisplay ? (
                  `${(systemData.heapDisplay / 1000).toFixed(2)} GB`
                ) : (
                  <Spin size="small" />
                )}{" "}
                /{" "}
                {systemData?.totalMemory ? (
                  `${(systemData.totalMemory / 1000).toFixed(2)} GB`
                ) : (
                  <Spin size="small" />
                )}
              </Text>
            </Flex>
          </Space>
        </Flex>
      ),
      children: (
        <div style={{ padding: "20px", marginTop: "20px" }}>
          <MemoryChart
            heapUsed={heapUsed}
            totalMemory={totalMemory}
            freeMemory={freeMemory}
          />
        </div>
      ),
    },
    {
      key: "DISK",
      label: (
        <Flex align="center" gap={10}>
          {typeof diskUsedPercentage === "number" ? (
            <SmallAreaChart data={diskUsedPercentage} />
          ) : (
            <Spin size="small" />
          )}
          <Space direction="vertical">
            <Flex align="center" gap={5}>
              <FontAwesomeIcon icon={faHardDrive} aria-label="Disk icon" />
              <Text strong>Disk</Text>
              <Badge color={statusColor(diskUsedPercentage)} />
            </Flex>
            <Flex align="center" gap={5}>
              <Text>
                {diskUsedPercentage ? (
                  `${diskUsedPercentage} %`
                ) : (
                  <Spin size="small" />
                )}
              </Text>
            </Flex>
          </Space>
        </Flex>
      ),
      children: (
        <div style={{ padding: "20px", marginTop: "20px" }}>
          <DiskChart
            diskUsed={diskUsed}
            diskTotal={diskTotal}
            diskFree={diskFree}
            diskUsedPercentage={diskUsedPercentage}
          />
        </div>
      ),
    },
    {
      key: "TEMP",
      label: (
        <Flex align="center" gap={10}>
          {typeof temperatureValue === "number" ? (
            <SmallAreaChart data={temperatureValue} />
          ) : (
            <Spin size="small" />
          )}
          <Space direction="vertical">
            <Flex align="center" gap={5}>
              <FontAwesomeIcon
                icon={faThermometerHalf}
                aria-label="Temperature icon"
              />
              <Text strong>Temperature</Text>
              <Badge color={statusColor(temperatureValue)} />
            </Flex>
            <Flex align="center" gap={5}>
              <Text>
                {temperatureValue ? (
                  `${temperatureValue} ${temperatureUnit}`
                ) : (
                  <Spin size="small" />
                )}
              </Text>
            </Flex>
          </Space>
        </Flex>
      ),
      children: (
        <div style={{ padding: "20px", marginTop: "20px" }}>
          <TemperatureChart value={temperatureValue} unit={temperatureUnit} />
        </div>
      ),
    },
  ];

  const showResetModal = () => setIsModalOpen(true);

  const handleOk = async () => {
    try {
      setIsModalOpen(false);
      setLoading(true);
      const response = await resetSystem();
      // Assuming the API returns a status or success indicator
      if (response.status === 200 || response.data?.success) {
        message.success("The system has been reset successfully.");
        const resetTime = new Date().toISOString(); // Store as ISO string for precision
        setLastReset(resetTime);
        localStorage.setItem("lastReset", resetTime);
        setUptime("0 seconds");
        // Fetch updated data after reset
        await fetchData();
      } else {
        throw new Error("Reset API call did not confirm success");
      }
    } catch (error) {
      console.error("Reset system failed:", error);
      message.error("System reset failed. Please try again.");
    } finally {
      setLoading(false);
    }
  };

  const handleCancel = () => setIsModalOpen(false);

  return (
    <>
      <PageTitle title="Device Manager" description="Monitor your system" />
      <Card style={{ marginBottom: "20px" }}>
        <Flex
          justify="space-between"
          align="center"
          style={{ marginBottom: "0px" }}
        >
          <Text strong style={{ fontSize: "20px" }}>
            Device Overview
          </Text>
          <Space>
            <Text type="main">System Uptime: {uptime || "Loading..."}</Text>
          </Space>
          <Space>
            <Text type="main">
              Last reset:{" "}
              {lastReset === "Never"
                ? "Never"
                : new Date(lastReset).toLocaleString("en-US", {
                    weekday: "short",
                    month: "numeric",
                    day: "numeric",
                    year: "numeric",
                    hour: "numeric",
                    minute: "numeric",
                    second: "numeric",
                    hour12: true,
                  })}
            </Text>
          </Space>
          <Space>
            <Button
              type="primary"
              danger
              style={{ backgroundColor: "#ff4d4f", borderColor: "#ff4d4f" }}
              onClick={showResetModal}
            >
              RESET SYSTEM
            </Button>
          </Space>
        </Flex>
      </Card>
      <Tabs
        tabPosition="left"
        items={items}
        style={{
          background: "white",
          padding: "20px",
          marginTop: "10px",
          borderRadius: "5px",
        }}
      />
      <Modal
        title="Confirm System Reset"
        open={isModalOpen}
        onOk={handleOk}
        onCancel={handleCancel}
        okText="Confirm"
        cancelText="Cancel"
      >
        <p>Are you sure you want to reset the system?</p>
      </Modal>
    </>
  );
}
