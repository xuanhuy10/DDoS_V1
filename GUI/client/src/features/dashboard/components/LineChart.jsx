import {
  Card,
  Space,
  Flex,
  Typography,
  Divider,
  Row,
  Col,
  Tooltip,
} from "antd";
import { AreaChartOutlined, InfoCircleOutlined } from "@ant-design/icons";
const { Title } = Typography;

import { PlayCircleTwoTone, PauseCircleTwoTone } from "@ant-design/icons";

import { useEffect, useState, memo, useCallback } from "react"; // Added useState import

import "@/features/dashboard/styles/main.css";

import OneChartImg from "@/assets/icons/onechart.svg";
import DuoChartImg from "@/assets/icons/duochart.svg";
import QuadChartImg from "@/assets/icons/quadchart.svg";

import RateChart from "./lineChart/RateChart";
import AttackChart from "./lineChart/AttackChart";
import ProtocolChart from "./lineChart/ProtocolChart";

import { useTrafficStore } from "@/store/trafficStore"; // Import store

// Chart info component moved outside and memoized
const ChartInfo = memo(({ title }) => {
  const handleMouseEnter = useCallback((e) => {
    e.target.style.opacity = "1";
  }, []);

  const handleMouseLeave = useCallback((e) => {
    e.target.style.opacity = "0.7";
  }, []);

  return (
    <div style={{ position: "absolute", top: 20, right: 8, zIndex: 10 }}>
      <Tooltip title={title} placement="topRight">
        <InfoCircleOutlined
          style={{
            fontSize: "16px",
            color: "#666",
            cursor: "pointer",
            opacity: 0.7,
            transition: "opacity 0.2s",
          }}
          onMouseEnter={handleMouseEnter}
          onMouseLeave={handleMouseLeave}
        />
      </Tooltip>
    </div>
  );
});

ChartInfo.displayName = "ChartInfo";

const TrafficChart = ({ psData }) => {
  const [columns, setColumns] = useState(1); // This line now works with useState imported

  // Lấy trạng thái và các hàm từ store
  const { timeseriesData, pause, updateTimeseriesData, togglePause } =
    useTrafficStore();

  // Cập nhật dữ liệu khi psData thay đổi
  useEffect(() => {
    updateTimeseriesData(psData);
  }, [psData, updateTimeseriesData]);

  return (
    <Card bordered={false} className="db-chart-area">
      <Flex className="area-selector" justify="space-between" align="middle">
        <Space>
          <AreaChartOutlined style={{ fontSize: "1.3rem" }} />
          <Title level={5}>SysnetDef Traffic</Title>
        </Space>
        <Flex
          className="tool-bar"
          justify="space-between"
          align="center"
          gap={10}
        >
          {pause ? (
            <PlayCircleTwoTone
              twoToneColor="#1FFF00"
              style={{ fontSize: 21, cursor: "pointer" }}
              onClick={togglePause}
            />
          ) : (
            <PauseCircleTwoTone
              twoToneColor="#FF4400"
              style={{ fontSize: 21, cursor: "pointer" }}
              onClick={togglePause}
            />
          )}
          <img
            src={OneChartImg}
            alt="One Chart"
            onClick={() => setColumns(1)}
            style={{ opacity: columns === 1 ? 1 : 0.5, cursor: "pointer" }}
          />
          <img
            src={DuoChartImg}
            alt="Duo Chart"
            onClick={() => setColumns(2)}
            style={{ opacity: columns === 2 ? 1 : 0.5, cursor: "pointer" }}
          />
          <img
            src={QuadChartImg}
            alt="Quad Chart"
            onClick={() => setColumns(4)}
            style={{ opacity: columns === 4 ? 1 : 0.5, cursor: "pointer" }}
          />
        </Flex>
      </Flex>
      <Divider style={{ margin: "0.3rem 0", borderColor: "#BCBCBC" }} />
      <div className="chart-container">
        <Row gutter={[16, 16]}>
          {columns === 1 && (
            <Col
              key={1}
              xs={24}
              sm={24}
              md={24}
              lg={24}
              xl={24}
              style={{ height: "100%", position: "relative" }}
            >
              <ChartInfo title="Protocol Chart - Network traffic by protocol type" />
              <ProtocolChart
                protocolData={timeseriesData}
                time={timeseriesData.timestamps}
                col={columns}
                paused={pause}
              />
            </Col>
          )}
          {columns === 2 && (
            <>
              <Col
                key={1}
                xs={24}
                sm={24}
                md={24}
                lg={24}
                xl={24}
                style={{ height: "48%", position: "relative" }}
              >
                <ChartInfo title="Attack Chart - Security threats and attack patterns" />
                <AttackChart
                  attackData={timeseriesData}
                  time={timeseriesData.timestamps}
                  col={columns}
                  paused={pause}
                />
              </Col>
              <Col
                key={2}
                xs={24}
                sm={24}
                md={24}
                lg={columns === 4 ? 12 : 24}
                xl={columns === 4 ? 12 : 24}
                style={{ height: "48%", position: "relative" }}
              >
                <ChartInfo title="Protocol Chart - Network traffic by protocol type" />
                <ProtocolChart
                  protocolData={timeseriesData}
                  time={timeseriesData.timestamps}
                  col={columns}
                  paused={pause}
                />
              </Col>
            </>
          )}
          {columns === 4 && (
            <>
              <Col
                key={1}
                xs={24}
                sm={24}
                md={24}
                lg={24}
                xl={24}
                style={{ height: "48%", position: "relative" }}
              >
                <ChartInfo title="Rate Chart - Network traffic rate and bandwidth usage" />
                <RateChart
                  rateData={timeseriesData}
                  time={timeseriesData.timestamps}
                  col={columns}
                  paused={pause}
                />
              </Col>
              <Col
                key={2}
                xs={24}
                sm={24}
                md={24}
                lg={12}
                xl={12}
                style={{ height: "48%", position: "relative" }}
              >
                <ChartInfo title="Attack Chart - Security threats and attack patterns" />
                <AttackChart
                  attackData={timeseriesData}
                  time={timeseriesData.timestamps}
                  col={columns}
                  paused={pause}
                />
              </Col>
              <Col
                key={3}
                xs={24}
                sm={24}
                md={24}
                lg={12}
                xl={12}
                style={{ height: "48%", position: "relative" }}
              >
                <ChartInfo title="Protocol Chart - Network traffic by protocol type" />
                <ProtocolChart
                  protocolData={timeseriesData}
                  time={timeseriesData.timestamps}
                  col={columns}
                  paused={pause}
                />
              </Col>
            </>
          )}
        </Row>
      </div>
    </Card>
  );
};

export default TrafficChart;
