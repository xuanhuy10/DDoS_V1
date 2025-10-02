import { Typography } from "antd";
const { Title, Paragraph } = Typography;

import InterfaceLayout from "@/features/defense/interface/components/InterfaceLayout";
import PortInterface from "@/features/defense/interface/components/InterfaceGet";

export default function General() {
  return (
    <InterfaceLayout selectedKey="general">
      <Title level={3}>General</Title>
      <Paragraph>
        Setting your network interface to protect your network.
      </Paragraph>
      <PortInterface />
    </InterfaceLayout>
  );
}
