import React, { useEffect, useState } from "react";
import { ExportOutlined, DeleteOutlined } from "@ant-design/icons";
import {
  Button,
  Modal,
  Table,
  Space,
  Flex,
  Alert,
  Tag,
  message,
  DatePicker,
  Form,
} from "antd";
import * as XLSX from "xlsx";
import { saveAs } from "file-saver";

import {
  getDeviceLogByLogType,
  deleteDeviceLogByLogTypeAndIds,
  deleteDeviceLogByTypeAndTimeRange,
} from "@/features/api/DeviceLogs";

const ActivityList = ({ type, refreshKey }) => {
  const [selectedRowKeys, setSelectedRowKeys] = useState([]);
  const [error, setError] = useState(null);
  const [loading, setLoading] = useState(false);
  const [dataSource, setDataSource] = useState([]);
  const [modalVisible, setModalVisible] = useState(false);
  const [modalAction, setModalAction] = useState("export"); // 'export' or 'delete'
  const [form] = Form.useForm();

  const fetchLogs = async () => {
    setLoading(true);
    try {
      const response = await getDeviceLogByLogType(type);
      setDataSource(response.data);
    } catch (error) {
      setError("Failed to fetch logs. Please try again later.");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchLogs();
  }, [type, refreshKey]);

  // Export logs to Excel (FE)
  const exportToExcel = (
    logs,
    fileName = `${type.toLowerCase()}_logs.xlsx`
  ) => {
    if (!logs || logs.length === 0) {
      message.error("No logs to export!");
      return;
    }

    // Prepare data for Excel
    const sheetData = logs.map((log) => ({
      Time: log.LogActionTime,
      Action: log.LogActionContent,
      Username: log.Username,
      Status: log.LogActionResult,
      Result: log.LogActionResultDetail,
      Detail: log.LogActionDetail,
    }));

    // Create workbook and worksheet
    const wb = XLSX.utils.book_new();
    const ws = XLSX.utils.json_to_sheet(sheetData, {
      header: ["Time", "Action", "Username", "Status", "Result", "Detail"],
    });
    XLSX.utils.book_append_sheet(wb, ws, "Activity Logs");

    // Generate and save file
    const excelBuffer = XLSX.write(wb, { bookType: "xlsx", type: "array" });
    const data = new Blob([excelBuffer], { type: "application/octet-stream" });
    saveAs(data, fileName);

    message.success("Export successful!");
  };

  // Handle export button click
  const handleDownload = async () => {
    if (selectedRowKeys.length > 0) {
      // Export các logs đã chọn
      const selectedLogs = dataSource.filter((log) =>
        selectedRowKeys.includes(log.LogId)
      );
      exportToExcel(selectedLogs, `${type.toLowerCase()}_selected_logs.xlsx`);
    } else {
      setModalAction("export");
      setModalVisible(true);
    }
  };

  // Handle delete button click
  const handleDelete = () => {
    if (selectedRowKeys.length > 0) {
      Modal.confirm({
        title: "Are you sure you want to delete the selected logs?",
        okText: "Yes",
        okType: "danger",
        cancelText: "No",
        onOk: async () => {
          try {
            await deleteDeviceLogByLogTypeAndIds(type, selectedRowKeys);
            message.success("Logs deleted successfully");
            fetchLogs();
            setSelectedRowKeys([]);
          } catch (error) {
            setError("Failed to delete logs. Please try again later.");
            message.error("Failed to delete logs. Please try again later.");
          }
        },
      });
    } else {
      setModalAction("delete");
      setModalVisible(true);
    }
  };

  // Handle modal confirmation for time range export/delete
  const handleModalOk = async () => {
    try {
      const values = await form.validateFields();
      const from = values.range[0];
      const to = values.range[1];
      if (modalAction === "export") {
        // Lọc logs theo timerange trên FE
        const logsInRange = dataSource.filter((log) => {
          const logTime = new Date(log.LogActionTime.replace(/-/g, "/"));
          return logTime >= from.toDate() && logTime <= to.toDate();
        });
        exportToExcel(
          logsInRange,
          `${type.toLowerCase()}_logs_${from.format(
            "YYYYMMDD_HHmmss"
          )}_to_${to.format("YYYYMMDD_HHmmss")}.xlsx`
        );
      } else {
        // Xóa logs theo timerange (gọi BE)
        const fromStr = from.format("YYYY/MM/DD HH:mm:ss");
        const toStr = to.format("YYYY/MM/DD HH:mm:ss");
        await deleteDeviceLogByTypeAndTimeRange(type, fromStr, toStr);
        message.success("Logs deleted successfully");
        fetchLogs();
      }
      setModalVisible(false);
      form.resetFields();
    } catch (error) {
      message.error("Please select a valid date range.");
    }
  };

  const columns = [
    Table.SELECTION_COLUMN,
    {
      key: "Time",
      title: "Time",
      dataIndex: "LogActionTime",
      sorter: (a, b) =>
        new Date(a.LogActionTime.replace(/-/g, "/")) -
        new Date(b.LogActionTime.replace(/-/g, "/")),
      sortDirections: ["descend", "ascend"],
      width: "13%",
    },
    {
      key: "title",
      title: "Action",
      dataIndex: "LogActionContent",
      width: "13%",
    },
    Table.EXPAND_COLUMN,
    {
      key: "UName",
      title: "Username",
      dataIndex: "Username",
      width: "13%",
    },
    {
      key: "Result",
      title: "Status",
      dataIndex: "LogActionResult",
      width: "13%",
      render: (text) => (
        <Tag color={text === "Success" ? "green" : "red"}>{text}</Tag>
      ),
      filters: [
        { text: "Success", value: "Success" },
        { text: "Failed", value: "Failed" },
      ],
      onFilter: (value, record) => record.LogActionResult === value,
    },
    {
      key: "Reason",
      title: "Result",
      dataIndex: "LogActionResultDetail",
      width: "40%",
    },
  ];

  const rowSelection = {
    selectedRowKeys,
    onChange: setSelectedRowKeys,
  };

  return (
    <Flex vertical gap={15}>
      <Space>
        <Button
          type="primary"
          onClick={handleDownload}
          icon={<ExportOutlined />}
        >
          Export Activity
        </Button>
        <Button danger onClick={handleDelete} icon={<DeleteOutlined />}>
          Delete
        </Button>
      </Space>
      {error && <Alert message={error} type="error" showIcon />}
      <Table
        loading={loading}
        columns={columns}
        rowSelection={rowSelection}
        expandable={{
          expandedRowRender: (record) => (
            <span
              style={{
                margin: 0,
                // width: "100%",
                wordBreak: "break-word",
                whiteSpace: "pre-wrap",
                overflow: "hidden",
                textOverflow: "ellipsis",
              }}
            >
              {record.LogActionDetail}
            </span>
          ),
          rowExpandable: (record) => record.LogActionDetail !== "",
        }}
        dataSource={dataSource}
        rowKey={(record) => record.LogId}
      />
      <Modal
        title={
          modalAction === "export"
            ? "Export Activity logs by time range"
            : "Delete Activity logs by time range"
        }
        open={modalVisible}
        onOk={handleModalOk}
        onCancel={() => setModalVisible(false)}
        okText={modalAction === "export" ? "Export" : "Delete"}
        okButtonProps={{ danger: modalAction === "delete" }}
      >
        <Form form={form} layout="vertical">
          <Form.Item
            label="Select time range"
            name="range"
            rules={[{ required: true, message: "Please select a date range" }]}
          >
            <DatePicker.RangePicker
              showTime={{ format: "HH:mm:ss" }}
              format="YYYY/MM/DD HH:mm:ss"
              allowClear={false}
              style={{ width: "100%" }}
              placeholder={["Start date & time", "End date & time"]}
            />
          </Form.Item>
        </Form>
      </Modal>
    </Flex>
  );
};

export default ActivityList;
