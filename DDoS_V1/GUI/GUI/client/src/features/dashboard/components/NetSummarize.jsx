import React, { useState } from "react";
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

  const bitPerSecond =
    (fdata?.onsec?.total?.attack?.bits || 0) +
    (fdata?.onsec?.total?.bypass?.bits || 0);
  const packetPerSecond =
    (fdata?.onsec?.total?.attack?.packets || 0) +
    (fdata?.onsec?.total?.bypass?.packets || 0);

  // NEW: Use accumulated prop for total
  const totalBits =
    (accumulated?.total?.bypass?.bits || 0) +
    (accumulated?.total?.attack?.bits || 0);
  const totalPackets =
    (accumulated?.total?.bypass?.packets || 0) +
    (accumulated?.total?.attack?.packets || 0);

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
                  onClick={onResetBoth}
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
