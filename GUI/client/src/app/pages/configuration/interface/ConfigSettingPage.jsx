import { Typography } from "antd";
const { Title, Paragraph } = Typography;

import InterfaceLayout from "@/features/defense/interface/components/InterfaceLayout";
import ConfigSettingProfile from "@/features/defense/interface/components/ConfigSettingProfile";

const ConfigSettingPage = () => {
  return (
    <InterfaceLayout selectedKey="config-setting-profile">
      <Title level={3}>Configuration Setting Profile</Title>
      <Paragraph>
        Manage your configuration setting profiles for network defense.
      </Paragraph>
      <ConfigSettingProfile />
    </InterfaceLayout>
  );
};

export default ConfigSettingPage;
