import React, { useState, useEffect } from "react";
import PropTypes from "prop-types";
import { Space, Button, Table, Alert, Input, Checkbox, Tag } from "antd";
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
      align: "left",
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
            style={{
              width: 188,
              marginBottom: 8,
              display: "block",
              borderRadius: "6px",
              borderColor: "#d9d9d9",
              boxShadow: "0 2px 4px rgba(0,0,0,0.05)",
            }}
          />
          <Space>
            <Button
              type="primary"
              onClick={() => {
                confirm();
                setSearchText(selectedKeys[0] || "");
              }}
              icon={<SearchOutlined />}
              size="middle"
              style={{
                width: 90,
                borderRadius: "6px",
                backgroundColor: "#1890ff",
                borderColor: "#1890ff",
                color: "#fff",
                boxShadow: "0 2px 8px rgba(24, 144, 255, 0.2)",
                transition: "all 0.3s ease",
              }}
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
              size="middle"
              style={{
                width: 90,
                borderRadius: "6px",
                backgroundColor: "#f5f5f5",
                borderColor: "#d9d9d9",
                color: "#666",
                transition: "all 0.3s ease",
              }}
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
      render: (text) => (
        <div
          style={{
            textAlign: "left",
            color: "#52c41a",
            fontWeight: "bold",
          }}
        >
          {text}
        </div>
      ),
    },
    ...(showPortColumn
      ? [
          {
            title: "Port",
            dataIndex: "Port",
            key: "Port",
            align: "center",
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
            render: (value) => (
              <div
                style={{
                  color: "#fa8c16",
                  fontWeight: "bold",
                }}
              >
                {value || "-"}
              </div>
            ),
          },
        ]
      : []),
    {
      title: "Version",
      dataIndex: "AddressVersion",
      key: "Version",
      align: "center",
      filters: [
        { text: "IPv4", value: "IPv4" },
        { text: "IPv6", value: "IPv6" },
      ],
      onFilter: (value, record) =>
        record.AddressVersion && record.AddressVersion.indexOf(value) === 0,
      render: (text) => (
        <span
          style={{
            borderRadius: "12px",
            fontWeight: "bold",
            padding: "4px 8px",
            color: text === "IPv4" ? "#1890ff" : "#722ed1",
          }}
        >
          {text}
        </span>
      ),
    },
    {
      title: "Date Added",
      dataIndex: "AddressAddedDate",
      key: "AddedDate",
      align: "center",
      render: (text) => (
        <div
          style={{
            color: "#595959",
            fontSize: "14px",
          }}
        >
          {text}
        </div>
      ),
    },
    // Ẩn cột Config nếu hideConfigColumn = true
    ...(!hideConfigColumn
      ? [
          {
            title: "Config",
            key: "action",
            align: "center",
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
        bordered
        size="middle"
        rowClassName={(record, index) =>
          index % 2 === 0 ? "table-row-even" : "table-row-odd"
        }
        tableLayout="fixed"
        components={{
          header: {
            cell: (props) => {
              const align = props.column?.align || "left";
              return (
                <th
                  {...props}
                  style={{
                    ...props.style,
                    backgroundColor: "#f0f2f5",
                    color: "#262626",
                    fontWeight: "bold",
                    borderBottom: "2px solid #d9d9d9",
                    padding: "16px 12px",
                    borderRight: "1px solid #e8e8e8",
                    borderTop: "1px solid #e8e8e8",
                  }}
                >
                  <div style={{ textAlign: align, width: "100%" }}>
                    {props.children}
                  </div>
                </th>
              );
            },
          },
          body: {
            cell: (props) => {
              const align = props.column?.align || "left";
              return (
                <td
                  {...props}
                  style={{
                    ...props.style,
                    padding: "12px",
                    borderRight: "1px solid #f0f0f0",
                    borderBottom: "1px solid #f0f0f0",
                    backgroundColor:
                      props.index % 2 === 0 ? "#fafafa" : "#ffffff",
                  }}
                >
                  <div style={{ textAlign: align, width: "100%" }}>
                    {props.children}
                  </div>
                </td>
              );
            },
          },
        }}
      />
    </>
  );
};

export default AddressGet;
