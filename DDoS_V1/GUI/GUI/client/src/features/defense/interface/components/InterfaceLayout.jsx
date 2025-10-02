import React, { useState, useEffect } from "react";
import { Link } from "react-router-dom";
import { Menu, Layout } from "antd";
const { Sider, Content } = Layout;
import {
  ProfileOutlined,
  WifiOutlined,
  UnorderedListOutlined,
  ControlOutlined,
  FileProtectOutlined,
  ExceptionOutlined,
  CloseSquareOutlined,
} from "@ant-design/icons";
import { GoMirror } from "react-icons/go";
import PageTitle from "@/components/common/PageTitle";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faAddressCard } from "@fortawesome/free-regular-svg-icons";

const items = [
  {
    key: "general",
    label: <Link to="/defense/interface">General</Link>,
    icon: <ControlOutlined />,
  },
  {
    key: "profile",
    label: (
      <Link to="/defense/interface/defense-profile-list">Defense Profile</Link>
    ),
    icon: <UnorderedListOutlined />,
  },
  {
    key: "ipsec-profile",
    label: (
      <Link to="/defense/interface/ipsec-profile">IP Security Profile </Link>
    ),
    icon: <FontAwesomeIcon icon={faAddressCard} />,
  },
  {
    key: "mirroring",
    label: <Link to="/defense/interface/port-mirroring">Port Mirroring</Link>,
    icon: <GoMirror />,
  },
  {
    key: "network",
    label: "Network Groups",
    icon: <WifiOutlined />,
    children: [
      {
        key: "protected",
        icon: <ProfileOutlined />,
        label: (
          <Link to="/defense/interface/network-protect">
            Network Protection
          </Link>
        ),
      },
      {
        key: "acl",
        label: "Access Control List",
        type: "group",
        children: [
          {
            key: "blocked",
            icon: <CloseSquareOutlined />,
            label: (
              <Link to="/defense/interface/attack-list">Attacker List</Link>
            ),
          },
          {
            key: "vpn_white",
            icon: <FileProtectOutlined />,
            label: (
              <Link to="/defense/interface/vpn-allowed">
                VPN Server Whitelist
              </Link>
            ),
          },
          {
            key: "http_black",
            icon: <ExceptionOutlined />,
            label: (
              <Link to="/defense/interface/http-attacker">
                HTTP/HTTPs Blacklist
              </Link>
            ),
          },
        ],
      },
    ],
  },
];

export default function InterfaceLayout({ selectedKey, children }) {
  const [openKeys, setOpenKeys] = useState([]);

  useEffect(() => {
    const networkChildren = [
      "protected",
      "acl",
      "blocked",
      "vpn_white",
      "http_black",
    ];
    const aclChildren = ["blocked", "vpn_white", "http_black"];

    let next = openKeys;
    let changed = false;

    if (
      networkChildren.includes(selectedKey) &&
      !openKeys.includes("network")
    ) {
      next = [...openKeys, "network"];
      changed = true;
    }
    if (aclChildren.includes(selectedKey) && !next.includes("acl")) {
      next = [...next, "acl"];
      changed = true;
    }
    if (changed) setOpenKeys(next);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [selectedKey]);

  const handleOpenChange = (keys) => {
    setOpenKeys(keys);
  };

  return (
    <>
      <PageTitle
        title="Interface"
        description="Configure your network defense interfaces"
      />
      <Layout style={{ borderRadius: "10px", overflow: "hidden" }}>
        <Sider width={256} style={{ background: "#fff" }}>
          <Menu
            style={{ height: "100%" }}
            mode="inline"
            items={items}
            selectedKeys={[selectedKey]}
            openKeys={openKeys}
            onOpenChange={handleOpenChange}
          />
        </Sider>
        <Layout>
          <Content style={{ padding: "24px", background: "#fff" }}>
            {children}
          </Content>
        </Layout>
      </Layout>
    </>
  );
}
