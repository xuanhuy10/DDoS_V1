import React, { useState } from "react";
import { Card, Table, Popover, Checkbox } from "antd";
import { SettingOutlined } from "@ant-design/icons";
import { bitFormatter, cntFormatter } from "@/lib/formatter";

export default function FlowTable({ pkt, attack_info }) {
  const [visible, setVisible] = useState({
    sourceIP: true,
    destinationIP: true,
    sourcePort: true,
    destinationPort: true,
    protocol: true,
    packetCount: true,
    bitCount: true,
  });

  const flowCols = [
    {
      title: "Source IP",
      dataIndex: "srcIP",
      key: "sourceIP",
      hidden: !visible.sourceIP,
    },
    {
      title: "Destination IP",
      dataIndex: "dstIP",
      key: "destinationIP",
      hidden: !visible.destinationIP,
    },
    {
      title: "Source Port",
      dataIndex: "srcPort",
      key: "sourcePort",
      hidden: !visible.sourcePort,
    },
    {
      title: "Destination Port",
      dataIndex: "dstPort",
      key: "destinationPort",
      hidden: !visible.destinationPort,
    },
    {
      title: "Protocol",
      dataIndex: "protocol",
      key: "protocol",
      hidden: !visible.protocol,
    },
    {
      title: "Packet per Second",
      dataIndex: "packetCount",
      key: "packetCount",
      hidden: !visible.packetCount,
      render: (text) => cntFormatter(text),
    },
    {
      title: "Throughput",
      dataIndex: "packetSize",
      key: "bitCount",
      hidden: !visible.bitCount,
      render: (text) => bitFormatter(text),
    },
  ];

  const handleVisibleChange = (checked, key) => {
    setVisible((prevVisible) => ({
      ...prevVisible,
      [key]: checked,
    }));
  };

  return (
    <Card
      size="small"
      title={attack_info.title + " flow's metric"}
      extra={
        <Popover
          content={
            <>
              {flowCols.map((col) => (
                <div key={col.key}>
                  <Checkbox
                    checked={visible[col.key]}
                    onChange={(e) =>
                      handleVisibleChange(e.target.checked, col.key)
                    }
                  >
                    {col.title}
                  </Checkbox>
                </div>
              ))}
            </>
          }
          placement="bottomRight"
          trigger="click"
        >
          <SettingOutlined />
        </Popover>
      }
    >
      <Table
        size="small"
        pagination={false}
        columns={flowCols.filter((col) => !col.hidden)}
        dataSource={Array.isArray(pkt) ? pkt : []}
        rowKey={(record) => record.id}
      />
    </Card>
  );
}
