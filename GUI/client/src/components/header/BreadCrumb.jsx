import { Link, useLocation, useNavigate } from "react-router-dom";
import { Breadcrumb, Button } from "antd";
import { useEffect, useState } from "react";
import { ArrowLeftOutlined } from "@ant-design/icons";

const BreadCrumb = () => {
  const location = useLocation();
  const navigate = useNavigate();
  const [canGoBack, setCanGoBack] = useState(false);

  // Kiểm tra xem trang đầu tiên người dùng truy cập sau khi đăng nhập
  useEffect(() => {
    // Kiểm tra nếu đây là lần truy cập đầu tiên
    if (!sessionStorage.getItem("firstPage")) {
      sessionStorage.setItem("firstPage", location.pathname); // Lưu trang đầu tiên
    }

    // Check if the browser history length is greater than 1
    if (window.history.length > 1) {
      setCanGoBack(true);
    } else {
      setCanGoBack(false);
    }
  }, [location]);

  const handleBack = () => {
    const firstPage = sessionStorage.getItem("firstPage");
    const currentPage = location.pathname;

    // Nếu đang ở trang đầu tiên sau đăng nhập thì không cho quay lại
    if (firstPage === currentPage) {
      return;
    }

    // Nếu có thể quay lại và không phải trang đầu tiên
    if (canGoBack) {
      navigate(-1);
    }
  };

  const pathSnippets = location.pathname.split("/").filter((i) => i);

  const transformPath = (path) => {
    if (path === "defense-profile-list") return "Defense Profile Settings";
    else if (path === "profile") return "Defense Profile";
    else if (path === "ipsec-profile") return "IP Security Profile";
    else if (path === "ipsec-settings") return "IP Security Settings";
    else if (path === "port-mirroring") return "Port Mirroring";
    else if (path === "attack-list") return "Attack List";
    else if (path === "network-protect") return "Network Protecting";
    else if (path === "http-attacker") return "HTTP Blacklist";
    else if (path === "vpn-allowed") return "VPN Server Whitelist";
    else if (path === "synFlood") return "SYN Flood";
    else if (path === "udpFlood") return "UDP Flood";
    else if (path === "icmpFlood") return "ICMP Flood";
    else if (path === "dnsFlood") return "DNS Flood";
    else if (path === "httpFlood") return "HTTP Flood";
    else if (path === "land") return "Land Attack";
    else if (path === "ipsec") return "IPSec IKE Flood";
    else if (path === "tcpFrag") return "TCP Fragmentation";
    else if (path === "udpFrag") return "UDP Fragmentation";
    return path.charAt(0).toUpperCase() + path.slice(1);
  };

  const extraBreadcrumbItems = pathSnippets.map((_, index) => {
    const url = `/${pathSnippets.slice(0, index + 1).join("/")}`;
    return {
      title: transformPath(_),
      href: url,
    };
  });

  const breadcrumbItems = [
    {
      title: "Home",
      href: "/",
    },
  ].concat(extraBreadcrumbItems);

  // Kiểm tra xem có phải là trang đầu tiên sau đăng nhập không
  const isFirstPage = sessionStorage.getItem("firstPage") === location.pathname;

  return (
    <div style={{ display: "flex", alignItems: "center", padding: "20px" }}>
      <Button
        type="text"
        icon={
          <ArrowLeftOutlined
            style={{
              fontSize: "15px",
              color: canGoBack && !isFirstPage ? "#000" : "#ccc",
            }}
          />
        }
        onClick={handleBack}
        disabled={!canGoBack || isFirstPage}
        style={{ marginRight: "10px" }}
      />
      <Breadcrumb separator=">">
        {breadcrumbItems.map((item, index) => (
          <Breadcrumb.Item key={index} href={item.href}>
            {item.title}
          </Breadcrumb.Item>
        ))}
      </Breadcrumb>
    </div>
  );
};

export default BreadCrumb;
