import {
  Flex,
  Form,
  InputNumber,
  Typography,
  Input,
  Radio,
  Divider,
  Select,
} from "antd";
import { ClockCircleOutlined } from "@ant-design/icons";
const { Title } = Typography;

const onChange = (value) => {
  console.log(`selected ${value}`);
};
const onSearch = (value) => {
  console.log("search:", value);
};

const ThresholdInput = ({ label, name, min = 1, max = 65535, prefix }) => (
  <Form.Item
    label={<Title level={5}>{label}</Title>}
    name={name}
    rules={[{ required: true, message: `Please set ${label.toLowerCase()}!` }]}
  >
    <InputNumber
      min={min}
      max={max}
      style={{ width: "100%" }}
      prefix={prefix}
    />
  </Form.Item>
);

const NodeSpecSetting = () => {
  // Use Form.useWatch to get the current values for conditional enabling/disabling
  const localGatewayIPv4 = Form.useWatch("LocalGatewayIPv4");
  const localGatewayIPv6 = Form.useWatch("LocalGatewayIPv6");
  const connectionCount = Form.useWatch("ConnectionCount");

  const validateIpAddress = (fieldName, ipv4, ipv6, message) => ({
    validator(_, value) {
      if (!ipv4 && !ipv6) {
        return Promise.reject(new Error(message));
      }
      return Promise.resolve();
    },
  });
  const remoteGatewayIPv4Fields = Array.from({ length: 10 }, (_, index) =>
    Form.useWatch(
      index === 0 && connectionCount === 1
        ? "RemoteGatewayIPv4"
        : `RemoteGatewayIPv4_${index}`
    )
  );
  const remoteGatewayIPv6Fields = Array.from({ length: 10 }, (_, index) =>
    Form.useWatch(
      index === 0 && connectionCount === 1
        ? "RemoteGatewayIPv6"
        : `RemoteGatewayIPv6_${index}`
    )
  );
  return (
    <div className="config-panes">
      <Form.Item
        label={<Title level={4}> IPSec Profile name</Title>}
        name="IpSecProfileName"
        rules={[
          {
            required: true,
            message: "Please set your profile name!",
          },
          { max: 100, message: "The maximum length is 100 characters!" },
        ]}
      >
        <Input placeholder="Please set your profile name!" />
      </Form.Item>

      <Form.Item
        label={<Title level={5}> IPSec Profile Description</Title>}
        name="IpSecProfileDescription"
        rules={[
          {
            required: true,
            message: "Please give a description for your profile!",
          },
        ]}
      >
        <Input.TextArea placeholder="Please give a description for your profile!" />
      </Form.Item>

      <Divider variant="solid" style={{ borderColor: "#001529" }}>
        Node Specific Settings
      </Divider>

      <Form.Item
        label={<Title level={4}>Connection </Title>}
        name="ConnectionCount"
        rules={[
          { required: true, message: "Please set the number of connections!" },
        ]}
      >
        <InputNumber min={1} max={10} style={{ width: "100%" }} />
      </Form.Item>

      <Form.Item
        label={<Title level={4}>Local Gateway Ip Address</Title>}
        required
        rules={[
          validateIpAddress(
            "LocalGateway",
            localGatewayIPv4,
            localGatewayIPv6,
            "Please set either IPv4 or IPv6 for the local gateway!"
          ),
        ]}
      >
        <Input.Group compact>
          <Form.Item
            name="LocalGatewayIPv4"
            noStyle
            rules={[
              {
                pattern:
                  /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$/,
                message: "Invalid IPv4 address",
                required: false,
              },
            ]}
          >
            <Input
              style={{ width: "50%" }}
              placeholder="IPv4"
              disabled={!!localGatewayIPv6}
            />
          </Form.Item>
          <Form.Item
            name="SubnetLocalGatewayIPv4"
            noStyle
            rules={[
              {
                pattern:
                  /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\/(3[0-2]|[1-2]?[0-9]))?$/,
                message: "Invalid IPv6 address",
                required: false,
              },
            ]}
          >
            <Input
              style={{ width: "50%" }}
              placeholder="Subnet Local Gateway Ipv4"
              disabled={!!localGatewayIPv6}
            />
          </Form.Item>
        </Input.Group>

        <Input.Group compact>
          <Form.Item
            name="LocalGatewayIPv6"
            noStyle
            rules={[
              {
                pattern:
                  /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9]))$/,
                message: "Invalid IPv6 address",
                required: false,
              },
            ]}
          >
            <Input
              style={{ width: "50%" }}
              placeholder="IPv6"
              disabled={!!localGatewayIPv4}
            />
          </Form.Item>
          <Form.Item
            name="SubnetLocalGatewayIPv6"
            noStyle
            rules={[
              {
                pattern:
                  /^s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|...)(\/([0-9]|[1-9][0-9]|1[0-1][0-9]|12[0-8]))?$/,
                message: "Invalid Subnet IPv6 address",
                required: false,
              },
            ]}
          >
            <Input
              style={{ width: "50%" }}
              placeholder="Subnet Local Gateway Ipv6"
              disabled={!!localGatewayIPv4}
            />
          </Form.Item>
        </Input.Group>
      </Form.Item>

      {Array.from({ length: connectionCount || 1 }, (_, index) => {
        const remoteGatewayIPv4Field =
          connectionCount === 1
            ? "RemoteGatewayIPv4"
            : `RemoteGatewayIPv4_${index}`;
        const remoteGatewayIPv6Field =
          connectionCount === 1
            ? "RemoteGatewayIPv6"
            : `RemoteGatewayIPv6_${index}`;
        const remoteGatewayIPv4 = remoteGatewayIPv4Fields[index];
        const remoteGatewayIPv6 = remoteGatewayIPv6Fields[index];

        return (
          <Form.Item
            key={`remote-gateway-${index}`}
            label={
              <Title level={4}>
                Remote Gateway IP Address{" "}
                {connectionCount > 1 ? `#${index + 1}` : ""}
              </Title>
            }
            rules={[
              validateIpAddress(
                `RemoteGateway_${index}`,
                remoteGatewayIPv4,
                remoteGatewayIPv6,
                `Please set either IPv4 or IPv6 for the remote gateway ${
                  connectionCount > 1 ? index + 1 : ""
                }!`
              ),
            ]}
            required
          >
            <Input.Group compact>
              <Form.Item
                name={remoteGatewayIPv4Field}
                noStyle
                rules={[
                  {
                    pattern:
                      /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$/,
                    message: "Invalid IPv4 address",
                    required: false,
                  },
                ]}
              >
                <Input
                  style={{ width: "50%" }}
                  placeholder="IPv4"
                  disabled={!!remoteGatewayIPv6}
                />
              </Form.Item>
              <Form.Item
                name={
                  connectionCount === 1
                    ? "SubnetRemoteGatewayIPv4"
                    : `SubnetRemoteGatewayIPv4_${index}`
                }
                noStyle
                rules={[
                  {
                    pattern:
                      /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\/(3[0-2]|[1-2]?[0-9]))?$/,
                    message: "Invalid IPv4 subnet (e.g., 10.0.0.0/24)",
                    required: false,
                  },
                ]}
              >
                <Input
                  style={{ width: "50%" }}
                  placeholder={`Subnet Remote Gateway IPv4 ${
                    connectionCount > 1 ? index + 1 : ""
                  }`}
                  disabled={!!remoteGatewayIPv6}
                />
              </Form.Item>
            </Input.Group>

            <Input.Group compact>
              <Form.Item
                name={remoteGatewayIPv6Field}
                noStyle
                rules={[
                  {
                    pattern:
                      /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9]))$/,
                    message: "Invalid IPv6 address",
                    required: false,
                  },
                ]}
              >
                <Input
                  style={{ width: "50%" }}
                  placeholder="IPv6"
                  disabled={!!remoteGatewayIPv4}
                />
              </Form.Item>
              <Form.Item
                name={
                  connectionCount === 1
                    ? "SubnetRemoteGatewayIPv6"
                    : `SubnetRemoteGatewayIPv6_${index}`
                }
                noStyle
                rules={[
                  {
                    pattern:
                      /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9])?[0-9]))(\/([0-9]|[1-9][0-9]|1[0-1][0-9]|12[0-8]))?$/,
                    message: "Invalid Subnet IPv6 address",
                    required: false,
                  },
                ]}
              >
                <Input
                  style={{ width: "50%" }}
                  placeholder={`Subnet Remote Gateway IPv6 ${
                    connectionCount > 1 ? index + 1 : ""
                  }`}
                  disabled={!!remoteGatewayIPv4}
                />
              </Form.Item>
            </Input.Group>
          </Form.Item>
        );
      })}
    </div>
  );
};

