import { useEffect, useState } from "react";
import { Col, Flex, Row, Space } from "antd";
import { useTranslation } from "react-i18next";
import { socket } from "@/utils/socket";
import DefenseProfile from "@/features/dashboard/components/DefenseProfile";
import NetStatus from "@/features/dashboard/components/NetStatus";
import InterfaceStatus from "@/features/dashboard/components/InterfaceStatus";
import NetSummarize from "@/features/dashboard/components/NetSummarize";
import LineChart from "@/features/dashboard/components/LineChart";
import BarChart from "@/features/dashboard/components/BarChart";
import SystemLog from "@/features/dashboard/components/SystemLog";
import TrafficLog from "@/features/dashboard/components/TrafficLog";

export default function Dashboard() {
  const { t } = useTranslation();
  const [trafficData, setTrafxData] = useState(null);
  const [chartSumData, setChartData] = useState(null);
  const [accumulated, setAccumulated] = useState(null); // Lưu accumulated từ server
  const [resetTimestamp, setResetTimestamp] = useState(null);
  const [isResetting, setIsResetting] = useState(false);

  // Lấy initial accumulated khi mount
  useEffect(() => {
    socket.emit("get_accumulated");
    const handleAccumulated = (data) => {
      console.log("Received initial accumulated from server:", data);
      setAccumulated(data);
    };
    socket.on("accumulated", handleAccumulated);
    return () => {
      socket.off("accumulated", handleAccumulated);
    };
  }, []);

  const handleResetBoth = () => {
    setIsResetting(true);
    socket.emit("reset_both");

    socket.once("reset_confirm", (previous) => {
      console.log("Reset confirmed, previous:", previous);
      setAccumulated({
        total: {
          bypass: { bits: 0, packets: 0 },
          attack: { bits: 0, packets: 0 },
        },
      });
      setTrafxData((prev) => ({
        ...prev,
        summary: {
          total: {
            bypass: { bits: 0, packets: 0 },
            attack: { bits: 0, packets: 0 },
          },
          onsec: prev?.onsec || 0,
          status: prev?.status || "normal",
        },
      }));
      setResetTimestamp(Date.now());
      setIsResetting(false);
    });

    // Fallback timeout
    setTimeout(() => {
      setIsResetting(false);
    }, 2000);
  };

  useEffect(() => {
    function HandleSummaryData({ summary, onsec, status, serverTimestamp }) {
      if (isResetting) {
        console.log("Ignoring data during reset");
        return;
      }
      if (
        resetTimestamp &&
        serverTimestamp &&
        serverTimestamp < resetTimestamp
      ) {
        console.log("Ignoring old data before reset:", serverTimestamp);
        return;
      }

      console.log("Received data from server:", {
        summary,
        onsec,
        serverTimestamp,
      });

      // Cập nhật accumulated từ summary (persistent từ server)
      setAccumulated(summary);

      // Cập nhật trafficData với summary và onsec từ server
      setTrafxData((prev) => ({
        ...prev,
        summary,
        onsec,
        status,
      }));
    }
    socket.on("traffic", HandleSummaryData);
    return () => {
      socket.off("traffic");
    };
  }, [resetTimestamp, isResetting]);

  useEffect(() => {
    if (accumulated || trafficData?.summary) {
      setChartData(processData(accumulated || trafficData.summary));
    } else {
      setChartData(null);
    }
  }, [trafficData, accumulated]);

  const processData = (data) => {
    try {
      const defaultCategory = {
        bypass: { bits: 0, packets: 0 },
        attack: { bits: 0, packets: 0 },
      };
      const protocol = data.protocol || {
        icmp: defaultCategory,
        udp: defaultCategory,
        tcp: defaultCategory,
        http: defaultCategory,
        esp: defaultCategory,
        unknown: defaultCategory,
      };

      const getData = (category) => {
        if (category === "total") {
          return {
            bits: {
              received: data.total?.bypass?.bits || 0,
              drop: -(data.total?.attack?.bits || 0),
              total:
                (data.total?.bypass?.bits || 0) +
                (data.total?.attack?.bits || 0),
            },
            packets: {
              received: data.total?.bypass?.packets || 0,
              drop: -(data.total?.attack?.packets || 0),
              total:
                (data.total?.bypass?.packets || 0) +
                (data.total?.attack?.packets || 0),
            },
          };
        }

        const categoryData = protocol[category] || defaultCategory;

        return {
          bits: {
            received: categoryData.bypass?.bits || 0,
            drop: -(categoryData.attack?.bits || 0),
            total:
              (categoryData.bypass?.bits || 0) +
              (categoryData.attack?.bits || 0),
          },
          packets: {
            received: categoryData.bypass?.packets || 0,
            drop: -(categoryData.attack?.packets || 0),
            total:
              (categoryData.bypass?.packets || 0) +
              (categoryData.attack?.packets || 0),
          },
        };
      };

      return {
        total: getData("total"),
        icmp: getData("icmp"),
        udp: getData("udp"),
        tcp: getData("tcp"),
        http: getData("http"),
        esp: getData("esp"),
        unknown: getData("unknown"),
      };
    } catch (error) {
      console.error("Error processing data:", error);
      return null;
    }
  };

  return (
    <div>
      <Space direction={"vertical"} size={10} style={{ width: "100%" }}>
        <Row gutter={[12, 16]}>
          <Col xs={24} sm={24} md={24} lg={24} xl={6}>
            <NetStatus
              psdata={trafficData?.onsec}
              status={trafficData?.status}
            />
          </Col>
          <NetSummarize
            fdata={trafficData}
            sdata={chartSumData}
            accumulated={accumulated}
            onResetBoth={handleResetBoth}
          />
        </Row>

        <Row gutter={[12, 16]}>
          <Flex style={{ height: "100%", width: "100%" }}>
            <Col
              xs={24}
              sm={24}
              md={24}
              lg={24}
              xl={24}
              style={{ height: "100%", maxWidth: "30%" }}
            >
              <BarChart sdata={chartSumData} />
            </Col>
            <Col
              xs={24}
              sm={24}
              md={24}
              lg={24}
              xl={24}
              style={{ height: "100%", maxWidth: "70%" }}
            >
              <LineChart psData={trafficData?.onsec} />
            </Col>
          </Flex>
        </Row>

        <Row gutter={[12, 16]}>
          <Col xs={24} sm={24} md={24} lg={24} xl={15}>
            <Space direction={"vertical"} style={{ width: "100%" }}>
              <InterfaceStatus />
              <TrafficLog />
            </Space>
          </Col>
          <Col xs={24} sm={24} md={24} lg={24} xl={9}>
            <Space direction={"vertical"} style={{ width: "100%" }}>
              <SystemLog />
              <DefenseProfile />
            </Space>
          </Col>
        </Row>
      </Space>
    </div>
  );
}
