import React, { useRef, useState } from "react";
import {
  Flex,
  Card,
  Row,
  Col,
  Space,
  Button,
  Table,
  DatePicker,
  Form,
  Typography,
} from "antd";
import {
  ClockCircleOutlined,
  ExportOutlined,
  SearchOutlined,
} from "@ant-design/icons";

const { Title } = Typography;
const { RangePicker } = DatePicker;

import dayjs from "dayjs";

import { bitFormatter, cntFormatter } from "@/lib/formatter";
import "@/features/analyze/styles/main.css";

import { analyzeNetwork } from "@/features/api/Network";

import PageTitle from "@/components/common/PageTitle";
import TrafficChart from "@/features/analyze/components/TrafficChart";
import TopChart from "@/features/analyze/components/TopChart";
import TrendChart from "@/features/analyze/components/TrendChart";

import jsPDF from "jspdf";
import html2canvas from "html2canvas";
import autoTable from "jspdf-autotable";
import logoSrc from "@/assets/ddos_logo_black.svg?url";
import { message } from "antd";

const generateColorForIP = (ip, existingColors) => {
  const hashIP = (ip) => {
    let hash = 0;
    for (let i = 0; i < ip.length; i++) {
      const char = ip.charCodeAt(i);
      hash = (hash << 5) - hash + char;
      hash = hash & hash; // Convert to 32-bit int
    }
    return Math.abs(hash) % 360; // Hue range 0-359
  };

  const hslToHex = (h, s, l) => {
    l /= 100;
    const a = (s * Math.min(l, 1 - l)) / 100;
    const f = (n) => {
      const k = (n + h / 30) % 12;
      const color = l - a * Math.max(Math.min(k - 3, 9 - k, 1), -1);
      return Math.round(255 * color)
        .toString(16)
        .padStart(2, "0");
    };
    return `#${f(0)}${f(8)}${f(4)}`;
  };

  let hue = hashIP(ip); // Use full IP hash for better IPv6 support
  let attempts = 0;
  const maxAttempts = 360;
  let color;

  // Ensure no duplicates
  do {
    color = hslToHex(hue, 40 + Math.random() * 20, 65 + Math.random() * 10);
    hue = (hue + 137) % 360; // Prime number step for variation
    attempts++;
  } while (existingColors.includes(color) && attempts < maxAttempts);

  return color;
};

const ipColors = {};

// Helper function to sanitize IP for CSS class name (replace invalid chars like : with -)
const sanitizeForCSS = (ip) => {
  return ip.replace(/[:.]/g, "-"); // Replace both : and . with -
};

