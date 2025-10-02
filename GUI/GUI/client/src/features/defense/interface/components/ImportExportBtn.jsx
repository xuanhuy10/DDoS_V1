import React, { useState, useEffect } from "react";
import { Modal, Button, Tabs, message, Upload, Checkbox, Row, Col } from "antd";
import {
  ExportOutlined,
  ImportOutlined,
  InboxOutlined,
  SettingOutlined,
} from "@ant-design/icons";
import * as XLSX from "xlsx";
import { saveAs } from "file-saver";
import API from "@/utils/api_v2";
import ipaddr from "ipaddr.js";

const { Dragger } = Upload;
const ipv4Regex =
  /^(25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d)){3}$/;

// Normalize IP function for consistent IP comparison
function normalizeIp(address) {
  try {
    const addr = ipaddr.parse(address);
    if (addr.kind() === "ipv6" && addr.isIPv4MappedAddress()) {
      return addr.toIPv4Address().toString();
    }
    return addr.toString();
  } catch {
    return address.trim().toLowerCase();
  }
}

const ImportExportBtn = ({
  title,
  visible,
  setVisible,
  onImportIPs,
  exportIPs = [],
  type,
  currentIPs = [],
  maxIpLimit,
  interfaceOptions = [],
  isImportLoading,
}) => {
  const [fileList, setFileList] = useState([]);
  const [pendingIPs, setPendingIPs] = useState([]);
  const [selectedPorts, setSelectedPorts] = useState([]);
  const [loading, setLoading] = useState(false);
  const [estimatedTime, setEstimatedTime] = useState(0);
  const [countdown, setCountdown] = useState(null);

  // Timer effect to handle countdown
  useEffect(() => {
    let countdownInterval;
    if (countdown > 0) {
      countdownInterval = setInterval(() => {
        setCountdown((prev) => {
          if (prev <= 1) {
            clearInterval(countdownInterval);
            return 0;
          }
          return prev - 1;
        });
      }, 1000);
    }
    return () => clearInterval(countdownInterval);
  }, [countdown]);

  // Xử lý khi chọn file
  const handleBeforeUpload = (file) => {
    if (fileList.length > 0) {
      message.error("Only one file can be imported at a time!");
      return false;
    }
    setFileList([file]);
    const reader = new FileReader();
    reader.onload = async (e) => {
      const data = new Uint8Array(e.target.result);
      const workbook = XLSX.read(data, { type: "array" });
      const allRows = [];
      workbook.SheetNames.forEach((sheetName) => {
        const sheet = workbook.Sheets[sheetName];
        const rows = XLSX.utils.sheet_to_json(sheet, { header: 1 });
        rows.forEach((row, idx) => {
          if (idx === 0 && (row[0] || "").toLowerCase().includes("address"))
            return;
          if (row[0]) {
            allRows.push({
              Address: row[0],
              AddressVersion: ipv4Regex.test(row[0]) ? "IPv4" : "IPv6",
            });
          }
        });
      });
      setPendingIPs(allRows);
    };
    reader.readAsArrayBuffer(file);
    return false;
  };

  // Xác nhận import
  const handleImport = async () => {
    if (!pendingIPs.length) {
      message.error("No IP addresses to import!");
      return;
    }
    if (
      Array.isArray(currentIPs) &&
      maxIpLimit &&
      currentIPs.length + pendingIPs.length > maxIpLimit
    ) {
      message.error(
        `Importing will exceed the maximum of ${maxIpLimit} IP addresses!`
      );
      return;
    }
    if (Array.isArray(interfaceOptions) && interfaceOptions.length > 0) {
      if (!selectedPorts || selectedPorts.length === 0) {
        message.error("Please select at least one interface!");
        return;
      }
    }
    setLoading(true);
    // Start timer when "Apply" is clicked
    let totalProcessingTime = 10; // Default: 10 seconds for import processing
    setEstimatedTime(totalProcessingTime);
    setCountdown(totalProcessingTime);

    try {
      let importData = [];
      let duplicateCount = 0;
      if (Array.isArray(interfaceOptions) && interfaceOptions.length > 0) {
        pendingIPs.forEach((ip) => {
          selectedPorts.forEach((iface) => {
            // Check for duplicates
            const isDuplicate = currentIPs.some(
              (existingIp) =>
                normalizeIp(existingIp.Address) === normalizeIp(ip.Address) &&
                existingIp.Port === iface
            );
            if (!isDuplicate) {
              importData.push({
                ...ip,
                InterfaceName: iface,
                Port: iface,
              });
            } else {
              duplicateCount++;
            }
          });
        });
      } else {
        importData = pendingIPs.filter(
          (ip) =>
            !currentIPs.some(
              (existingIp) =>
                normalizeIp(existingIp.Address) === normalizeIp(ip.Address)
            )
        );
        duplicateCount = pendingIPs.length - importData.length;
      }

      if (importData.length === 0) {
        message.info(
          "No new IP addresses to import after filtering duplicates!"
        );
        setVisible(false);
        setFileList([]);
        setPendingIPs([]);
        setSelectedPorts([]);
        setEstimatedTime(0);
        setCountdown(null);
        setLoading(false);
        return;
      }

      // Adjust timer based on the number of IPs to import
      totalProcessingTime = 10;
      setEstimatedTime(totalProcessingTime);
      setCountdown(totalProcessingTime);

      let importApi = "defense/address/common/white/import";
      if (type === "http_black") {
        importApi = "defense/address/http/black/import";
      } else if (type === "vpn_white") {
        importApi = "defense/address/vpn/white/import";
      } else if (type === "blocked") {
        importApi = "defense/address/blocked/import";
      }
      await API.post(importApi, importData);
      message.success(
        `Imported ${importData.length} new IP address(es) successfully!${
          duplicateCount > 0
            ? ` ${duplicateCount} duplicate IP(s) were skipped.`
            : ""
        }`
      );
      setVisible(false);
      setFileList([]);
      setPendingIPs([]);
      setSelectedPorts([]);
      setEstimatedTime(0);
      setCountdown(null);
      onImportIPs(() => API.post(importApi, importData));
    } catch (error) {
      message.error(error.response?.data?.message || "Import failed!");
      setEstimatedTime(0);
      setCountdown(null);
    } finally {
      setLoading(false);
    }
  };

  // Cancel
  const handleCancel = () => {
    setVisible(false);
    setFileList([]);
    setPendingIPs([]);
    setSelectedPorts([]);
    setEstimatedTime(0);
    setCountdown(null);
  };

  // Export giữ nguyên
  const handleExportExcel = () => {
    if (!exportIPs || exportIPs.length === 0) {
      message.error("No IP addresses to export!");
      return;
    }
    const ipv4 = exportIPs.filter((ip) => ipv4Regex.test(ip.Address));
    const ipv6 = exportIPs.filter((ip) => !ipv4Regex.test(ip.Address));
    const wb = XLSX.utils.book_new();

    const showPort = type === "protected" || type === "blocked";
    if (ipv4.length) {
      const ipv4Sheet = XLSX.utils.aoa_to_sheet([
        showPort ? ["IPv4 Address", "Port"] : ["IPv4 Address"],
        ...ipv4.map((ip) =>
          showPort ? [ip.Address, ip.Port || ""] : [ip.Address]
        ),
      ]);
      XLSX.utils.book_append_sheet(wb, ipv4Sheet, "IPv4");
    }
    if (ipv6.length) {
      const ipv6Sheet = XLSX.utils.aoa_to_sheet([
        showPort ? ["IPv6 Address", "Port"] : ["IPv6 Address"],
        ...ipv6.map((ip) =>
          showPort ? [ip.Address, ip.Port || ""] : [ip.Address]
        ),
      ]);
      XLSX.utils.book_append_sheet(wb, ipv6Sheet, "IPv6");
    }
    const excelBuffer = XLSX.write(wb, { bookType: "xlsx", type: "array" });
    const data = new Blob([excelBuffer], { type: "application/octet-stream" });
    const fileName = `${type || "exported"}_ips.xlsx`;
    saveAs(data, fileName);

    message.success("Export successful!");
    setVisible(false);
  };

  // Giao diện import
  const ImportBox = () => (
    <div style={{ textAlign: "center", padding: 24 }}>
      <Dragger
        fileList={fileList}
        beforeUpload={handleBeforeUpload}
        showUploadList={false}
        accept=".xlsx,.csv"
        style={{ marginBottom: 16 }}
        disabled={fileList.length > 0 || isImportLoading || loading}
      >
        {!fileList.length ? (
          <>
            <p className="ant-upload-drag-icon">
              <InboxOutlined style={{ fontSize: 48 }} />
            </p>
            <p className="ant-upload-text">
              Click or drag an Excel/CSV file to import
            </p>
            <p className="ant-upload-hint">
              Only one file can be imported at a time
            </p>
          </>
        ) : (
          <div style={{ padding: 24 }}>
            <div style={{ fontSize: 16, marginBottom: 8 }}>
              {fileList[0].name}
            </div>
          </div>
        )}
      </Dragger>
      {fileList.length > 0 && (
        <>
          {Array.isArray(interfaceOptions) && interfaceOptions.length > 0 ? (
            <div style={{ margin: "16px 0 0 0", textAlign: "left" }}>
              <div style={{ marginBottom: 8, fontWeight: 500 }}>
                Choose interfaces to apply:
              </div>
              <Checkbox.Group
                value={selectedPorts}
                onChange={setSelectedPorts}
                style={{ width: "100%", marginTop: 12 }}
                disabled={isImportLoading || loading}
              >
                <Row gutter={[16, 16]}>
                  {interfaceOptions.map((item) => (
                    <Col
                      span={6}
                      key={item.value}
                      style={{ textAlign: "left" }}
                    >
                      <Checkbox value={item.value}>{item.label}</Checkbox>
                    </Col>
                  ))}
                </Row>
              </Checkbox.Group>
              {estimatedTime > 0 && (
                <div
                  style={{
                    display: "flex",
                    alignItems: "center",
                    justifyContent: "center",
                    marginTop: 16,
                    marginBottom: 16,
                  }}
                >
                  <SettingOutlined
                    spin={isImportLoading || countdown > 0}
                    style={{ fontSize: 20, color: "#1890ff", marginRight: 8 }}
                  />
                  <p style={{ color: "#888", margin: 0 }}>
                    Estimated Time: {estimatedTime}s
                    {countdown > 0 && <span> (Remaining: {countdown}s)</span>}
                  </p>
                </div>
              )}
              <div style={{ marginTop: 16, textAlign: "center" }}>
                <Button
                  onClick={handleCancel}
                  style={{ width: 80 }}
                  disabled={isImportLoading || loading}
                >
                  Cancel
                </Button>
                <Button
                  type="primary"
                  onClick={handleImport}
                  loading={isImportLoading || loading}
                  disabled={
                    !selectedPorts ||
                    selectedPorts.length === 0 ||
                    isImportLoading ||
                    loading
                  }
                  style={{ width: 120, marginLeft: 8 }}
                >
                  Apply
                </Button>
              </div>
            </div>
          ) : (
            <div style={{ marginTop: 16, textAlign: "center" }}>
              <Button
                onClick={handleCancel}
                style={{ width: 80 }}
                disabled={isImportLoading || loading}
              >
                Cancel
              </Button>
              <Button
                type="primary"
                onClick={handleImport}
                loading={isImportLoading || loading}
                disabled={pendingIPs.length === 0 || isImportLoading || loading}
                style={{ width: 120, marginLeft: 8 }}
              >
                Apply
              </Button>
            </div>
          )}
        </>
      )}
    </div>
  );

  return (
    <Modal title={title} open={visible} onCancel={handleCancel} footer={null}>
      <Tabs
        defaultActiveKey="1"
        centered
        size="large"
        items={[
          {
            key: "1",
            label: `Import`,
            children: <ImportBox />,
            icon: <ImportOutlined />,
          },
          {
            key: "2",
            label: `Export`,
            icon: <ExportOutlined />,
            children: (
              <div
                style={{
                  display: "flex",
                  justifyContent: "center",
                  alignItems: "center",
                  paddingTop: "20px",
                }}
              >
                <Button
                  type="primary"
                  size="large"
                  style={{
                    width: "50%",
                    alignContent: "center",
                    margin: "20px",
                    padding: "10px",
                  }}
                  onClick={handleExportExcel}
                >
                  Export to Excel
                </Button>
              </div>
            ),
          },
        ]}
      />
    </Modal>
  );
};

export default ImportExportBtn;
