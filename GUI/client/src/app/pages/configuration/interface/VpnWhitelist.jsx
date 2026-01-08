import React, { useEffect, useState } from "react";
import { Button, Typography, Space, message, Modal, Upload } from "antd";
import {
  PlusOutlined,
  DeleteOutlined,
  InboxOutlined,
  SettingOutlined,
} from "@ant-design/icons";
import { RiFileExcel2Line } from "react-icons/ri";
import * as XLSX from "xlsx";

const { Title, Paragraph } = Typography;

import InterfaceLayout from "@/features/defense/interface/components/InterfaceLayout";
import AddressGet from "@/features/defense/interface/components/AddressGet";
import AddressInsert from "@/features/defense/interface/components/AddressInsert";
import ImportExportBtn from "@/features/defense/interface/components/ImportExportBtn";
import {
  getAllVPNAllowedAddresses,
  insertVPNAllowedAddresses,
  importDeleteBulkVPNAllowedAddresses,
  deleteNetworkAddressesByAddressAndVersionList,
} from "@/features/api/NetworkAddresses";
import ipaddr from "ipaddr.js";

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

export default function VPNWhitelist() {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [refreshKey, setRefreshKey] = useState(0);
  const [isModalVisible, setIsModalVisible] = useState(false);
  const [multiDeleteMode] = useState(true);
  const [ipList, setIpList] = useState([]);
  const [selectedRowKeys, setSelectedRowKeys] = useState([]);
  const [isImportDeleteModalVisible, setIsImportDeleteModalVisible] =
    useState(false);
  const [isImportLoading, setIsImportLoading] = useState(false);
  const [importDeleteFileList, setImportDeleteFileList] = useState([]);
  const [importDeletePendingIPs, setImportDeletePendingIPs] = useState([]);
  const [importDeleteSelectedIfs, setImportDeleteSelectedIfs] = useState([]);
  const [estimatedTime, setEstimatedTime] = useState(0);
  const [countdown, setCountdown] = useState(null);
  const [isDeleteModalVisible, setIsDeleteModalVisible] = useState(false); // New modal for Delete Selected
  const [deleteEstimatedTime, setDeleteEstimatedTime] = useState(0); // Timer for Delete Selected
  const [deleteCountdown, setDeleteCountdown] = useState(null); // Timer for Delete Selected
  const MAX_IP_LIMIT = 128;

  useEffect(() => {
    async function fetchData() {
      const res = await getAllVPNAllowedAddresses();
      setIpList(res.data || []);
    }
    fetchData();
  }, [refreshKey]);

  const createCountdownInterval = (
    setCountdownFn,
    setEstimatedTimeFn,
    isLoading
  ) => {
    return setInterval(() => {
      setCountdownFn((prev) => {
        if (prev <= 1) {
          if (isLoading) {
            setEstimatedTimeFn((prevTime) => prevTime + 5);
            return 5;
          }
          // Clear only if not loading
          return 0;
        }
        return prev - 1;
      });
    }, 1000);
  };

  // Thêm IP mới (thủ công) với timer
  const handleCreate = async (newIp) => {
    try {
      const normalizedNewIp = normalizeIp(newIp.Address || newIp.address);
      const isDuplicate = ipList.some(
        (ip) =>
          normalizeIp(ip.Address || ip.address) === normalizedNewIp &&
          (ip.Port || "") === (newIp.Port || "")
      );
      if (isDuplicate) {
        message.error("IP address with the same interface already exists!");
        return;
      }
      if (ipList.length >= MAX_IP_LIMIT) {
        message.error(
          `The maximum number of IP addresses allowed is ${MAX_IP_LIMIT}!`
        );
        return;
      }

      setIsImportLoading(true); // Use isImportLoading for consistency
      const totalProcessingTime = 2; // 2 seconds per IP
      setEstimatedTime(totalProcessingTime);
      setCountdown(totalProcessingTime);

      const countdownInterval = createCountdownInterval(
        setCountdown,
        setEstimatedTime,
        () => isImportLoading
      );

      await insertVPNAllowedAddresses({
        Address: newIp.Address,
        AddressVersion: newIp.AddressVersion,
        InterfaceId: newIp.InterfaceId || 1,
        Port: newIp.Port,
      });
      message.success("Successfully added new IP address!");
      const res = await getAllVPNAllowedAddresses();
      if (res && res.data) setIpList(res.data);
      setIsModalOpen(false);
      setRefreshKey(refreshKey + 1);
      clearInterval(countdownInterval);
      setEstimatedTime(0);
      setCountdown(null);
    } catch (error) {
      console.error(error);
      message.error("Failed to add new IP address!");
      clearInterval(countdownInterval);
      setEstimatedTime(0);
      setCountdown(null);
    } finally {
      setIsImportLoading(false);
    }
  };

  // Import bằng file (bulk) với timer
  const handleImportIPs = async () => {
    setIsImportLoading(true);
    const totalProcessingTime = 10; // 10 seconds per file
    setEstimatedTime(totalProcessingTime);
    setCountdown(totalProcessingTime);

    const countdownInterval = createCountdownInterval(
      setCountdown,
      setEstimatedTime,
      () => isImportLoading
    );

    try {
      const res = await getAllVPNAllowedAddresses();
      if (res && res.data) setIpList(res.data);
      setIsModalVisible(false);
      message.success("Successfully imported IP addresses!");
      clearInterval(countdownInterval);
      setEstimatedTime(0);
      setCountdown(null);
    } catch (error) {
      message.error("Failed to refresh IP addresses!");
      clearInterval(countdownInterval);
      setEstimatedTime(0);
      setCountdown(null);
    } finally {
      setIsImportLoading(false);
    }
  };

  const handleCancel = () => {
    setIsModalOpen(false);
    setRefreshKey(refreshKey + 1);
    setEstimatedTime(0);
    setCountdown(null);
  };

  // Xóa theo select (theo ID, từng lệnh) với timer
  const handleDeleteSelected = async () => {
    if (selectedRowKeys.length === 0) {
      message.warning("Please select at least one IP address to delete.");
      return;
    }
    setIsDeleteModalVisible(true); // Show delete confirmation modal
    const totalProcessingTime = selectedRowKeys.length * 2; // 2 seconds per IP
    setDeleteEstimatedTime(totalProcessingTime);
    setDeleteCountdown(totalProcessingTime);
  };

  // Handle Delete Confirmation
  const handleDeleteConfirm = async () => {
    if (!selectedRowKeys.length) {
      message.warning("No IP addresses selected for deletion.");
      setIsDeleteModalVisible(false);
      setIsImportLoading(false);
      return;
    }

    setIsImportLoading(true);
    const countdownInterval = createCountdownInterval(
      setDeleteCountdown,
      setDeleteEstimatedTime,
      () => isImportLoading
    );

    try {
      // Validate selectedRowKeys against ipList
      const validIds = ipList
        .filter((item) => selectedRowKeys.includes(item.AddressId))
        .map((item) => item.AddressId);

      if (!validIds.length) {
        message.error(
          "No valid IPs to delete. Selected IDs do not match any IP addresses."
        );
        setIsDeleteModalVisible(false);
        setDeleteEstimatedTime(0);
        setDeleteCountdown(null);
        setIsImportLoading(false);
        clearInterval(countdownInterval);
        return;
      }

      // Alternative: Prepare objects with Address and AddressVersion if API requires it
      const toDeleteObjects = ipList
        .filter((item) => selectedRowKeys.includes(item.AddressId))
        .map((item) => ({
          Address: item.Address,
          AddressVersion: item.AddressVersion,
        }));

      // Remove duplicates based on Address + AddressVersion
      const uniqueToDelete = Array.from(
        new Map(
          toDeleteObjects.map((ip) => [ip.Address + ip.AddressVersion, ip])
        ).values()
      );

      // Call API to delete IPs (use validIds or uniqueToDelete based on API requirements)
      await deleteNetworkAddressesByAddressAndVersionList(uniqueToDelete); // or validIds

      // Update ipList by removing deleted IPs
      setIpList((prev) =>
        prev.filter((item) => !selectedRowKeys.includes(item.AddressId))
      );

      // Show success message
      message.success(`Deleted ${uniqueToDelete.length} IP address(es)!`);

      // Reset states
      setSelectedRowKeys([]);
      setIsDeleteModalVisible(false);
      setDeleteEstimatedTime(0);
      setDeleteCountdown(null);
    } catch (error) {
      // Log detailed error for debugging
      console.error("Delete error:", error.response?.data || error.message);
      message.error(
        error.response?.data?.message ||
          "Failed to delete selected address(es)."
      );
    } finally {
      // Clean up timer and loading state
      clearInterval(countdownInterval);
      setIsImportLoading(false);
    }
  };

  // ==================== IMPORT DELETE =====================
  // Đọc file import delete
  const handleImportDeleteBeforeUpload = (file) => {
    setImportDeleteFileList([file]);
    const reader = new FileReader();
    reader.onload = (e) => {
      const data = new Uint8Array(e.target.result);
      const workbook = XLSX.read(data, { type: "array" });
      const allRows = [];
      workbook.SheetNames.forEach((sheetName) => {
        const sheet = workbook.Sheets[sheetName];
        const rows = XLSX.utils.sheet_to_json(sheet, { header: 1 });
        rows.forEach((row, idx) => {
          if (idx === 0 && (row[0] || "").toLowerCase().includes("address"))
            return;
          if (row[0]) allRows.push(row[0].toString().trim());
        });
      });
      setImportDeletePendingIPs(allRows);
      const totalProcessingTime = 10; // 10 seconds per file
      setEstimatedTime(totalProcessingTime);
      setCountdown(totalProcessingTime);
    };
    reader.readAsArrayBuffer(file);
    return false;
  };

  const handleImportDeleteApply = async () => {
    if (!importDeletePendingIPs.length) {
      message.error("No IP addresses to delete!");
      return;
    }

    setIsImportLoading(true);

    const totalProcessingTime = 10; // 10 seconds per file
    setEstimatedTime(totalProcessingTime);
    setCountdown(totalProcessingTime);

    const countdownInterval = createCountdownInterval(
      setCountdown,
      setEstimatedTime,
      () => isImportLoading
    );

    try {
      const deletePairs = [];
      importDeletePendingIPs.forEach((ip) => {
        deletePairs.push({ Address: ip });
      });

      const toDeleteObjects = ipList
        .filter((ip) =>
          deletePairs.some(
            (pair) => normalizeIp(pair.Address) === normalizeIp(ip.Address)
          )
        )
        .map((ip) => ({
          Address: ip.Address,
          AddressVersion: ip.AddressVersion,
        }));

      if (toDeleteObjects.length === 0) {
        message.info("No matching IP address found to delete.");
        clearInterval(countdownInterval);
        setCountdown(0);
        setEstimatedTime(0);
        setIsImportLoading(false);
        return;
      }

      await importDeleteBulkVPNAllowedAddresses(toDeleteObjects);
      const res = await getAllVPNAllowedAddresses();
      if (res && res.data) setIpList(res.data);
      setSelectedRowKeys([]);
      message.success(
        `Deleted ${toDeleteObjects.length} IP address(es) from file.`
      );
      setIsImportDeleteModalVisible(false);
      setImportDeleteFileList([]);
      setImportDeletePendingIPs([]);
      setImportDeleteSelectedIfs([]);
      clearInterval(countdownInterval);
      setCountdown(0);
      setEstimatedTime(0);
    } catch (err) {
      message.error("Failed to delete addresses from file!");
      clearInterval(countdownInterval);
      setCountdown(0);
      setEstimatedTime(0);
    } finally {
      setIsImportLoading(false);
    }
  };

  // Hàm Cancel
  const handleImportDeleteCancel = () => {
    setIsImportDeleteModalVisible(false);
    setImportDeleteFileList([]);
    setImportDeletePendingIPs([]);
    setImportDeleteSelectedIfs([]);
    setCountdown(null);
    setEstimatedTime(0);
  };

  // MODAL của nút Import Delete
  const ImportDeleteModal = (
    <Modal
      title="Import Delete"
      open={isImportDeleteModalVisible}
      onCancel={handleImportDeleteCancel}
      footer={null}
    >
      <div style={{ textAlign: "center", padding: 24 }}>
        <Upload.Dragger
          fileList={importDeleteFileList}
          beforeUpload={handleImportDeleteBeforeUpload}
          showUploadList={false}
          accept=".xlsx,.csv"
          style={{ marginBottom: 16 }}
          disabled={!!importDeleteFileList.length || isImportLoading}
        >
          {!importDeleteFileList.length ? (
            <>
              <p className="ant-upload-drag-icon">
                <InboxOutlined style={{ fontSize: 48 }} />
              </p>
              <p className="ant-upload-text">
                Click or drag an Excel/CSV file here to import for deletion
              </p>
            </>
          ) : (
            <div style={{ padding: 24 }}>
              <div style={{ fontSize: 16, marginBottom: 8 }}>
                {importDeleteFileList[0].name}
              </div>
            </div>
          )}
        </Upload.Dragger>
        {importDeleteFileList.length > 0 && (
          <>
            {isImportLoading && countdown > 0 && (
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
                  spin={isImportLoading}
                  style={{ fontSize: 20, color: "#1890ff", marginRight: 8 }}
                />
                <p style={{ color: "#888", margin: 0 }}>
                  <span> Estimated Time: {countdown}s</span>
                </p>
              </div>
            )}
            <div style={{ marginTop: 16, textAlign: "center" }}>
              <Button
                onClick={handleImportDeleteCancel}
                style={{ width: 80 }}
                disabled={isImportLoading}
              >
                Cancel
              </Button>
              <Button
                type="primary"
                danger
                onClick={handleImportDeleteApply}
                loading={isImportLoading}
                style={{ width: 120, marginLeft: 8 }}
              >
                Apply
              </Button>
            </div>
          </>
        )}
      </div>
    </Modal>
  );

  // Delete Confirmation Modal
  const DeleteConfirmModal = (
    <Modal
      title="Delete Selected IP Addresses"
      open={isDeleteModalVisible}
      footer={null}
      closable={false}
    >
      <div style={{ textAlign: "center", padding: 24 }}>
        <p style={{ marginBottom: 16 }}>
          Are you sure you want to delete {selectedRowKeys.length} IP
          address(es)? This action cannot be undone.
        </p>
        {isImportLoading && deleteCountdown > 0 && (
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
              spin={isImportLoading}
              style={{ fontSize: 20, color: "#1890ff", marginRight: 8 }}
            />
            <p style={{ color: "#888", margin: 0 }}>
              <span> Estimated Time: {deleteCountdown}s</span>
            </p>
          </div>
        )}

        <div style={{ textAlign: "center" }}>
          <Button
            onClick={() => {
              setIsDeleteModalVisible(false);
              setDeleteEstimatedTime(0);
              setDeleteCountdown(null);
            }}
            style={{ width: 80 }}
            disabled={isImportLoading}
          >
            Cancel
          </Button>
          <Button
            type="primary"
            danger
            onClick={handleDeleteConfirm}
            loading={isImportLoading}
            style={{ width: 120, marginLeft: 8 }}
          >
            Delete
          </Button>
        </div>
      </div>
    </Modal>
  );

  // Các hàm mở modal
  const openAddModal = () => setIsModalOpen(true);
  const openImportModal = () => setIsModalVisible(true);
  const openImportDeleteModal = () => setIsImportDeleteModalVisible(true);

  const isRowSelected = selectedRowKeys.length > 0;

  return (
    <InterfaceLayout selectedKey="vpn_white">
      <Title level={3}>VPN Server Whitelist</Title>
      <Paragraph>
        Allow IP addresses to access your network through VPN. The addresses in
        the list are allowed to connect.
      </Paragraph>
      <Space>
        <Button
          icon={<PlusOutlined />}
          type="primary"
          onClick={openAddModal}
          style={{ marginBottom: 16 }}
          disabled={isRowSelected || isImportLoading}
        >
          Add
        </Button>
        <Button
          variant="solid"
          color="green"
          icon={<RiFileExcel2Line />}
          style={{ marginBottom: 16 }}
          onClick={openImportModal}
          disabled={isRowSelected || isImportLoading}
        >
          Import & Export
        </Button>
        {isModalVisible && (
          <ImportExportBtn
            title="Import & Export"
            visible={isModalVisible}
            setVisible={(val) => {
              setIsModalVisible(val);
              if (!val) setIsImportLoading(false); // Đặt isLoading về false khi đóng modal
            }}
            onImportIPs={handleImportIPs}
            exportIPs={ipList}
            type="vpn_white"
            isImportLoading={isImportLoading}
            currentIPs={ipList}
            maxIpLimit={128}
          />
        )}
        <Button
          variant="solid"
          color="red"
          icon={<RiFileExcel2Line />}
          style={{ marginBottom: 16 }}
          onClick={openImportDeleteModal}
          disabled={isRowSelected || isImportLoading}
        >
          Import Delete
        </Button>
        <Button
          icon={<DeleteOutlined />}
          danger
          type="primary"
          onClick={handleDeleteSelected}
          style={{ marginBottom: 16 }}
          disabled={!isRowSelected || isImportLoading}
        >
          Delete Selected
        </Button>
      </Space>

      {ImportDeleteModal}
      {DeleteConfirmModal}

      <AddressGet
        type="vpn_white"
        refreshKey={refreshKey}
        importedIps={ipList}
        setImportedIps={setIpList}
        multiDeleteMode={true}
        setMultiDeleteMode={() => {}}
        selectedRowKeys={selectedRowKeys}
        setSelectedRowKeys={setSelectedRowKeys}
        hideConfigColumn={true}
      />

      <AddressInsert
        type="vpn_white"
        isModalOpen={isModalOpen}
        onCreate={handleCreate}
        onCancel={handleCancel}
        isImportLoading={isImportLoading}
        estimatedTime={estimatedTime}
        countdown={countdown}
      />
    </InterfaceLayout>
  );
}
