import { ExportOutlined } from "@ant-design/icons";
import { Button, Modal, Table, Space, Flex, Alert, message, Radio } from "antd";
import React, { useEffect, useState } from "react";
import { HiOutlineTrash } from "react-icons/hi";

import {
  getTrafficLogByLogType,
  deleteLogTrafficbyLogTypeAndLogName,
  exportLogTrafficbyLogTypeAndLogName,
} from "@/features/api/DeviceLogs";

import { byteFormatter } from "@/lib/formatter";

const LogList = ({ type, refreshKey }) => {
  const [selectedRowKeys, setSelectedRowKeys] = useState([]);
  const [error, setError] = useState(null);
  const [loading, setLoading] = useState(false);
  const [dataSource, setDataSource] = useState([]);

  // Modal export format
  const [formatModalVisible, setFormatModalVisible] = useState(false);
  const [exportFormat, setExportFormat] = useState("xlsx");
  const [warningModalVisible, setWarningModalVisible] = useState(false);

  const getLatestFile = () => {
    if (!Array.isArray(dataSource) || dataSource.length === 0) return null;
    return dataSource.reduce((latest, file) => {
      return parseDate(file.lastModified) > parseDate(latest.lastModified)
        ? file
        : latest;
    }, dataSource[0]);
  };

  // Xác nhận xóa -> nếu có file mới nhất thì cảnh báo, không thì xóa luôn
  const handleDelete = () => {
    Modal.confirm({
      title: "Are you sure you want to delete the selected logs?",
      okText: "Yes",
      okType: "danger",
      cancelText: "No",
      onOk: () => {
        const latestFile = getLatestFile();
        const isLatestSelected =
          latestFile && selectedRowKeys.includes(latestFile.filename);
        if (isLatestSelected) {
          setWarningModalVisible(true);
        } else {
          doDeleteLogs();
        }
      },
    });
  };

  // Xác nhận cảnh báo sẽ xóa thật sự
  const doDeleteLogs = async () => {
    try {
      await deleteLogTrafficbyLogTypeAndLogName(type, selectedRowKeys);
      message.success("Logs deleted successfully");
    } catch (error) {
      console.log(error);
      setError("Failed to delete logs. Please try again later.");
    } finally {
      setSelectedRowKeys([]);
      fetchLogs();
    }
  };

  // Khi bấm Export Logs, mở modal chọn định dạng
  const handleDownload = () => {
    setFormatModalVisible(true);
  };

  // Hàm thực hiện export theo định dạng
  const handleExportWithFormat = async () => {
    try {
      if (
        (exportFormat === "txt" || exportFormat === "xlsx") &&
        selectedRowKeys.length > 1
      ) {
        // Tải từng file riêng biệt
        for (const file of selectedRowKeys) {
          await exportLogTrafficbyLogTypeAndLogName(type, [file], exportFormat);
        }
      } else {
        // ZIP các file lại
        await exportLogTrafficbyLogTypeAndLogName(
          type,
          selectedRowKeys,
          exportFormat
        );
      }
      setFormatModalVisible(false);
      setSelectedRowKeys([]);
    } catch (error) {
      setError("Failed to download logs. Please try again later.");
    }
  };

  const fetchLogs = async () => {
    setLoading(true);
    try {
      const response = await getTrafficLogByLogType(type);
      setDataSource(Array.isArray(response.data) ? response.data : []);
    } catch (error) {
      setError("Failed to fetch logs. Please try again later.");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchLogs();
  }, [type, refreshKey]);

  const parseDate = (dateStr) => {
    if (!dateStr) return new Date(0);
    const [datePart, timePart = "00:00:00"] = dateStr.split(" ");
    const [day, month, year] = datePart.split("/").map(Number);
    const fullYear = year < 100 ? 2000 + year : year;
    return new Date(
      fullYear,
      month - 1,
      day,
      ...timePart.split(":").map(Number)
    );
  };

  const columns = [
    Table.SELECTION_COLUMN,
    {
      key: "title",
      title: "Logs",
      dataIndex: "filename",
      width: "27%",
    },
    {
      key: "from",
      title: "Capture from",
      dataIndex: "createdAt",
      sorter: (a, b) => parseDate(a.createdAt) - parseDate(b.createdAt),
      sortDirections: ["descend", "ascend"],
      width: "25%",
    },
    {
      key: "to",
      title: "To",
      dataIndex: "lastModified",
      sorter: (a, b) => parseDate(a.lastModified) - parseDate(b.lastModified),
      sortDirections: ["descend", "ascend"],
      width: "25%",
    },
    {
      key: "size",
      title: "Size",
      dataIndex: "size",
      sorter: (a, b) => Number(a.size) - Number(b.size),
      sortDirections: ["descend", "ascend"],
      render: (text) => byteFormatter(text),
      width: "20%",
    },
  ];

  const rowSelection = {
    selectedRowKeys,
    onChange: (selectedRowKeys) => setSelectedRowKeys(selectedRowKeys),
  };

  const hasSelected = selectedRowKeys.length > 0;

  return (
    <Flex vertical gap={15}>
      <Space>
        <Button
          type="primary"
          onClick={handleDownload}
          disabled={!hasSelected}
          icon={<ExportOutlined />}
        >
          Export Logs
        </Button>
        <Button
          danger
          onClick={handleDelete}
          disabled={!hasSelected}
          icon={<HiOutlineTrash />}
        >
          Delete
        </Button>
      </Space>

      {error && <Alert message={error} type="error" showIcon />}
      <Table
        showSorterTooltip={false}
        loading={loading}
        rowSelection={rowSelection}
        bordered
        columns={columns}
        dataSource={Array.isArray(dataSource) ? dataSource : []}
        rowKey={(record) => record.filename}
      />

      {/* Modal chọn định dạng export */}
      <Modal
        title="Export Selected Logs"
        open={formatModalVisible}
        onOk={handleExportWithFormat}
        onCancel={() => setFormatModalVisible(false)}
        okText="Export"
      >
        <div>
          <b>Selected files:</b>
          <ul
            style={{
              maxHeight: 120,
              overflow: "auto",
              margin: 0,
              paddingLeft: 18,
            }}
          >
            {(Array.isArray(dataSource) ? dataSource : [])
              .filter((log) => selectedRowKeys.includes(log.filename))
              .map((log) => (
                <li key={log.filename}>{log.filename}</li>
              ))}
          </ul>
        </div>
        <div style={{ marginTop: 16 }}>
          <Radio.Group
            value={exportFormat}
            onChange={(e) => setExportFormat(e.target.value)}
            style={{ marginTop: 8 }}
          >
            <Radio value="xlsx">Excel</Radio>
            <Radio value="txt">Text</Radio>
            <Radio value="zip-excel">ZIP by Excel</Radio>
            <Radio value="zip-txt">ZIP by Text</Radio>
          </Radio.Group>
        </div>
      </Modal>

      <Modal
        open={warningModalVisible}
        onOk={() => {
          setWarningModalVisible(false);
          doDeleteLogs();
        }}
        onCancel={() => setWarningModalVisible(false)}
        okText="Delete"
        okType="danger"
        cancelText="Cancel"
      >
        <b>Warning:</b> You are about to delete the active log file that is
        currently being written to. Are you sure you want to proceed?
      </Modal>
    </Flex>
  );
};

export default LogList;
