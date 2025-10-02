import { Link, useLocation } from "react-router-dom";

import { Menu, Modal } from "antd";
import {
  AppstoreOutlined,
  BarChartOutlined,
  DashboardOutlined,
  LogoutOutlined,
  SafetyOutlined,
  SettingOutlined,
  UserOutlined,
} from "@ant-design/icons";

import { useAuth } from "@/hooks/useAuth";

const MenuList = ({ darkTheme }) => {
  const location = useLocation();
  const MapLocationPathToKey = (path) => {
    if (path === "/") {
      return "dashboard";
    } else if (path === "/defense") {
      return "defense";
    } else if (path.startsWith("/defense/interface")) {
      return "interface";
    } else if (path === "/defense/profile") {
      return "profile";
    } else if (path === "/defense/ipsec-profile") {
      return "ipsec-profile";
    } else if (path === "/analyze") {
      return "analyze";
    } else if (path.startsWith("/monitor")) {
      return "monitor";
    } else if (path === "/manager") {
      return "manager";
    } else if (path === "/manager/users") {
      return "user-manager";
    } else if (path === "/manager/logs") {
      return "log-manager";
    } else if (path === "/manager/device") {
      return "other-setting";
    } else if (path === "/user/profile") {
      return "user";
    } else if (path === "/notification") {
      return "noti";
    } else {
      return "";
    }
  };
  const selectedKey = MapLocationPathToKey(location.pathname);

  const { user, logout } = useAuth();

  const handleLogout = () => {
    Modal.confirm({
      title: "Logout",
      content: "Are you sure you want to logout?",
      onOk: () => logout(),
    });
  };

  const items = [
    {
      key: "dashboard",
      icon: <AppstoreOutlined />,
      label: <Link to="/">DASHBOARD</Link>,
    },
    {
      key: "defense",
      icon: <SafetyOutlined />,
      label: (
        <Link
          to="/defense/interface"
          style={{ textDecoration: "none", color: "inherit" }}
        >
          DEFENSE
        </Link>
      ),
      children: [
        {
          key: "interface",
          label: <Link to="/defense/interface">Interface</Link>,
        },
        {
          key: "profile",
          label: <Link to="/defense/profile">Defense Profile</Link>,
        },
        {
          key: "ipsec-settings",
          label: <Link to="/defense/ipsec-settings">IP Security Settings</Link>,
        },
      ],
    },
    {
      key: "analyze",
      icon: <BarChartOutlined />,
      label: <Link to="/analyze">ANALYZE</Link>,
    },
    {
      key: "monitor",
      icon: <DashboardOutlined />,
      label: <Link to="/monitor">MONITOR</Link>,
    },
    {
      key: "manager",
      icon: <SettingOutlined />,
      label: (
        <Link
          to="/manager"
          style={{ textDecoration: "none", color: "inherit" }}
        >
          MANAGEMENT
        </Link>
      ),
      disabled: user?.role !== "admin",
      children: [
        { key: "user-manager", label: <Link to="/manager/users">Users</Link> },
        {
          key: "log-manager",
          label: <Link to="/manager/logs">Activities & Logs</Link>,
        },
        {
          key: "other-setting",
          label: <Link to="/manager/device">Device</Link>,
        },
      ],
    },
    {
      type: "divider",
    },
    {
      key: "user",
      icon: <UserOutlined />,
      label: <Link to="/user/profile">USER PROFILE</Link>,
    },
    // {
    //     key: 'noti',
    //     icon: <BellOutlined />,
    //     label: <Link to='/notification'>NOTIFICATION</Link>,
    // },
    {
      key: "logout",
      icon: <LogoutOutlined />,
      label: "LOGOUT",
      onClick: () => handleLogout(),
    },
  ];

  return (
    <Menu
      theme={darkTheme ? "dark" : "light"}
      mode="inline"
      className="menu-bar"
      selectedKeys={selectedKey}
      items={items}
    />
  );
};

export default MenuList;
