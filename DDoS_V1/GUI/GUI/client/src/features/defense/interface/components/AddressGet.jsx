import React, { useState } from "react";
import { Space, Button, Table, Alert, Input, Checkbox } from "antd";
import { SearchOutlined } from "@ant-design/icons";

const AddressGet = ({
  importedIps = [],
  multiDeleteMode = false,
  selectedRowKeys = [],
  setSelectedRowKeys = () => {},
  hideConfigColumn = false,
  showPortColumn = false, // thêm prop này
}) => {
  const [error, setError] = useState(null);
  const [searchText, setSearchText] = useState("");
  const [pagination, setPagination] = useState({ pageSize: 5, current: 1 });
  const [loading, setLoading] = useState(false);

  const filteredData = importedIps.filter(
    (item) =>
      item.Address &&
      item.Address.toLowerCase().includes(searchText.toLowerCase())
  );

  const columns = [
    ...(multiDeleteMode
      ? [
          {
            title: (
              <Checkbox
                checked={
                  selectedRowKeys.length === filteredData.length &&
                  filteredData.length > 0
                }
                indeterminate={
                  selectedRowKeys.length > 0 &&
                  selectedRowKeys.length < filteredData.length
                }
                onChange={(e) => {
                  if (e.target.checked) {
                    setSelectedRowKeys(
                      filteredData.map((item) => item.AddressId)
                    );
                  } else {
                    setSelectedRowKeys([]);
                  }
                }}
              />
            ),
            dataIndex: "select",
            key: "select",
            render: (_, record) => (
              <Checkbox
                checked={selectedRowKeys.includes(record.AddressId)}
                onChange={(e) => {
                  if (e.target.checked) {
                    setSelectedRowKeys((prev) => [...prev, record.AddressId]);
                  } else {
                    setSelectedRowKeys((prev) =>
                      prev.filter((id) => id !== record.AddressId)
                    );
                  }
                }}
              />
            ),
            width: 48,
          },
        ]
      : []),
    {
      title: "IP Address",
      dataIndex: "Address",
      key: "IP",
      filterDropdown: ({
        setSelectedKeys,
        selectedKeys,
        confirm,
        clearFilters,
      }) => (
        <div style={{ padding: 8 }}>
          <Input
            placeholder="Search IP"
            value={selectedKeys[0] || ""}
            onChange={(e) =>
              setSelectedKeys(e.target.value ? [e.target.value] : [])
            }
            onPressEnter={() => {
              confirm();
              setSearchText(selectedKeys[0] || "");
            }}
            style={{ width: 188, marginBottom: 8, display: "block" }}
          />
          <Space>
            <Button
              type="primary"
              onClick={() => {
                confirm();
                setSearchText(selectedKeys[0] || "");
              }}
              icon={<SearchOutlined />}
              size="small"
              style={{ width: 90 }}
            >
              Search
            </Button>
            <Button
              onClick={() => {
                setSelectedKeys([]);
                clearFilters();
                setSearchText("");
                confirm();
              }}
              size="small"
              style={{ width: 90 }}
            >
              Reset
            </Button>
          </Space>
        </div>
      ),
      filterIcon: (filtered) => (
        <SearchOutlined style={{ color: filtered ? "#1890ff" : undefined }} />
      ),
      onFilter: (value, record) =>
        record.Address &&
        record.Address.toLowerCase().includes(value.toLowerCase()),
    },
    ...(showPortColumn
      ? [
          {
            title: "Port",
            dataIndex: "Port",
            key: "Port",
            filters: [
              { text: "eth1", value: "eth1" },
              { text: "eth2", value: "eth2" },
              { text: "eth3", value: "eth3" },
              { text: "eth4", value: "eth4" },
              { text: "eth5", value: "eth5" },
              { text: "eth6", value: "eth6" },
              { text: "eth7", value: "eth7" },
              { text: "eth8", value: "eth8" },
            ],
            onFilter: (value, record) => record.Port === value,
            render: (value) => value || "-",
          },
        ]
      : []),
    {
      title: "Version",
      dataIndex: "AddressVersion",
      key: "Version",
      filters: [
        { text: "IPv4", value: "IPv4" },
        { text: "IPv6", value: "IPv6" },
      ],
      onFilter: (value, record) =>
        record.AddressVersion && record.AddressVersion.indexOf(value) === 0,
    },
    {
      title: "Date Added",
      dataIndex: "AddressAddedDate",
      key: "AddedDate",
    },
    // Ẩn cột Config nếu hideConfigColumn = true
    ...(!hideConfigColumn
      ? [
          {
            title: "Config",
            key: "action",
            render: (_, record) => null,
          },
        ]
      : []),
  ];

  return (
    <>
      {error && (
        <Alert
          type="error"
          message={error}
          showIcon
          style={{ marginBottom: 16 }}
        />
      )}
      <Table
        showSorterTooltip={false}
        rowKey={(record) => record.AddressId || record.key || record.Address}
        columns={columns}
        dataSource={filteredData}
        loading={loading}
        pagination={{
          ...pagination,
          showSizeChanger: true,
          pageSizeOptions: ["5", "10", "20"],
        }}
        onChange={(pagination) => setPagination(pagination)}
      />
    </>
  );
};

export default AddressGet;
