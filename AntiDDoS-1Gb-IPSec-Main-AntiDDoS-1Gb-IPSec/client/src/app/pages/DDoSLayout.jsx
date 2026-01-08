import { useState, useEffect } from "react";
import { Button, theme, Layout, Row, Col } from "antd";
import { Outlet } from "react-router-dom";

const { Header, Sider, Content } = Layout;

import Logo from "@/components/sidebar/Logo";
import MenuList from "@/components/sidebar/MenuList";
import BreadCrumb from "@/components/header/BreadCrumb";

import ddosLogo from "@/assets/ddos_logo_black.svg";

function DDoSLayout() {
  const [isDarkTheme, setDarkTheme] = useState(true);
  const [isCollapsed, setCollapsed] = useState(true);

  const toggleTheme = () => {
    setDarkTheme(!isDarkTheme);
  };
  const {
    token: { colorBgContainer },
  } = theme.useToken();

  // const getSocketData = useTrafxStore((state) => state.getSocketData);
  // const trafixSum = useTrafxStore((state) => state.trafixSum);

  // useEffect(() => {
  //     getSocketData(); // Initialize only if not already connected
  // }, []);

  // useEffect(() => {
  //     console.log("data:", trafixSum);
  // }, [trafixSum]);

  return (
    <Layout>
      <Sider
        width={230}
        collapsed={isCollapsed}
        collapsible
        trigger={null}
        theme={isDarkTheme ? "dark" : "light"}
        style={{
          overflow: "auto",
          height: "100vh",
          position: "sticky",
          top: 0,
          left: 0,
        }}
        onMouseEnter={() => setCollapsed(false)}
        onMouseLeave={() => setCollapsed(true)}
      >
        <Logo />
        <MenuList darkTheme={isDarkTheme} />
      </Sider>
      <Layout>
        <Header
          style={{
            background: colorBgContainer,
            position: "sticky",
            top: 0,
            zIndex: 1,
            width: "100%",
            display: "flex",
            alignItems: "center",
            padding: "0 20px",
            justifyContent: "space-between",
          }}
        >
          <BreadCrumb />
          <img src={ddosLogo} style={{ width: "120px" }} />
        </Header>

        {/* pages content */}
        <Content>
          <div className="wrapper">
            <Outlet />
          </div>
        </Content>
      </Layout>
    </Layout>
  );
}

export default DDoSLayout;
