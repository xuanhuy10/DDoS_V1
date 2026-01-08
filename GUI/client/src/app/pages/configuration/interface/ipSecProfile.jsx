import { Button, Flex, Typography, Modal, Select } from "antd";
const { Title, Paragraph } = Typography;
import { useState } from "react";

import InterfaceLayout from "@/features/defense/interface/components/InterfaceLayout";
import IpSecProfile from "@/features/defense/interface/components/IpSecProfile";

const IpSecProfileList = () => {
  const [modalVisible, setModalVisible] = useState(false);
  const [selectedProfile, setSelectedProfile] = useState(null);
  const [profiles, setProfiles] = useState([
    {
      id: 1,
      name: "Default Profile",
      ddos: null,
      ipsec: "Default IPSec configuration",
      status: true,
    },
    {
      id: 2,
      name: "Custom Profile",
      ddos: null,
      ipsec: "Custom IPSec configuration",
      status: false,
    },
  ]);

  const handleSaveAs = () => {
    setModalVisible(true);
  };

  const handleApply = () => {
    if (!selectedProfile) {
      return;
    }
    const currentConfigBackup =
      "Current interface configuration backup - saved on " +
      new Date().toISOString();

    setProfiles(
      profiles.map((profile) =>
        profile.id === selectedProfile
          ? { ...profile, ddos: currentConfigBackup }
          : profile
      )
    );
    setModalVisible(false);
    setSelectedProfile(null);
  };

  const profileOptions = profiles.map((profile) => ({
    label: profile.name,
    value: profile.id,
  }));
  return (
    <InterfaceLayout selectedKey="ipsec-profile">
      <Title level={3}>IP Security Profile</Title>
      <Flex justify="space-between" align="center" style={{ marginBottom: 16 }}>
        <Paragraph>
          Setting your network interface to protect your network.
        </Paragraph>
        <Flex gap="8px">
          <Button type="default">Save</Button>
          <Button type="primary" onClick={handleSaveAs}>
            Save as
          </Button>
        </Flex>
      </Flex>
      <IpSecProfile />
      <Modal
        title="Save Configuration As"
        open={modalVisible}
        onCancel={() => setModalVisible(false)}
        footer={[
          <Button key="cancel" onClick={() => setModalVisible(false)}>
            Cancel
          </Button>,
          <Button
            key="apply"
            type="primary"
            onClick={handleApply}
            disabled={!selectedProfile}
          >
            Apply
          </Button>,
        ]}
      >
        <Paragraph>
          Select the config name to save the current IPSec configuration:
        </Paragraph>
        <Select
          placeholder="Choose a config name"
          style={{ width: "100%" }}
          options={profileOptions}
          onChange={setSelectedProfile}
        />
      </Modal>
    </InterfaceLayout>
  );
};

export default IpSecProfileList;
