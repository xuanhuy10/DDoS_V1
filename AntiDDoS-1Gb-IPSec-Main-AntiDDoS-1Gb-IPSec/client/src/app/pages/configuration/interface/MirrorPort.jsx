import React, { useState } from "react";
import { Button, Typography, Modal } from "antd";
import { PlusOutlined } from "@ant-design/icons";

import InterfaceLayout from "@/features/defense/interface/components/InterfaceLayout";
import MirrorIntefaces from "@/features/defense/interface/components/MirroringGet";
import MirroringAdd from "@/features/defense/interface/components/MirroringAdd";

const { Title, Paragraph } = Typography;

export default function MirrorPort() {
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [refresh, setRefresh] = useState(false); // ⚡ trigger reload

  return (
    <InterfaceLayout selectedKey="mirroring">
      <Title level={3}>Port Mirroring</Title>
      <Paragraph>
        Monitor network traffic by duplicating packets to a designated port for
        analysis and diagnostics.
      </Paragraph>
      <Button
        icon={<PlusOutlined />}
        type="primary"
        onClick={() => setIsModalOpen(true)}
        style={{ marginBottom: 16 }}
      >
        Add
      </Button>

      <MirrorIntefaces refresh={refresh} />

      <Modal
        title="Add Port Mirroring"
        open={isModalOpen}
        onCancel={() => setIsModalOpen(false)}
        footer={null}
        width={700}
        destroyOnClose
      >
        <MirroringAdd
          onSaved={() => {
            setIsModalOpen(false);
            setRefresh((prev) => !prev); // 🔁 trigger reload
          }}
          onCancel={() => setIsModalOpen(false)} // <-- Thêm dòng này
        />
      </Modal>
    </InterfaceLayout>
  );
}