const IKESetting = () => {
  const protocol = Form.useWatch("ESPAHProtocol");
  return (
    <Flex vertical gap="middle">
      <div className="config-panes">
        <Title level={3}>IKEv2 Settings</Title>

        <Form.Item
          label={<Title level={4}>IKE2 version</Title>}
          name="IKE2Version"
          rules={[{ required: true, message: "Please set IKE2 version!" }]}
        >
          <Input placeholder="Set IKE2 version!" />
        </Form.Item>

        <Form.Item
          label={<Title level={4}>Mode</Title>}
          name="IKEMode"
          rules={[{ required: true, message: "Please select a mode!" }]}
        >
          <Radio.Group>
            <Radio value="tunnel">Tunnel Mode</Radio>
            <Radio value="transport">Transport Mode</Radio>
          </Radio.Group>
        </Form.Item>

        <Form.Item
          label={<Title level={4}>ESP/AH Protocol</Title>}
          name="ESPAHProtocol"
          rules={[
            {
              required: true,
              message: "Please set ESP/AH Protocol!",
            },
          ]}
        >
          <Select
            showSearch
            placeholder="Select ESP/AH Protocol"
            optionFilterProp="label"
            onChange={onChange}
            onSearch={onSearch}
            options={[
              { value: "ESP", label: "ESP Protocol" },
              { value: "AH", label: "AH Protocol" },
            ]}
          />
        </Form.Item>

        <ThresholdInput
          label="IKE Reauth time (optional)"
          name="IKEReauthTime"
          min={1}
          max={86400}
          prefix={<ClockCircleOutlined />}
        />

        <Form.Item
          label={<Title level={4}>Encryption Algorithm</Title>}
          name="EncryptionAlgorithm"
          rules={[
            { required: true, message: "Please set encryption algorithm!" },
          ]}
        >
          <Select
            showSearch
            placeholder="Select encryption algorithm"
            optionFilterProp="label"
            onChange={onChange}
            onSearch={onSearch}
            options={[
              { value: "aes-128", label: "AES-128" },
              { value: "aes-192", label: "AES-192" },
              { value: "aes-256", label: "AES-256" },
              { value: "3des", label: "3DES" },
              { value: "des", label: "DES" },
            ]}
          />
        </Form.Item>

        {protocol === "ESP" && (
          <Form.Item
            label={<Title level={4}>Hash Algorithm</Title>}
            name="HashAlgorithm"
            rules={[{ required: true, message: "Please set hash algorithm!" }]}
          >
            <Select
              showSearch
              placeholder="Select hash algorithm"
              optionFilterProp="label"
              onChange={onChange}
              onSearch={onSearch}
              options={[
                { value: "md5", label: "MD5" },
                { value: "sha1", label: "SHA-1" },
                { value: "sha256", label: "SHA-256" },
                { value: "sha384", label: "SHA-384" },
                { value: "sha512", label: "SHA-512" },
              ]}
            />
          </Form.Item>
        )}

        <ThresholdInput
          label="Re-key Time"
          name="ReKeyTime"
          min={60}
          max={86400}
          prefix={<ClockCircleOutlined />}
        />
      </div>
    </Flex>
  );
};

export { NodeSpecSetting, IKESetting };
