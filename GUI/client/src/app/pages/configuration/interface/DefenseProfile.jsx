import React, { useState } from "react";
import { Button, Flex, Typography } from "antd";
const { Title, Paragraph } = Typography;

import InterfaceLayout from "@/features/defense/interface/components/InterfaceLayout";
import ProfileList from "@/features/defense/interface/components/ProfileList";

export default function DefenseProfile() {
  const [isModalOpen, setIsModalOpen] = React.useState(false);
  const [refreshKey, setRefreshKey] = useState(0);

  const handleCreate = () => {
    setIsModalOpen(false);
    setRefreshKey(refreshKey + 1);
  };
  return (
    <InterfaceLayout selectedKey="profile">
      <Title level={3}>Defense Profile Setting</Title>
      <Paragraph>
        Configure and manage defense profiles to protect your network against
        potential threats. Customize settings to enhance security and mitigate
        risks effectively.
      </Paragraph>
      <ProfileList />
    </InterfaceLayout>
  );
}
