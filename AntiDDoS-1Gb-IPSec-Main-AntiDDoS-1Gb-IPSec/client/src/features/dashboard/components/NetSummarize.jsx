import React, { useState, useEffect } from "react";
import { Card, Col, Typography, Button } from "antd";
import { PieChartTwoTone, ReloadOutlined } from "@ant-design/icons";
import "@/features/dashboard/styles/main.css";
import { InfoModal } from "@/components/common/InfoModal";
import { SummaryByteChart, SummaryPacketChart } from "./PieChart";
import { bitFormatter, cntFormatter } from "@/lib/formatter";
import AdaptiveAreaChart from "./AdaptiveAreaChart.jsx";

const { Title } = Typography;

const NetSummarize = ({ fdata, sdata, accumulated, onResetBoth }) => {
  const [isBytesModalVisible, setBytesModalVisible] = useState(false);
  const [isModalVisible, setModalVisible] = useState(false);
  const [totalBackup, setTotalBackup] = useState({ bits: 0, packets: 0 });
  const [prevAccumulated, setPrevAccumulated] = useState(null);

  // Lấy totalBackup từ localStorage khi mount (lịch sử vĩnh viễn)
  useEffect(() => {
    const savedBackup = localStorage.getItem("total_backup");
    if (savedBackup) {
      setTotalBackup(JSON.parse(savedBackup));
    }
  }, []);

  // Cập nhật totalBackup với delta khi accumulated thay đổi
  useEffect(() => {
    if (accumulated && prevAccumulated) {
      // Tính delta
      const currentBits =
        (accumulated.total?.bypass?.bits || 0) +
        (accumulated.total?.attack?.bits || 0);
      const prevBits =
        (prevAccumulated.total?.bypass?.bits || 0) +
        (prevAccumulated.total?.attack?.bits || 0);
      const deltaBits = currentBits - prevBits;

      const currentPackets =
        (accumulated.total?.bypass?.packets || 0) +
        (accumulated.total?.attack?.packets || 0);
      const prevPackets =
        (prevAccumulated.total?.bypass?.packets || 0) +
        (prevAccumulated.total?.attack?.packets || 0);
      const deltaPackets = currentPackets - prevPackets;

      // Chỉ cộng nếu delta > 0 (tránh âm khi PM2 reset hoặc lỗi)
      if (deltaBits > 0 || deltaPackets > 0) {
        const updatedBackup = {
          bits: totalBackup.bits + Math.max(0, deltaBits),
          packets: totalBackup.packets + Math.max(0, deltaPackets),
        };

        setTotalBackup(updatedBackup);
        localStorage.setItem("total_backup", JSON.stringify(updatedBackup));
      }
    }

    // Luôn cập nhật prev (kể cả lần đầu hoặc sau reset)
    setPrevAccumulated(accumulated);
  }, [accumulated]);

  // Hàm xử lý reset: Clear backup về 0 (reset lịch sử), reset server
  const handleReset = () => {
    setTotalBackup({ bits: 0, packets: 0 });
    localStorage.removeItem("total_backup");
    setPrevAccumulated(null); // Reset prev để skip delta lần đầu sau reset
    onResetBoth(); // Reset server accumulated về 0
  };

  const bitPerSecond =
    (fdata?.onsec?.total?.attack?.bits || 0) +
    (fdata?.onsec?.total?.bypass?.bits || 0);
  const packetPerSecond =
    (fdata?.onsec?.total?.attack?.packets || 0) +
    (fdata?.onsec?.total?.bypass?.packets || 0);

  // Hiển thị total = totalBackup (đã bao gồm tất cả delta lịch sử + hiện tại qua updates)
  const totalBits = totalBackup.bits;
  const totalPackets = totalBackup.packets;

  return (
    <>
      <Col xs={12} sm={12} md={12} lg={12} xl={5}>
        <Card bordered={false}>
          <div className="summary-box">
            <div
              style={{ display: "flex", alignItems: "center", height: "100%" }}
            >
              <div style={{ flex: "1" }}>
                <div
                  style={{
                    color: "#8c8c8c",
                    fontSize: "14px",
                    marginBottom: "8px",
                    fontWeight: "400",
                  }}
                >
                  Incoming bps
                </div>
                <div
                  style={{
                    fontSize: "24px",
                    fontWeight: "600",
                    color: "#262626",
                    margin: 0,
                  }}
                >
                  {bitFormatter(bitPerSecond) + "/s"}
                </div>
              </div>
              <div style={{ flex: "0 0 150px", marginRight: "10px" }}>
                <AdaptiveAreaChart
                  data={bitPerSecond}
                  label="Bits"
                  color="#52c41a"
                  height={80}
                  width={150}
                />
              </div>
            </div>
          </div>
        </Card>
      </Col>
      <Col xs={12} sm={12} md={12} lg={12} xl={5}>
        <Card bordered={false}>
          <div className="summary-box">
            <div
              style={{ display: "flex", alignItems: "center", height: "100%" }}
            >
              <div style={{ flex: "1" }}>
                <div
                  style={{
                    color: "#8c8c8c",
                    fontSize: "14px",
                    marginBottom: "8px",
                    fontWeight: "400",
                  }}
                >
                  Incoming pps
                </div>
                <div
                  style={{
                    fontSize: "24px",
                    fontWeight: "600",
                    color: "#262626",
                    margin: 0,
                  }}
                >
                  {cntFormatter(packetPerSecond) + "/s"}
                </div>
              </div>
              <div style={{ flex: "0 0 150px", marginRight: "10px" }}>
                <AdaptiveAreaChart
                  data={packetPerSecond}
                  label="Packets"
                  color="#f5222d"
                  height={80}
                  width={150}
                />
              </div>
            </div>
          </div>
        </Card>
      </Col>
      <Col xs={12} sm={12} md={12} lg={12} xl={4}>
        <Card bordered={false}>
          <div className="summary-box" style={{ height: "80px" }}>
            <div
              style={{ display: "flex", alignItems: "center", height: "100%" }}
            >
              <div style={{ flex: "1" }}>
                <div
                  style={{
                    display: "flex",
                    alignItems: "center",
                    justifyContent: "space-between",
                  }}
                >
                  <div
                    style={{
                      color: "#8c8c8c",
                      fontSize: "14px",
                      marginBottom: "8px",
                      fontWeight: "400",
                    }}
                  >
                    Total Bits
                  </div>
                </div>
                <Title level={3} className="box-info">
                  <div
                    style={{
                      fontSize: "24px",
                      fontWeight: "600",
                      color: "#262626",
                      margin: 0,
                    }}
                  >
                    {bitFormatter(totalBits)}
                  </div>
                </Title>
              </div>
              <div style={{ flex: "0 0 10px" }}>
                <PieChartTwoTone
                  twoToneColor="#52c41a"
                  style={{ fontSize: "32px" }}
                  onClick={() => setBytesModalVisible(true)}
                />
              </div>
              <InfoModal
                title="Total bits"
                visible={isBytesModalVisible}
                setVisible={setBytesModalVisible}
              >
                <SummaryByteChart
                  sdata={sdata}
                  fdata={fdata}
                  accumulated={accumulated}
                />
              </InfoModal>
            </div>
          </div>
        </Card>
      </Col>
      <Col xs={12} sm={12} md={12} lg={12} xl={4}>
        <Card bordered={false}>
          <div
            className="summary-box"
            style={{ height: "80px", display: "flex", alignItems: "center" }}
          >
            <div style={{ flex: "1" }}>
              <div
                style={{
                  display: "flex",
                  alignItems: "center",
                  justifyContent: "space-between",
                  marginBottom: "8px",
                }}
              >
                <div
                  style={{
                    color: "#8c8c8c",
                    fontSize: "14px",
                    fontWeight: "400",
                  }}
                >
                  Total packets
                </div>
                <Button
                  type="default"
                  shape="circle"
                  icon={<ReloadOutlined />}
                  onClick={handleReset}
                />
              </div>
              <Title level={3} className="box-info">
                <div
                  style={{
                    fontSize: "24px",
                    fontWeight: "600",
                    color: "#262626",
                    margin: 0,
                  }}
                >
                  {cntFormatter(totalPackets)}
                </div>
              </Title>
            </div>
            <div style={{ flex: "0 0 10px" }}>
              <PieChartTwoTone
                twoToneColor="#52c41a"
                style={{ fontSize: "32px" }}
                onClick={() => setModalVisible(true)}
              />
            </div>
            <InfoModal
              title="Total Packets"
              visible={isModalVisible}
              setVisible={setModalVisible}
            >
              <SummaryPacketChart
                sdata={sdata}
                fdata={fdata}
                accumulated={accumulated}
              />
            </InfoModal>
          </div>
        </Card>
      </Col>
    </>
  );
};

export default NetSummarize;
