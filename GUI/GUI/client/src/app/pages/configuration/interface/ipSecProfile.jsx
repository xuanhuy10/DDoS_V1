import { Typography } from "antd";
const { Title, Paragraph } = Typography;

import InterfaceLayout from "@/features/defense/interface/components/InterfaceLayout";
import IpSecProfile from "@/features/defense/interface/components/IpSecProfile";

const IpSecProfileList = () => {
  return (
    <InterfaceLayout selectedKey="ipsec-profile">
      <Title level={3}>IP Security Profile</Title>
      <Paragraph>
        Setting your network interface to protect your network.
      </Paragraph>
      <IpSecProfile />
    </InterfaceLayout>
  );
};

export default IpSecProfileList;