export default function Analyze() {
  const now = dayjs();
  const dateFormat = "YYYY-MM-DD HH:mm:ss";
  const chartComponentRef = useRef(null);
  const [form] = Form.useForm();
  const [timeRange, setTimeRange] = useState({});
  const [isOverlayVisible, setIsOverlayVisible] = useState(true);
  const [isAnalyzeLoading, setIsAnalyzeLoading] = useState(false);
  const [isExporting, setIsExporting] = useState(false);
  const [sortField, setSortField] = useState("bits");
  const [sortOrder, setSortOrder] = useState("descend");

  const disabledDate = (current) => {
    // current là giá trị ngày đang được kiểm tra (dayjs object)
    return current && current > now.endOf("day");
  };

  // Vô hiệu hóa thời gian trong tương lai nếu chọn ngày hiện tại
  const disabledRangeTime = (current, type) => {
    if (!current) return {};

    // Nếu ngày được chọn là ngày hiện tại
    const isToday = current.isSame(now, "day");
    if (!isToday) return {};

    const currentHour = now.hour();
    const currentMinute = now.minute();
    const currentSecond = now.second();

    return {
      disabledHours: () =>
        isToday
          ? Array.from({ length: 24 }, (_, i) => i).filter(
              (h) => h > currentHour
            )
          : [],
      disabledMinutes: (selectedHour) =>
        isToday && selectedHour === currentHour
          ? Array.from({ length: 60 }, (_, i) => i).filter(
              (m) => m > currentMinute
            )
          : [],
      disabledSeconds: (selectedHour, selectedMinute) =>
        isToday &&
        selectedHour === currentHour &&
        selectedMinute === currentMinute
          ? Array.from({ length: 60 }, (_, i) => i).filter(
              (s) => s > currentSecond
            )
          : [],
    };
  };

  const address_sum_columns = [
    {
      title: "IP Address",
      dataIndex: "ip",
      key: "ip",
      width: 150,
      sorter: (a, b) => a.ip.localeCompare(b.ip),
      render: (text) => (
        <span
          style={{
            display: "block",
            overflow: "hidden",
            textOverflow: "ellipsis",
            whiteSpace: "nowrap",
          }}
        >
          {text}
        </span>
      ),
    },
    {
      title: "Bits",
      dataIndex: "bits",
      key: "bits",
      width: 120,
      render: (text) => (
        <span
          style={{
            display: "block",
            overflow: "hidden",
            textOverflow: "ellipsis",
            whiteSpace: "nowrap",
          }}
        >
          {bitFormatter(text)}
        </span>
      ),
      sorter: (a, b) => a.bits - b.bits,
    },
    {
      title: "Packets",
      dataIndex: "packets",
      key: "packets",
      width: 120,
      render: (text) => (
        <span
          style={{
            display: "block",
            overflow: "hidden",
            textOverflow: "ellipsis",
            whiteSpace: "nowrap",
          }}
        >
          {cntFormatter(text)}
        </span>
      ),
      sorter: (a, b) => a.packets - b.packets,
    },
  ];

  const [trafficData, setTrafficData] = useState([]);
  const [topSrcChartData, setTopSrcChartData] = useState([]);
  const [sumSrcTableData, setSumSrcTableData] = useState([]);
  const [topDstChartData, setTopDstChartData] = useState([]);
  const [sumDstTableData, setSumDstTableData] = useState([]);
  const [trendProtocolData, setTrendProtocolData] = useState([]);
  const [trendAttackData, setTrendAttackData] = useState([]);

  const [trafficSumData, setTrafficSumData] = useState({
    total: { bypass: { bits: 0, packets: 0 }, attack: { bits: 0, packets: 0 } },
    peak: { bypass: { bps: 0, pps: 0 }, attack: { bps: 0, pps: 0 } },
    avg: { bypass: { bps: 0, pps: 0 }, attack: { bps: 0, pps: 0 } },
  });

  const distributeDataToChart = (data) => {
    const throughputData = [["Type"]];
    const topSrcData = [["Source"]];
    const topDstData = [["Destination"]];

    const timestamps = new Set();
    const normalThroughputs = [];
    const attackThroughputs = [];
    const srcIpMap = new Map();
    const dstIpMap = new Map();

    data.forEach((entry) => {
      Object.entries(entry).forEach(([timestamp, records]) => {
        let normalThroughput = 0;
        let attackThroughput = 0;

        timestamps.add(timestamp);

        records.forEach((record) => {
          const { SrcIp, DstIp, Type, bps } = record;

          if (!srcIpMap.has(SrcIp)) srcIpMap.set(SrcIp, new Map());
          srcIpMap
            .get(SrcIp)
            .set(timestamp, (srcIpMap.get(SrcIp).get(timestamp) || 0) + bps);

          if (!dstIpMap.has(DstIp)) dstIpMap.set(DstIp, new Map());
          dstIpMap
            .get(DstIp)
            .set(timestamp, (dstIpMap.get(DstIp).get(timestamp) || 0) + bps);

          if (Type === "Normal") normalThroughput += bps;
          else attackThroughput += bps;
        });

        normalThroughputs.push(normalThroughput);
        attackThroughputs.push(attackThroughput);
      });
    });

    const sortedTimestamps = Array.from(timestamps).sort();

    topSrcData[0].push(...sortedTimestamps);
    srcIpMap.forEach((valueMap, srcIp) => {
      const row = [srcIp];
      sortedTimestamps.forEach((timestamp) =>
        row.push(valueMap.get(timestamp) || 0)
      );
      topSrcData.push(row);
    });

    topDstData[0].push(...sortedTimestamps);
    dstIpMap.forEach((valueMap, dstIp) => {
      const row = [dstIp];
      sortedTimestamps.forEach((timestamp) =>
        row.push(valueMap.get(timestamp) || 0)
      );
      topDstData.push(row);
    });

    throughputData[0].push(...sortedTimestamps);
    throughputData.push(["Normal", ...normalThroughputs]);
    throughputData.push(["Attack", ...attackThroughputs]);

    setTrafficData(throughputData);
    setTopSrcChartData(topSrcData);
    setTopDstChartData(topDstData);
  };

  const distributeSummaryData = async (data, from, to) => {
    const diffInSeconds =
      to.isValid() && from.isValid() ? to.diff(from, "seconds") : 1;

    const trafficSum = {
      total: {
        bypass: {
          bits: data.total.bypass.bits,
          packets: data.total.bypass.packets,
        },
        attack: {
          bits: data.total.attack.bits,
          packets: data.total.attack.packets,
        },
      },
      peak: {
        bypass: { bps: data.peak.bypass.bps, pps: data.peak.bypass.pps },
        attack: { bps: data.peak.attack.bps, pps: data.peak.attack.pps },
      },
      avg: {
        bypass: {
          bps: data.total.bypass.bits / diffInSeconds,
          pps: data.total.bypass.packets / diffInSeconds,
        },
        attack: {
          bps: data.total.attack.bits / diffInSeconds,
          pps: data.total.attack.packets / diffInSeconds,
        },
      },
    };

    const existingColors = Object.values(ipColors);
    const sourceIpTableData = Object.keys(data.srcIp)
      .map((ip) => {
        if (!ipColors[ip]) {
          ipColors[ip] = generateColorForIP(ip, existingColors);
          existingColors.push(ipColors[ip]);
        }
        return {
          key: ip,
          ip,
          packets: data.srcIp[ip].packets,
          bits: data.srcIp[ip].bits,
        };
      })
      .sort((a, b) => b.bits - a.bits)
      .slice(0, 10);

    const destIpTableData = Object.entries(data.dstIp)
      .map(([ip, value]) => {
        if (!ipColors[ip]) {
          ipColors[ip] = generateColorForIP(ip, existingColors);
          existingColors.push(ipColors[ip]);
        }
        return {
          key: ip,
          ip,
          packets: value.packets,
          bits: value.bits,
        };
      })
      .sort((a, b) => b.bits - a.bits)
      .slice(0, 10);

    setSumSrcTableData(sourceIpTableData);
    setSumDstTableData(destIpTableData);
    setTrendProtocolData(data.protocol);
    setTrendAttackData(data.attack);
    setTrafficSumData(trafficSum);
  };

  const HandleTimeSelect = async (values) => {
    setIsAnalyzeLoading(true);

    const from = values.timeRange[0];
    const to = values.timeRange[1];

    setTimeRange({ from: from.format(dateFormat), to: to.format(dateFormat) });

    try {
      const chartData = await analyzeNetwork(
        from.format(dateFormat),
        to.format(dateFormat)
      );
      const summaryData = chartData.data?.summary || {};
      const timeSeriesData = chartData.data?.timeSeries || [];

      // Check if data is empty or invalid
      if (!summaryData || Object.keys(summaryData).length === 0) {
        setTrafficSumData({
          total: {
            bypass: { bits: 0, packets: 0 },
            attack: { bits: 0, packets: 0 },
          },
          peak: { bypass: { bps: 0, pps: 0 }, attack: { bps: 0, pps: 0 } },
          avg: { bypass: { bps: 0, pps: 0 }, attack: { bps: 0, pps: 0 } },
        });
        setSumSrcTableData([]);
        setSumDstTableData([]);
        setTrendProtocolData({});
        setTrendAttackData([]);
        setTrafficData([["Time"], ...getEmptyTimeRange(from, to)]);
        setTopSrcChartData([["Source"], ...getEmptyIpList()]);
        setTopDstChartData([["Destination"], ...getEmptyIpList()]);
      } else {
        await distributeSummaryData(summaryData, from, to);
        distributeDataToChart(timeSeriesData);
      }
    } catch (error) {
      // Set default zeroed-out values on error
      setTrafficSumData({
        total: {
          bypass: { bits: 0, packets: 0 },
          attack: { bits: 0, packets: 0 },
        },
        peak: { bypass: { bps: 0, pps: 0 }, attack: { bps: 0, pps: 0 } },
        avg: { bypass: { bps: 0, pps: 0 }, attack: { bps: 0, pps: 0 } },
      });
      setSumSrcTableData([]);
      setSumDstTableData([]);
      setTrendProtocolData({});
      setTrendAttackData([]);
      setTrafficData([["Time"], ...getEmptyTimeRange()]);
      setTopSrcChartData([["Source"], ...getEmptyIpList()]);
      setTopDstChartData([["Destination"], ...getEmptyIpList()]);
    }

    setIsAnalyzeLoading(false);
    setIsOverlayVisible(false);
  };

  // Helper functions to generate empty data
  const getEmptyTimeRange = (from, to) => {
    const fallbackFrom =
      from && dayjs.isDayjs(from) ? from : dayjs().subtract(1, "week");
    const fallbackTo = to && dayjs.isDayjs(to) ? to : dayjs();
    const timestamps = [];
    const start = fallbackFrom.unix();
    const end = fallbackTo.unix();
    const step = Math.floor((end - start) / 10) || 1;
    for (let i = start; i <= end; i += step) {
      timestamps.push([dayjs.unix(i).format("YYYY-MM-DD HH:mm:ss"), 0]);
    }
    return timestamps;
  };

  const getEmptyIpList = () => {
    const timestamps = getEmptyTimeRange(
      dayjs().subtract(1, "week"),
      dayjs()
    ).map(([time]) => time);
    return [
      ["Source", ...timestamps],
      ["192.168.0.1", ...Array(timestamps.length).fill(0)],
      ["192.168.0.2", ...Array(timestamps.length).fill(0)],
    ];
  };

  const getTimeDifference = () => {
    if (timeRange.from && timeRange.to) {
      const from = dayjs(timeRange.from);
      const to = dayjs(timeRange.to);
      const diffInHours = to.diff(from, "hours");
      const days = Math.floor(diffInHours / 24);
      const hours = diffInHours % 24;
      return `${days} days, ${hours} hours`;
    }
    return "";
  };

  const formatProtocolTrend = () => {
    const totalBits = Object.values(trendProtocolData).reduce(
      (sum, p) => sum + (p.bypass?.bits || 0) + (p.attack?.bits || 0),
      0
    );
    return [
      [
        "TCP",
        bitFormatter(
          (trendProtocolData?.tcp?.bypass?.bits || 0) +
            (trendProtocolData?.tcp?.attack?.bits || 0)
        ),
        calcPercent(trendProtocolData?.tcp, totalBits),
      ],
      [
        "UDP",
        bitFormatter(
          (trendProtocolData?.udp?.bypass?.bits || 0) +
            (trendProtocolData?.udp?.attack?.bits || 0)
        ),
        calcPercent(trendProtocolData?.udp, totalBits),
      ],
      [
        "DNS",
        bitFormatter(
          (trendProtocolData?.dns?.bypass?.bits || 0) +
            (trendProtocolData?.dns?.attack?.bits || 0)
        ),
        calcPercent(trendProtocolData?.dns, totalBits),
      ],
      [
        "ICMP",
        bitFormatter(
          (trendProtocolData?.icmp?.bypass?.bits || 0) +
            (trendProtocolData?.icmp?.attack?.bits || 0)
        ),
        calcPercent(trendProtocolData?.icmp, totalBits),
      ],
      [
        "HTTP",
        bitFormatter(
          (trendProtocolData?.http?.bypass?.bits || 0) +
            (trendProtocolData?.http?.attack?.bits || 0)
        ),
        calcPercent(trendProtocolData?.http, totalBits),
      ],
      [
        "ESP",
        bitFormatter(
          (trendProtocolData?.esp?.bypass?.bits || 0) +
            (trendProtocolData?.esp?.attack?.bits || 0)
        ),
        calcPercent(trendProtocolData?.esp, totalBits),
      ],
      [
        "Unknown",
        bitFormatter(
          (trendProtocolData?.unknown?.bypass?.bits || 0) +
            (trendProtocolData?.unknown?.attack?.bits || 0)
        ),
        calcPercent(trendProtocolData?.unknown, totalBits),
      ],
    ];
  };

  const formatAttackTrend = () => {
    const totalBits = Object.values(trendAttackData).reduce(
      (sum, a) => sum + (a.bits || 0),
      0
    );
    return [
      [
        "SYN Flood",
        bitFormatter(trendAttackData?.synFlood?.bits || 0),
        calcPercent(trendAttackData?.synFlood, totalBits),
      ],
      [
        "UDP Flood",
        bitFormatter(trendAttackData?.udpFlood?.bits || 0),
        calcPercent(trendAttackData?.udpFlood, totalBits),
      ],
      [
        "ICMP Flood",
        bitFormatter(trendAttackData?.icmpFlood?.bits || 0),
        calcPercent(trendAttackData?.icmpFlood, totalBits),
      ],
      [
        "DNS Flood",
        bitFormatter(trendAttackData?.dnsFlood?.bits || 0),
        calcPercent(trendAttackData?.dnsFlood, totalBits),
      ],
      [
        "HTTP Flood",
        bitFormatter(trendAttackData?.httpFlood?.bits || 0),
        calcPercent(trendAttackData?.httpFlood, totalBits),
      ],
      [
        "IPSec IKE",
        bitFormatter(trendAttackData?.ipsec?.bits || 0),
        calcPercent(trendAttackData?.ipsec, totalBits),
      ],
      [
        "TCP Fragment",
        bitFormatter(trendAttackData?.tcpFrag?.bits || 0),
        calcPercent(trendAttackData?.tcpFrag, totalBits),
      ],
      [
        "UDP Fragment",
        bitFormatter(trendAttackData?.udpFrag?.bits || 0),
        calcPercent(trendAttackData?.udpFrag, totalBits),
      ],
      [
        "Land Attack",
        bitFormatter(trendAttackData?.land?.bits || 0),
        calcPercent(trendAttackData?.land, totalBits),
      ],
      [
        "Normal",
        bitFormatter(trendAttackData?.unknown?.bits || 0),
        calcPercent(trendAttackData?.unknown, totalBits),
      ],
    ];
  };

  const calcPercent = (item, totalBits) => {
    const bits =
      (item?.bypass?.bits || 0) + (item?.attack?.bits || 0) || item?.bits || 0;
    return totalBits === 0 ? "0%" : `${((bits / totalBits) * 100).toFixed(2)}%`;
  };

  const handleExportPDF = async () => {
    try {
      setIsExporting(true);
      chartComponentRef.current?.resetZoom();
      await new Promise((resolve) => setTimeout(resolve, 300));

      const pdf = new jsPDF("portrait", "pt", "a4");
      const pageWidth = pdf.internal.pageSize.getWidth();
      const pageHeight = pdf.internal.pageSize.getHeight();
      const margin = 40; // Lề thống nhất
      const sectionSpacing = 50; // Khoảng cách giữa các section
      const subSectionSpacing = 20; // Khoảng cách giữa các phần con (như tiêu đề và bảng)
      let y = margin;

      // Helper function để kiểm tra và thêm trang mới nếu cần
      const addNewPageIfNeeded = (requiredHeight) => {
        if (y + requiredHeight > pageHeight - margin) {
          pdf.addPage();
          y = margin;
        }
      };

      // Header Section
      const addHeader = async () => {
        const img = new Image();
        img.crossOrigin = "anonymous";
        img.src = logoSrc;

        return new Promise((resolve, reject) => {
          img.onload = () => {
            const logoCanvas = document.createElement("canvas");
            const scale = 0.5;
            logoCanvas.width = img.width * scale;
            logoCanvas.height = img.height * scale;
            const ctx = logoCanvas.getContext("2d");
            ctx.drawImage(img, 0, 0, logoCanvas.width, logoCanvas.height);
            const logoBase64 = logoCanvas.toDataURL("image/png", 1.0);

            pdf.addImage(
              logoBase64,
              "PNG",
              pageWidth - margin - 120,
              y,
              120,
              30
            );
            pdf.setFont("helvetica", "bold");
            pdf.setFontSize(20);
            pdf.text("Network Analytics Report", margin, y + 20);
            pdf.setFontSize(12);
            pdf.setFont("helvetica", "normal");
            pdf.text(
              `Period: ${timeRange?.from || "N/A"} to ${
                timeRange?.to || "N/A"
              }`,
              margin,
              y + 40
            );
            y += 80; // Tăng khoảng cách sau header
            resolve();
          };
          img.onerror = () => reject("Failed to load logo for PDF export.");
        });
      };

      // Section Title
      const addSectionTitle = (title) => {
        addNewPageIfNeeded(40);
        pdf.setFont("helvetica", "bold");
        pdf.setFontSize(16);
        pdf.setTextColor(0, 0, 0);
        pdf.text(title, margin, y);
        y += subSectionSpacing; // Khoảng cách giữa tiêu đề và nội dung
      };

      // Traffic Summary Table
      const addTrafficSummary = () => {
        addNewPageIfNeeded(180);
        addSectionTitle("SysnetDef Traffic Summary");
        autoTable(pdf, {
          startY: y,
          margin: { left: margin, right: margin },
          head: [["Type", "Total", "Average", "Peak"]],
          body: [
            [
              "Traffic Processed",
              bitFormatter(
                trafficSumData.total.bypass.bits +
                  trafficSumData.total.attack.bits
              ),
              bitFormatter(
                trafficSumData.avg.bypass.bps + trafficSumData.avg.attack.bps
              ) + "/s",
              bitFormatter(
                trafficSumData.peak.bypass.bps + trafficSumData.peak.attack.bps
              ) + "/s",
            ],
            [
              "Received",
              bitFormatter(trafficSumData.total.bypass.bits),
              bitFormatter(trafficSumData.avg.bypass.bps) + "/s",
              bitFormatter(trafficSumData.peak.bypass.bps) + "/s",
            ],
            [
              "Dropped",
              bitFormatter(trafficSumData.total.attack.bits),
              bitFormatter(trafficSumData.avg.attack.bps) + "/s",
              bitFormatter(trafficSumData.peak.attack.bps) + "/s",
            ],
          ],
          theme: "striped",
          styles: { fontSize: 10, cellPadding: 6 }, // Tăng cellPadding
          headStyles: {
            fillColor: [66, 139, 202],
            textColor: [255, 255, 255],
            fontSize: 11,
          },
          tableWidth: "auto",
          columnStyles: "auto",
        });
        y = pdf.lastAutoTable.finalY + sectionSpacing; // Tăng khoảng cách sau bảng
      };

      // Traffic Chart
      const addTrafficChart = async () => {
        addNewPageIfNeeded(280);
        addSectionTitle("Traffic Chart");
        const chartComponent = document.querySelector(
          ".traffic-chart:nth-child(1)"
        );
        const echartsInstance =
          chartComponent?.__reactFiber$?.return?.memoizedProps?.chartRef?.current?.getEchartsInstance?.();

        if (echartsInstance && !echartsInstance.isDisposed()) {
          echartsInstance.dispatchAction({
            type: "dataZoom",
            start: 0,
            end: 100,
          });
          await new Promise((resolve) => setTimeout(resolve, 500));
        } else {
          console.warn(
            "ECharts instance is not available or has been disposed."
          );
        }

        const trafficChartCanvas = chartComponent?.querySelector("canvas");
        if (trafficChartCanvas) {
          const trafficCanvas = await html2canvas(trafficChartCanvas, {
            scale: 2,
          });
          const imgWidth = pageWidth - 2 * margin;
          const imgHeight = 220;
          const trafficBase64 = trafficCanvas.toDataURL("image/png", 0.8);
          pdf.addImage(trafficBase64, "PNG", margin, y, imgWidth, imgHeight);
          y += imgHeight + sectionSpacing;
        } else {
          console.warn("Traffic chart canvas not found.");
        }
      };
      // IP Address Tables
      const addIPTable = (title, data) => {
        const sortData = data
          .sort((a, b) => {
            if (b.bits !== a.bits) return b.bits - a.bits;
            return b.packets - a.packets;
          })
          .slice(0, 5);

        addNewPageIfNeeded(180);
        addSectionTitle(title);
        autoTable(pdf, {
          startY: y,
          margin: { left: margin, right: margin },
          head: [["IP Address", "Bits", "Packets"]],
          body: sortData.map((ip) => [
            ip.ip,
            bitFormatter(ip.bits),
            cntFormatter(ip.packets),
            ip.country,
          ]),
          theme: "striped",
          styles: { fontSize: 10, cellPadding: 6 }, // Tăng cellPadding
          headStyles: {
            fillColor: [66, 139, 202],
            textColor: [255, 255, 255],
            fontSize: 11,
          },
          tableWidth: "auto",
          columnStyles: "auto",
          didParseCell: (data) => {
            const ip = data.row.raw[0];
            if (ipColors[ip]) {
              data.cell.styles.fillColor = ipColors[ip];
            }
          },
        });
        y = pdf.lastAutoTable.finalY + sectionSpacing;
      };

      // Trends Tables
      const addTrendsTables = () => {
        addNewPageIfNeeded(250);
        addSectionTitle("Traffic Trends");

        pdf.setFont("helvetica", "normal");
        pdf.setFontSize(12);
        pdf.text("Protocol Type", margin, y);
        pdf.text("Attack Type", margin + 300, y);

        autoTable(pdf, {
          startY: y + subSectionSpacing,
          margin: { left: margin },
          tableWidth: 240,
          head: [["Name", "Bits", "Percent"]],
          body: formatProtocolTrend(),
          theme: "striped",
          styles: { fontSize: 10, cellPadding: 6 },
          headStyles: {
            fillColor: [66, 139, 202],
            textColor: [255, 255, 255],
            fontSize: 11,
          },
        });

        autoTable(pdf, {
          startY: y + subSectionSpacing,
          margin: { left: margin + 300 }, // Tăng khoảng cách giữa hai bảng
          tableWidth: 240,
          head: [["Name", "Bits", "Percent"]],
          body: formatAttackTrend(),
          theme: "striped",
          styles: { fontSize: 10, cellPadding: 6 }, // Tăng cellPadding
          headStyles: {
            fillColor: [66, 139, 202],
            textColor: [255, 255, 255],
            fontSize: 11,
          },
        });

        y =
          Math.max(
            pdf.lastAutoTable.finalY || y,
            pdf.lastAutoTable.finalY || y
          ) + sectionSpacing;
      };

      // Footer
      const addFooter = () => {
        pdf.setFontSize(8);
        pdf.setFont("helvetica", "normal");
        pdf.setTextColor(100, 100, 100);
        pdf.text(
          `Generated on ${new Date().toLocaleString()}`,
          margin,
          pageHeight - 20
        );
        pdf.text(
          `Page ${pdf.internal.getCurrentPageInfo().pageNumber}`,
          pageWidth - margin - 30,
          pageHeight - 20
        );
      };

      await addHeader();
      addTrafficSummary();
      await addTrafficChart();
      addIPTable("Top Source IP Addresses", sumSrcTableData);
      addIPTable("Top Destination IP Addresses", sumDstTableData);
      addTrendsTables();
      addFooter();

      // Save PDF
      const fromDate = timeRange?.from?.slice(0, 10) || "from";
      const toDate = timeRange?.to?.slice(0, 10) || "to";
      pdf.save(`Network_Analytics_${fromDate}_${toDate}.pdf`);
      message.success("Export successful!");
    } catch (error) {
      console.error("PDF export error:", error);
      message.error("Export failed!");
    } finally {
      setIsExporting(false);
    }
  };

  const handleTableChange = (pagination, filters, sorter) => {
    setSortField(sorter.field || "bits");
    setSortOrder(sorter.order || "descend");
  };

  const getRowClassName = (record) => {
    const sanitizedClass = sanitizeForCSS(record.ip); // Use sanitized class for IPv6
    return `ip-row-${sanitizedClass}`;
  };

  return (
    <>
      <style>
        {Object.keys(ipColors)
          .map((ip) => {
            const sanitizedClass = sanitizeForCSS(ip); // Sanitize for CSS in style too
            return `
    .ip-row-${sanitizedClass} {
      background-color: ${ipColors[ip]} !important;
      opacity: 0.9 !important;
    }
    .ip-row-${sanitizedClass}:hover {
      background-color: ${ipColors[ip]} !important;
      opacity: 1 !important;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15) !important;
    }
    .ant-table-tbody > tr.ip-row-${sanitizedClass} {
      background-color: ${ipColors[ip]} !important;
      opacity: 0.9 !important;
    }
    .ant-table-tbody > tr.ip-row-${sanitizedClass}:hover {
      background-color: ${ipColors[ip]} !important;
      opacity: 1 !important;
      box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15) !important;
    }
    .ant-table-tbody > tr.ip-row-${sanitizedClass} td {
      color: #000 !important;
      font-weight: 600 !important;
      text-shadow: 0 0 1px rgba(255, 255, 255, 0.5) !important;
    }
    .ant-table-tbody > tr > td,
    .ant-table-thead > tr > th {
      padding: 10px 16px !important;
      border: 1px solid rgba(0, 0, 0, 0.1) !important;
    }
    .ant-table-thead > tr > th {
      background-color: #f5f5f5 !important;
      color: #000 !important;
      font-weight: 700 !important;
      border-bottom: 2px solid #d9d9d9 !important;
    }
  `;
          })
          .join("\n")}
      </style>

      <div
        className="analyze-overlay"
        style={{ display: isOverlayVisible ? "block" : "none" }}
      >
        <Form
          className="range-picker"
          onFinish={HandleTimeSelect}
          form={form}
          initialValues={{
            timeRange: [
              dayjs("2025-04-05 17:03:26"),
              dayjs("2025-04-12 17:12:59"),
              // dayjs(now).subtract(1, "weeks"),
              // dayjs(now),
            ],
          }}
        >
          <Title level={3} style={{ color: "#fff" }}>
            Select the time range for traffic analysis
          </Title>
          <Form.Item name="timeRange">
            <RangePicker
              style={{ backgroundColor: "#fff" }}
              showTime
              size="large"
              format={dateFormat}
              defaultValue={[dayjs(now).subtract(1, "week"), dayjs(now)]}
              disabledDate={disabledDate}
              disabledTime={disabledRangeTime}
            />
          </Form.Item>
          <Space>
            {timeRange.from && timeRange.to && (
              <Button
                type="default"
                size="middle"
                onClick={() => setIsOverlayVisible(false)}
                variant="solid"
                color="red"
              >
                Cancel
              </Button>
            )}

            <Button
              type="primary"
              htmlType="submit"
              icon={<SearchOutlined />}
              size="middle"
              iconPosition="end"
              loading={isAnalyzeLoading}
            >
              Analyze
            </Button>
          </Space>
        </Form>
      </div>
      <Flex justify="space-between" align="center" style={{ width: "100%" }}>
        <PageTitle
          title="Network Analytics"
          description={
            <span>
              From: <strong>{timeRange.from}</strong> To:{" "}
              <strong>{timeRange.to}</strong>
            </span>
          }
        />
        <Flex style={{ padding: "0px 5px" }} gap={8}>
          <Button
            variant="dashed"
            color="green"
            icon={<ClockCircleOutlined />}
            onClick={() => setIsOverlayVisible(true)}
          >
            Change Time
          </Button>
          <Button
            type="primary"
            icon={<ExportOutlined />}
            loading={isExporting}
            onClick={handleExportPDF}
          >
            Export Result
          </Button>
        </Flex>
      </Flex>
      <div
        style={{
          padding: "0 16px",
          maxWidth: "100%",
          overflowX: "auto",
        }}
      >
        <Flex vertical gap={16}>
          <Row gutter={[16, 8]} className="analyze">
            <Col span={24}>
              <Flex vertical gap={16}>
                {/* SysnetDef Traffic */}
                <Card
                  loading={isAnalyzeLoading}
                  title="SysnetDef Traffic"
                  size="small"
                >
                  <div className="traffic-chart" style={{ minHeight: "250px" }}>
                    <Flex
                      style={{
                        flex: 1,
                        height: "100%",
                        padding: "8px 10px 0px 10px",
                      }}
                      vertical
                    >
                      <p style={{ margin: "0 0 4px 0" }}>Traffic processed</p>
                      <p style={{ margin: "0 0 8px 0" }}>
                        <span className="bigger-highlight-word">
                          {bitFormatter(
                            trafficSumData.total.bypass.bits +
                              trafficSumData.total.attack.bits
                          )}
                        </span>{" "}
                        in{" "}
                        <span
                          className="highlight-word"
                          style={{ color: "#00c864" }}
                        >
                          {getTimeDifference()}
                        </span>
                      </p>
                      <p style={{ margin: "0 0 4px 0" }}>Received</p>
                      <Flex
                        justify="space-between"
                        style={{ width: "100%", margin: "0 0 8px 0" }}
                      >
                        <span className="highlight-word">
                          {bitFormatter(trafficSumData.total.bypass.bits)}
                        </span>
                        <span className="additional-info-area">
                          <p>
                            Avg{" "}
                            <span>
                              {bitFormatter(trafficSumData.avg.bypass.bps)}/s
                            </span>
                          </p>
                          <p>
                            Peak{" "}
                            <span>
                              {bitFormatter(trafficSumData.peak.bypass.bps)}/s
                            </span>
                          </p>
                        </span>
                      </Flex>
                      <p style={{ margin: "0 0 4px 0" }}>Dropped</p>
                      <Flex
                        justify="space-between"
                        style={{ width: "100%", margin: "0 0 8px 0" }}
                      >
                        <span className="highlight-word">
                          {bitFormatter(trafficSumData.total.attack.bits)}
                        </span>
                        <span className="additional-info-area">
                          <p>
                            Avg{" "}
                            <span>
                              {bitFormatter(trafficSumData.avg.attack.bps)}/s
                            </span>
                          </p>
                          <p>
                            Peak{" "}
                            <span>
                              {bitFormatter(trafficSumData.peak.attack.bps)}/s
                            </span>
                          </p>
                        </span>
                      </Flex>
                    </Flex>
                    <TrafficChart ref={chartComponentRef} data={trafficData} />
                  </div>
                </Card>
              </Flex>
            </Col>
          </Row>

          <Row gutter={[16, 8]} className="analyze">
            <Col span={17}>
              <Flex vertical gap={16}>
                {/* Top Source IP Addresses */}
                <Card
                  loading={isAnalyzeLoading}
                  title={
                    <span style={{ padding: "8px 0", display: "inline-block" }}>
                      Top Source IP Addresses
                    </span>
                  }
                  size="small"
                >
                  <div className="traffic-chart">
                    <TopChart data={topSrcChartData} ipColors={ipColors} />
                    <div
                      style={{ flex: 1, height: "100%", padding: "0px 7px" }}
                    >
                      <Table
                        style={{
                          width: "100%",
                          marginTop: "5px",
                        }}
                        columns={address_sum_columns}
                        dataSource={sumSrcTableData}
                        pagination={false}
                        size="small"
                        onChange={handleTableChange}
                        rowClassName={getRowClassName}
                        tableLayout="fixed"
                        scroll={{ y: 200 }}
                      />
                    </div>
                  </div>
                </Card>
                {/* Top Destination IP Addresses */}
                <Card
                  loading={isAnalyzeLoading}
                  title={
                    <span style={{ padding: "8px 0", display: "inline-block" }}>
                      Top Destination IP Addresses
                    </span>
                  }
                  size="small"
                >
                  <div className="traffic-chart">
                    <TopChart data={topDstChartData} ipColors={ipColors} />
                    <div
                      style={{ flex: 1, height: "100%", padding: "0px 7px" }}
                    >
                      <Table
                        style={{ width: "100%", marginTop: "5px" }}
                        columns={address_sum_columns}
                        dataSource={sumDstTableData}
                        pagination={false}
                        size="small"
                        rowClassName={getRowClassName}
                        tableLayout="fixed"
                        scroll={{ y: 200 }}
                      />
                    </div>
                  </div>
                </Card>
              </Flex>
            </Col>
            <Col span={7}>
              <Flex vertical gap={16}>
                <Card
                  loading={isAnalyzeLoading}
                  title="Traffic Trends"
                  size="small"
                >
                  <Flex
                    vertical
                    gap={80}
                    style={{ width: "100%", height: "100%" }}
                  >
                    <div className="trend-chart">
                      <TrendChart option="protocol" data={trendProtocolData} />
                    </div>
                    <div className="trend-chart">
                      <TrendChart option="attack" data={trendAttackData} />
                    </div>
                  </Flex>
                </Card>
              </Flex>
            </Col>
          </Row>
        </Flex>
      </div>
    </>
  );
}
