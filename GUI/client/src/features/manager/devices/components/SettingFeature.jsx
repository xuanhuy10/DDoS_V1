var __rest =
  (this && this.__rest) ||
  function (s, e) {
    var t = {};
    for (var p in s)
      if (Object.prototype.hasOwnProperty.call(s, p) && e.indexOf(p) < 0)
        t[p] = s[p];
    if (s != null && typeof Object.getOwnPropertySymbols === "function")
      for (var i = 0, p = Object.getOwnPropertySymbols(s); i < p.length; i++) {
        if (
          e.indexOf(p[i]) < 0 &&
          Object.prototype.propertyIsEnumerable.call(s, p[i])
        )
          t[p[i]] = s[p[i]];
      }
    return t;
  };
import React, { useState } from "react";
import { Button, Flex, Space, Switch, Table, Tag, Transfer } from "antd";
// Customize Table Transfer
const TableTransfer = (props) => {
  const { leftColumns, rightColumns } = props,
    restProps = __rest(props, ["leftColumns", "rightColumns"]);
  return (
    <Transfer style={{ width: "100%" }} {...restProps}>
      {({
        direction,
        filteredItems,
        onItemSelect,
        onItemSelectAll,
        selectedKeys: listSelectedKeys,
        disabled: listDisabled,
      }) => {
        const columns = direction === "left" ? leftColumns : rightColumns;
        const rowSelection = {
          getCheckboxProps: () => ({ disabled: listDisabled }),
          onChange(selectedRowKeys) {
            onItemSelectAll(selectedRowKeys, "replace");
          },
          selectedRowKeys: listSelectedKeys,
          selections: [
            Table.SELECTION_ALL,
            Table.SELECTION_INVERT,
            Table.SELECTION_NONE,
          ],
        };
        return (
          <Table
            rowSelection={rowSelection}
            columns={columns}
            dataSource={filteredItems}
            size="small"
            style={{ pointerEvents: listDisabled ? "none" : undefined }}
            onRow={({ key, disabled: itemDisabled }) => ({
              onClick: () => {
                if (itemDisabled || listDisabled) {
                  return;
                }
                onItemSelect(key, !listSelectedKeys.includes(key));
              },
            })}
          />
        );
      }}
    </Transfer>
  );
};
const mockTags = ["cat", "dog", "bird"];
const mockData = Array.from({ length: 20 }).map((_, i) => ({
  key: i.toString(),
  title: `content${i + 1}`,
  description: `description of content${i + 1}`,
  tag: mockTags[i % 3],
}));
const columns = [
  {
    dataIndex: "title",
    title: "Name",
  },
  {
    dataIndex: "tag",
    title: "Tag",
    render: (tag) => (
      <Tag style={{ marginInlineEnd: 0 }} color="cyan">
        {tag.toUpperCase()}
      </Tag>
    ),
  },
  {
    dataIndex: "description",
    title: "Description",
  },
];
const filterOption = (input, item) => {
  var _a, _b;
  return (
    ((_a = item.title) === null || _a === void 0
      ? void 0
      : _a.includes(input)) ||
    ((_b = item.tag) === null || _b === void 0 ? void 0 : _b.includes(input))
  );
};
const SettingFeature = () => {
  const [targetKeys, setTargetKeys] = useState([]);
  const [disabled, setDisabled] = useState(false);
  const onChange = (nextTargetKeys) => {
    setTargetKeys(nextTargetKeys);
  };
  const toggleDisabled = (checked) => {
    setDisabled(checked);
  };
  return (
    <Flex align="start" gap="middle" vertical>
      <Space direction="horizontal" gap={10}>
        <Button danger onClick={() => setTargetKeys([])}>
          Reset Settings
        </Button>
        <Switch
          unCheckedChildren="disabled"
          checkedChildren="disabled"
          checked={disabled}
          onChange={toggleDisabled}
        />
      </Space>
      <TableTransfer
        dataSource={mockData}
        targetKeys={targetKeys}
        disabled={disabled}
        showSearch
        showSelectAll={false}
        onChange={onChange}
        filterOption={filterOption}
        leftColumns={columns}
        rightColumns={columns}
        status="warning"
      />
    </Flex>
  );
};
export default SettingFeature;
