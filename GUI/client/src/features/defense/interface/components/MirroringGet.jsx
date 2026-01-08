import React, { useState, useEffect, useRef } from "react";
import {
  Table,
  Alert,
  Button,
  Modal,
  Space,
  message,
  Tag,
  Tooltip,
} from "antd";
import {
  FormOutlined,
  DeleteOutlined,
  SettingOutlined,
} from "@ant-design/icons";

import MirroringUpdate from "./MirroringUpdate";
import {
  getMirroringDeviceInterfaces,
  deleteMirroringDeviceInterface,
} from "@/features/api/DeviceInterfaces";

export default function MirrorIntefaces({ refresh }) {
  const [isLoading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [mirrorInterface, setMirrorInterface] = useState([]);
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [updatingInterfaceData, setUpdatingInterfaceData] = useState(null);

  // Modal state for delete
  const [deleteModalVisible, setDeleteModalVisible] = useState(false);
  const [deleteCountdownTime, setDeleteCountdownTime] = useState(0);
  const [deleteStep, setDeleteStep] = useState("confirm"); // 'confirm' | 'countdown'
  const [deleteId, setDeleteId] = useState(null);
  const [deleteRecord, setDeleteRecord] = useState(null); // Store record for calculation

  // Refs to manage timers / flags across renders
  const deleteIntervalRef = useRef(null);
  const deleteCountdownRef = useRef(0); // mirrors deleteCountdownTime but usable inside interval
  const backendEstimateRef = useRef(null); // null = haven't got response yet, number = estimated seconds from BE
  const isCancelledRef = useRef(false);
  const isDeletingRef = useRef(false);
  const deleteIdRef = useRef(null);

  // helper: parse response into the same shape as fetchMirroringDeviceInterfaces
  const parseData = (response) => {
    return (response.data || []).map((item) => {
      let valueObj = {};
      try {
        valueObj = item.Value ? JSON.parse(item.Value) : {};
      } catch {
        valueObj = {};
      }
      return { ...item, ...valueObj };
    });
  };

  // Hàm xóa interface với modal xác nhận và đếm ngược
  const handleDelete = (record) => {
    setDeleteRecord(record);
    setDeleteId(record.MirrorInterfaceId);
    deleteIdRef.current = record.MirrorInterfaceId;
    setDeleteStep("confirm");
    setDeleteModalVisible(true);
  };

  const clearDeleteInterval = () => {
    if (deleteIntervalRef.current) {
      clearInterval(deleteIntervalRef.current);
      deleteIntervalRef.current = null;
    }
  };

  const startCountdownInterval = (initialSeconds = 5) => {
    // initialize both state and ref
    deleteCountdownRef.current = initialSeconds;
    setDeleteCountdownTime(initialSeconds);

    // ensure no double interval
    clearDeleteInterval();

    deleteIntervalRef.current = setInterval(async () => {
      if (isCancelledRef.current) {
        clearDeleteInterval();
        return;
      }

      // decrement
      deleteCountdownRef.current = deleteCountdownRef.current - 1;
      // show to UI
      setDeleteCountdownTime(deleteCountdownRef.current);

      if (deleteCountdownRef.current <= 0) {
        // Case A: backend hasn't responded yet -> just extend by 5 and continue
        if (backendEstimateRef.current === null) {
          deleteCountdownRef.current = 5;
          setDeleteCountdownTime(5);
          return;
        }

        // Case B: backend already returned an estimate -> when reaching 0, double-check whether item is actually removed
        try {
          const response = await getMirroringDeviceInterfaces();
          const data = parseData(response);
          const stillExists = data.some(
            (i) => i.MirrorInterfaceId === deleteIdRef.current
          );

          if (stillExists) {
            // not done yet -> extend 5s and continue
            deleteCountdownRef.current = 5;
            setDeleteCountdownTime(5);
          } else {
            // done -> cleanup and update list (we already fetched list)
            clearDeleteInterval();
            isDeletingRef.current = false;
            backendEstimateRef.current = null;
            setDeleteModalVisible(false);
            message.success("Deleted successfully!");
            setMirrorInterface(data);
          }
        } catch (err) {
          // fetching failed - be conservative and extend
          deleteCountdownRef.current = 5;
          setDeleteCountdownTime(5);
        }
      }
    }, 1000);
  };

  const confirmDelete = async () => {
    setDeleteStep("countdown");
    setDeleteModalVisible(true);

    let mirrorType = [];
    try {
      mirrorType =
        typeof deleteRecord.MirrorType === "string"
          ? JSON.parse(deleteRecord.MirrorType)
          : deleteRecord.MirrorType;
    } catch {
      mirrorType = [];
    }
    const baseCommands = 2;
    const numTypes = Array.isArray(mirrorType) ? mirrorType.length : 0;
    const totalSeconds = (baseCommands + numTypes) * 2;

    // reset refs
    backendEstimateRef.current = null;
    isCancelledRef.current = false;
    isDeletingRef.current = true;

    // Bắt đầu countdown ngay lập tức
    startCountdownInterval(totalSeconds);

    // Gọi API delete bất đồng bộ
    try {
      const res = await deleteMirroringDeviceInterface(deleteIdRef.current);

      if (isCancelledRef.current) {
        isDeletingRef.current = false;
        backendEstimateRef.current = null;
        clearDeleteInterval();
        return;
      }

      const timeFromBE = res?.estimatedTime;

      // Nếu backend trả về thời gian ước tính > 0, cập nhật countdown
      if (typeof timeFromBE === "number" && timeFromBE > 0) {
        backendEstimateRef.current = timeFromBE;
        deleteCountdownRef.current = timeFromBE;
        setDeleteCountdownTime(timeFromBE);
      } else {
        // Backend không trả về thời gian hoặc đã hoàn tất, kiểm tra danh sách
        try {
          const response = await getMirroringDeviceInterfaces();
          const data = parseData(response);
          const stillExists = data.some(
            (i) => i.MirrorInterfaceId === deleteIdRef.current
          );

          if (!stillExists) {
            clearDeleteInterval();
            isDeletingRef.current = false;
            backendEstimateRef.current = null;
            setDeleteModalVisible(false);
            message.success("Deleted successfully!");
            setMirrorInterface(data);
          } else {
            // Chưa xóa xong, tiếp tục countdown
            backendEstimateRef.current = 0;
            deleteCountdownRef.current = 5;
            setDeleteCountdownTime(5);
          }
        } catch (err) {
          // Kiểm tra thất bại, tiếp tục countdown
          backendEstimateRef.current = 0;
          deleteCountdownRef.current = 5;
          setDeleteCountdownTime(5);
        }
      }
    } catch (err) {
      // API delete thất bại, dừng countdown và hiển thị lỗi
      if (!isCancelledRef.current) {
        isDeletingRef.current = false;
        backendEstimateRef.current = null;
        clearDeleteInterval();
        setDeleteModalVisible(false);
        setDeleteStep("confirm");
        setDeleteId(null);
        message.error("Delete failed!");
      }
    }
  };

  const cancelDeleteModal = () => {
    // mark as cancelled so in-flight responses will be ignored
    isCancelledRef.current = true;
    isDeletingRef.current = false;
    backendEstimateRef.current = null;
    deleteIdRef.current = null;

    clearDeleteInterval();
    setDeleteModalVisible(false);
    setDeleteStep("confirm");
    setDeleteId(null);
    setDeleteCountdownTime(0);
    setDeleteRecord(null);
  };

  const onCanCel = () => {
    setIsModalOpen(false);
    setUpdatingInterfaceData(null);
  };

  const fetchMirroringDeviceInterfaces = async () => {
    try {
      setError(null);
      setLoading(true);
      const response = await getMirroringDeviceInterfaces();
      const data = parseData(response);
      setMirrorInterface(data);
    } catch (error) {
      setError("Failed to fetch data, please try again later.");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchMirroringDeviceInterfaces();
    // eslint-disable-next-line
  }, [refresh]);

  // cleanup on unmount
  useEffect(() => {
    return () => {
      clearDeleteInterval();
    };
  }, []);

  const mirroring_cols = [
    {
      title: "Monitoring Interface",
      dataIndex: "MonitorInterfaceName",
      key: "MonitorInterfaceName",
      align: "center",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#52c41a",
            fontWeight: "bold",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "Mirroring Interface",
      dataIndex: "MirrorInterfaceName",
      key: "MirroringInterfaceName",
      align: "center",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#52c41a",
            fontWeight: "bold",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "Traffic Mirroring Direction",
      dataIndex: "MirrorSetting",
      key: "MirroringDirection",
      align: "center",
      render: (text) => (
        <div
          style={{
            textAlign: "center",
            color: "#fa8c16",
            fontWeight: "bold",
          }}
        >
          {text}
        </div>
      ),
    },
    {
      title: "Field",
      dataIndex: "MirrorType",
      key: "MirrorType",
      align: "center",
      render: (types, record) => {
        let mirrorType = [];
        try {
          mirrorType = typeof types === "string" ? JSON.parse(types) : types;
        } catch {
          mirrorType = [];
        }
        if (!Array.isArray(mirrorType)) mirrorType = [];
        let obj = {};
        try {
          if (typeof record.Value === "string" && record.Value.trim() !== "") {
            obj = JSON.parse(record.Value);
          }
        } catch {
          obj = {};
        }
        const fieldMap = {
          "Dest Mac": ["DestMac"],
          "Source Mac": ["SourceMac"],
          "Dest Port": ["DestPort"],
          "Source Port": ["SourcePort"],
          "Dest IP": ["DestIPv4", "DestIPv6"],
          "Source IP": ["SourceIPv4", "SourceIPv6"],
          Protocol: ["Protocol"],
        };

        let fields = [];
        mirrorType.forEach((type) => {
          if (Array.isArray(fieldMap[type])) {
            fields = fields.concat(fieldMap[type]);
          } else if (fieldMap[type]) {
            fields.push(fieldMap[type]);
          }
        });

        const filledFields = fields.filter(
          (key) =>
            obj[key] !== undefined && obj[key] !== null && obj[key] !== ""
        );

        return (
          <div
            style={{
              textAlign: "center",
              display: "flex",
              gap: "6px",
              flexDirection: "column",
              alignItems: "center",
            }}
          >
            {filledFields.map((key) => (
              <Tag
                key={key}
                color="blue"
                style={{
                  padding: "4px 8px",
                  borderRadius: "9px",
                  fontSize: "12px",
                }}
              >
                {key}
              </Tag>
            ))}
          </div>
        );
      },
    },
    {
      title: "Value",
      dataIndex: "Value",
      key: "Value",
      align: "center",
      render: (value, record) => {
        let obj = {};
        try {
          if (typeof value === "string" && value.trim() !== "") {
            obj = JSON.parse(value);
          }
        } catch {
          obj = {};
        }
        let mirrorType = record.MirrorType;
        if (typeof mirrorType === "string") {
          try {
            mirrorType = JSON.parse(mirrorType);
          } catch {
            mirrorType = [];
          }
        }
        mirrorType = Array.isArray(mirrorType) ? mirrorType : [];
        const fieldMap = {
          "Dest Mac": ["DestMac"],
          "Source Mac": ["SourceMac"],
          "Dest Port": ["DestPort"],
          "Source Port": ["SourcePort"],
          "Dest IP": ["DestIPv4", "DestIPv6"],
          "Source IP": ["SourceIPv4", "SourceIPv6"],
          Protocol: ["Protocol"],
        };
        let fields = [];
        mirrorType.forEach((type) => {
          if (Array.isArray(fieldMap[type])) {
            fields = fields.concat(fieldMap[type]);
          }
        });
        return (
          <div
            style={{
              textAlign: "center",
              display: "flex",
              gap: "6px",
              flexDirection: "column",
              alignItems: "center",
            }}
          >
            {fields
              .filter(
                (key) =>
                  obj[key] !== undefined && obj[key] !== null && obj[key] !== ""
              )
              .map((key) => (
                <Tag
                  key={key}
                  color="purple"
                  style={{
                    padding: "4px 8px",
                    borderRadius: "9px",
                    fontSize: "12px",
                  }}
                >
                  {obj[key]}
                </Tag>
              ))}
          </div>
        );
      },
    },
    {
      title: "Option",
      key: "action",
      align: "center",
      render: (_, record) => (
        <Space style={{ justifyContent: "center" }}>
          <Tooltip title="Edit Mirroring">
            <Button
              icon={<FormOutlined />}
              type="primary"
              onClick={() => {
                setUpdatingInterfaceData(record);
                setIsModalOpen(true);
              }}
              style={{
                borderRadius: "6px",
                backgroundColor: "#1890ff",
                borderColor: "#1890ff",
                transition: "all 0.3s ease",
                boxShadow: "0 2px 8px rgba(24, 144, 255, 0.2)",
              }}
            />
          </Tooltip>
          <Tooltip title="Delete Mirroring">
            <Button
              icon={<DeleteOutlined />}
              danger
              onClick={() => handleDelete(record)}
              style={{
                borderRadius: "6px",
                backgroundColor: "#ff4d4f",
                borderColor: "#ff4d4f",
                color: "#fff",
                transition: "all 0.3s ease",
                boxShadow: "0 2px 8px rgba(255, 77, 79, 0.2)",
              }}
            />
          </Tooltip>
        </Space>
      ),
    },
  ];

  return (
    <>
      {error && (
        <Alert
          type="error"
          message={error}
          showIcon
          style={{ marginBottom: 16 }}
          action={
            <Button
              size="small"
              type="primary"
              onClick={() => fetchMirroringDeviceInterfaces()}
            >
              Retry
            </Button>
          }
        />
      )}
      <MirroringUpdate
        isModalOpen={isModalOpen}
        onCancel={onCanCel}
        mirrorInterfaceData={updatingInterfaceData}
        onUpdated={fetchMirroringDeviceInterfaces}
      />
      <Table
        columns={mirroring_cols}
        dataSource={mirrorInterface}
        loading={isLoading}
        pagination={false}
        rowKey={(record) => record.MirrorInterfaceId}
        bordered
        size="middle"
        rowClassName={(record, index) =>
          index % 2 === 0 ? "table-row-even" : "table-row-odd"
        }
        tableLayout="fixed"
        components={{
          header: {
            cell: (props) => (
              <th
                {...props}
                style={{
                  ...props.style,
                  backgroundColor: "#f0f2f5",
                  color: "#262626",
                  fontWeight: "bold",
                  borderBottom: "2px solid #d9d9d9",
                  padding: "16px 12px",
                  textAlign: "center",
                  borderRight: "1px solid #e8e8e8",
                  borderTop: "1px solid #e8e8e8",
                }}
              />
            ),
          },
          body: {
            cell: (props) => (
              <td
                {...props}
                style={{
                  ...props.style,
                  padding: "12px",
                  textAlign: "center",
                  borderRight: "1px solid #f0f0f0",
                  borderBottom: "1px solid #f0f0f0",
                  backgroundColor:
                    props.index % 2 === 0 ? "#fafafa" : "#ffffff",
                }}
              />
            ),
          },
        }}
      />
      {/* Modal xác nhận và đếm ngược xóa */}
      <Modal
        open={deleteModalVisible}
        footer={null}
        closable={false}
        centered
        maskClosable={false}
        width={400}
      >
        {deleteStep === "confirm" ? (
          <div style={{ textAlign: "center", padding: 20 }}>
            <p style={{ fontSize: 18, marginBottom: 16 }}>
              Are you sure you want to delete this mirroring interface?
            </p>
            <Space>
              <Button danger onClick={cancelDeleteModal}>
                Cancel
              </Button>
              <Button type="primary" onClick={confirmDelete}>
                Yes, Delete
              </Button>
            </Space>
          </div>
        ) : (
          <div style={{ textAlign: "center", padding: 20 }}>
            <SettingOutlined style={{ fontSize: 48, color: "#ff4d4f" }} spin />
            <p style={{ marginTop: 16, fontSize: 18 }}>
              Removing configuration...
            </p>
            <p style={{ fontSize: 16, color: "#888" }}>
              Estimated time: {deleteCountdownTime}s
            </p>
          </div>
        )}
      </Modal>
    </>
  );
}
