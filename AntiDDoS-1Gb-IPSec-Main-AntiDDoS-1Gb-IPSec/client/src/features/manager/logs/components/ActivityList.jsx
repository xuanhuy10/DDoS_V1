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
import { saveAs } from "file-saver";
import ExcelJS from "exceljs";
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
    // Create workbook
    const workbook = new ExcelJS.Workbook();
    // Create worksheet
    const worksheet = workbook.addWorksheet("Activity Logs");
    // Define headers
    const headers = [
      "Time",
      "Action",
      "Username",
      "Status",
      "Result",
      "Detail",
    ];

    // Calculate column widths based on max content for full visibility and spacing
    const colWidths = headers.map((header, index) => {
      let maxLength = header.length;
      // Check data for max length in this column
      logs.forEach((log) => {
        const keys = {
          time: log.LogActionTime,
          action: log.LogActionContent,
          username: log.Username,
          status: log.LogActionResult,
          result: log.LogActionResultDetail,
          detail: log.LogActionDetail,
        };
        const cellValue = keys[headers[index].toLowerCase()] || "";
        maxLength = Math.max(
          maxLength,
          cellValue ? cellValue.toString().length : 0
        );
      });
      return Math.max(20, maxLength + 15); // Always use max content + larger padding for no overflow
    });

    // Set columns with calculated widths
    worksheet.columns = headers.map((header, index) => ({
      header,
      key: header.toLowerCase(),
      width: colWidths[index],
    }));

    // Prepare and add data
    logs.forEach((log) => {
      const rowData = {
        time: log.LogActionTime,
        action: log.LogActionContent,
        username: log.Username,
        status: log.LogActionResult,
        result: log.LogActionResultDetail,
        detail: log.LogActionDetail,
      };
      worksheet.addRow(rowData);
    });

    // Style headers: bold, colors, centered, borders
    const headerRow = worksheet.getRow(1);
    headerRow.eachCell((cell, colNumber) => {
      const header = headers[colNumber - 1];
      let headerColor = "FF4F81BD"; // Default blue
      if (header === "Time") {
        headerColor = "FF4472C4"; // Darker blue for Time
      } else if (header === "Status" || header === "Result") {
        headerColor = "FF70AD47"; // Green for status/result
      }
      cell.font = { bold: true, color: { argb: "FFFFFFFF" } };
      cell.fill = {
        type: "pattern",
        pattern: "solid",
        fgColor: { argb: headerColor },
      };
      cell.alignment = { horizontal: "center", vertical: "middle" };
      cell.border = {
        top: { style: "medium", color: { argb: "FF000000" } },
        left: { style: "medium", color: { argb: "FF000000" } },
        bottom: { style: "medium", color: { argb: "FF000000" } },
        right: { style: "medium", color: { argb: "FF000000" } },
      };
    });

    // Style data rows: alternating colors, borders, left-aligned, wrap text
    worksheet.eachRow({ includeEmpty: false }, (row, rowNumber) => {
      if (rowNumber > 1) {
        // Skip header
        const rowColor = rowNumber % 2 === 0 ? "FFF9F9F9" : "FFFFFFFF"; // Light gray even rows
        row.eachCell((cell) => {
          cell.alignment = {
            horizontal: "left",
            vertical: "middle",
            wrapText: true,
          }; // Wrap for long content
          cell.fill = {
            type: "pattern",
            pattern: "solid",
            fgColor: { argb: rowColor },
          };
          cell.border = {
            top: { style: "thin", color: { argb: "FF000000" } },
            left: { style: "thin", color: { argb: "FF000000" } },
            bottom: { style: "thin", color: { argb: "FF000000" } },
            right: { style: "thin", color: { argb: "FF000000" } },
          };
        });
      }
    });

    // Freeze header row
    worksheet.views = [
      { state: "frozen", xSplit: 0, ySplit: 1, activeCell: "A2" },
    ];

    // Write to buffer and download
    workbook.xlsx
      .writeBuffer()
      .then((buffer) => {
        const blob = new Blob([buffer], { type: "application/octet-stream" });
        saveAs(blob, fileName);
        message.success("Export successful!");
      })
      .catch((err) => {
        console.error("Export error:", err);
        message.error("Export failed!");
      });
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
      width: "20%",
    },
    {
      key: "title",
      title: "Action",
      dataIndex: "LogActionContent",
      width: "20%",
    },
    Table.EXPAND_COLUMN,
    {
      key: "UName",
      title: "Username",
      dataIndex: "Username",
      width: "20%",
    },
    {
      key: "Result",
      title: "Status",
      dataIndex: "LogActionResult",
      width: "20%",
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
      width: "20%",
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
