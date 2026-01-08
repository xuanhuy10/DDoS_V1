import { useEffect, useMemo, useState } from "react";
import { Card, Collapse, Flex, Table, Tag } from "antd";
import Column from "antd/es/table/Column";
import ColumnGroup from "antd/es/table/ColumnGroup";

import { socket } from "@/utils/socket";

import ethactive from "@/assets/icons/ethactive.svg";
import ethunactive from "@/assets/icons/ethunactive.svg";
import ethattack from "@/assets/icons/ethattack.svg"; // Add this import for attack icon
import ddosDeviceLogo from "@/assets/test_logo.svg";
import sfpPort from "@/assets/icons/SFP_Port_Icon.svg";
// import ddosLogo from "@/assets/ddos_logo_black.svg";

import InterfaceMarquee from "./InterfaceStatusMarquee";

import { bitFormatter, cntFormatter } from "@/lib/formatter";

const InterfaceStatus = () => {
  const [activeKey, setActiveKey] = useState(null);
  const [interfaceStatusData, setInterfaceStatusData] = useState(null);
  const [interfaceOneSecData, setInterfaceOneSecData] = useState(null);
  // const [isVisible, setVisible] = useState(false);

  const handleInterfaceData = (status, oneSec) => {
    setInterfaceStatusData(status);
    setInterfaceOneSecData(oneSec);
  };

  useEffect(() => {
    socket.on("interface", handleInterfaceData);
    return () => {
      socket.off("interface", handleInterfaceData);
    };
  }, []);

  // Function to determine port status and icon
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

  const liveData = useMemo(() => {
    if (!interfaceStatusData || !interfaceOneSecData) return [];

    const result = [];

    for (let i = 1; i <= 8; i++) {
      // eth1 to eth8
      const port = `eth${i}`;

      const statusInfo = interfaceStatusData.interface?.[port];
      const trafficInfo = interfaceOneSecData.interface?.[port];

      if (!statusInfo || !trafficInfo) continue;

      const { status } = getPortStatus(port);

      result.push({
        port,
        status: status,
        bypassBits: `${bitFormatter(trafficInfo.bypass.bits)} `,
        bypassPackets: `${bitFormatter(trafficInfo.bypass.packets)} `,
        attackBits: `${bitFormatter(trafficInfo.attack.bits)} `,
        attackPackets: `${bitFormatter(trafficInfo.attack.packets)} `,
      });
    }

    return result;
  }, [interfaceStatusData, interfaceOneSecData]);

  const items = [
    {
      key: "interface",
      label: (
        <Flex
          wrap="wrap"
          align="center"
          justify="space-around"
          gap="20px"
          style={{ marginTop: "10px", marginBottom: "10px" }}
        >
          <img src={ddosDeviceLogo} alt="Ddos Logo" style={{ width: 110 }} />
          <Flex
            wrap="wrap"
            justify="center"
            align="center"
            gap="8px" // Reduced gap for tighter spacing
            style={{ maxWidth: "100%" }}
          >
            {[...Array(8)].map((_, index) => {
              const portNumber = index + 1;
              const port = `eth${portNumber}`;
              const { status, icon, isUnderAttack } = getPortStatus(port);
              const isActive = interfaceStatusData?.interface?.[port]?.status;

              return (
                <div
                  key={index}
                  style={{
                    position: "relative",
                    display: "flex",
                    flexDirection: "column",
                    alignItems: "center",
                    transition: "all 0.3s ease",
                  }}
                >
                  <img
                    src={icon}
                    alt={`interface eth${portNumber}`}
                    style={{
                      width: 40,
                      opacity: isActive ? 1 : 0.6, // Less gray for inactive
                      filter: isActive ? "none" : "grayscale(30%)", // Subtle grayscale
                    }}
                  />
                  <span
                    style={{
                      fontSize: "10px",
                      fontWeight: "bold",
                      color: isUnderAttack
                        ? "#D12200"
                        : isActive
                        ? "#0073D1"
                        : "#888",
                      marginTop: "2px",
                    }}
                  >
                    {portNumber}
                  </span>
                </div>
              );
            })}
          </Flex>

          <Flex gap="20px" align="center">
            <div
              style={{
                display: "flex",
                flexDirection: "column",
                alignItems: "center",
              }}
            >
              <img src={sfpPort} alt="SFP Port 1" style={{ width: 50 }} />
              <span
                style={{
                  fontSize: "14px",
                  fontWeight: "bold",
                  color: "#0073D1",
                  marginTop: "2px",
                }}
              >
                SFP1
              </span>
            </div>
            <div
              style={{
                display: "flex",
                flexDirection: "column",
                alignItems: "center",
              }}
            >
              <img src={sfpPort} alt="SFP Port 2" style={{ width: 50 }} />
              <span
                style={{
                  fontSize: "14px",
                  fontWeight: "bold",
                  color: "#0073D1",
                  marginTop: "2px",
                }}
              >
                SFP2
              </span>
            </div>
          </Flex>

          <div
            style={{
              display: "flex",
              flexDirection: "column",
              alignItems: "center",
            }}
          >
            <Card
              style={{
                flex: "1",
                maxWidth: 200,
                border: "1px solid #0073D1",
                borderRadius: "5px",
                backgroundColor: "#F0F4F8",
              }}
            >
              <InterfaceMarquee />
            </Card>
          </div>
        </Flex>
      ),
      children: (
        <Table
          dataSource={liveData}
          pagination={false}
          size="small"
          rowKey="port"
        >
          <Column
            title="Interface"
            dataIndex="port"
            key="port"
            align="center"
            style={{ width: "100%" }}
          />
          <Column
            title="Status"
            dataIndex="status"
            key="status"
            align="center"
            style={{ width: "100%" }}
            render={(text) => (
              <span
                style={{
                  display: "flex",
                  justifyContent: "center",
                  alignItems: "center",
                }}
              >
                <Tag
                  color={
                    text === "Processing"
                      ? "green"
                      : text === "Under Attack"
                      ? "red"
                      : text === "Active"
                      ? "blue"
                      : "gray"
                  }
                  style={{ fontWeight: "bold", textSize: "14px" }}
                >
                  {text.toUpperCase()}
                </Tag>
              </span>
            )}
          />
          <ColumnGroup title="Bypass" key="bypass">
            <Column
              title="Bit"
              dataIndex="bypassBits"
              key="bypassBits"
              align="center"
              style={{ width: "100%" }}
              render={(text) => (
                <span style={{ color: "#0073D1", fontWeight: "bold" }}>
                  {text}
                </span>
              )}
            />
            <Column
              title="Packet"
              dataIndex="bypassPackets"
              key="bypassPackets"
              align="center"
              style={{ width: "100%" }}
              render={(text) => (
                <span style={{ color: "#0073D1", fontWeight: "bold" }}>
                  {text}
                </span>
              )}
            />
          </ColumnGroup>
          <ColumnGroup title="Attack" key="attack">
            <Column
              title="Bit"
              dataIndex="attackBits"
              key="attackBits"
              align="center"
              style={{ width: "100%" }}
              render={(text) => (
                <span style={{ color: "#D12200", fontWeight: "bold" }}>
                  {text}
                </span>
              )}
            />
            <Column
              title="Packet"
              dataIndex="attackPackets"
              key="attackPackets"
              align="center"
              style={{ width: "100%" }}
              render={(text) => (
                <span style={{ color: "#D12200", fontWeight: "bold" }}>
                  {text}
                </span>
              )}
            />
          </ColumnGroup>
        </Table>
      ),
      showArrow: false,
    },
  ];

  return (
    <div
      onMouseEnter={function () {
        setActiveKey("interface");
      }}
      onMouseLeave={function () {
        setActiveKey(null);
      }}
    >
      <Collapse accordion activeKey={activeKey} items={items} bordered={true} />
    </div>
  );
};

export default InterfaceStatus;
