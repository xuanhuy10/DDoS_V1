import PageTitle from "@/components/common/PageTitle";
import IpSecCertification from "@/features/defense/ipSetting/certificate/IpSecCertification";
import IpSecProfileSetting from "@/features/defense/ipSetting/setting/IpSecProfileSetting";
import IpSecProgress from "@/features/defense/ipSetting/setting/IpSecProgress";
import { faCog, faCheck, faAward } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { Button, Space, Steps, Switch, Modal, Progress } from "antd";
import React, { useState, useEffect, useRef } from "react";
import { insertIpSecProfileAndCertificates } from "@/features/api/Ipsecurity";
import {
  SettingOutlined,
  CheckCircleFilled,
  CloseCircleFilled,
} from "@ant-design/icons";

const IpSecSetting = () => {
  const [isEnabled, setIsEnabled] = useState(false);
  const [current, setCurrent] = useState(0);
  const [certificationValid, setCertificationValid] = useState(false);
  const [inputValid, setInputValid] = useState(false);
  const [uploadedFiles, setUploadedFiles] = useState({
    "ca-cert": { name: "key.der", size: 1024, content: null }, // Preload existing
    cert: { name: "key.der", size: 1024, content: null }, // Preload existing
    "private-key": {
      name: "key.der",
      size: 1024,
      content: null,
    }, // Preload existing
  });

  const [formData, setFormData] = useState(null);
  const [isSubmitting, setIsSubmitting] = useState(false);
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [timeLeft, setTimeLeft] = useState(0);
  const [apiStatus, setApiStatus] = useState("idle");
  const countdownIntervalRef = useRef(null);
  const [errorMessage, setErrorMessage] = useState("");

  useEffect(() => {
    const preloadFiles = async () => {
      const filesToPreload = [
        { type: "ca-cert", path: "/assets/key.der" }, // Adjust path to your actual file
        { type: "cert", path: "/assets/key.der" },
        { type: "private-key", path: "/assets/key.der" },
      ];

      const updatedFiles = { ...uploadedFiles };

      for (const { type, path } of filesToPreload) {
        try {
          const response = await fetch(path);
          if (response.ok) {
            const arrayBuffer = await response.arrayBuffer();
            updatedFiles[type] = {
              ...updatedFiles[type],
              content: arrayBuffer,
            };
          } else {
            console.warn(`Failed to preload ${path}: ${response.statusText}`);
          }
        } catch (error) {
          console.error(`Error preloading ${path}:`, error);
        }
      }

      setUploadedFiles(updatedFiles);
    };

    preloadFiles();
  }, []);

  // Cleanup on component unmount
  useEffect(() => {
    return () => {
      if (countdownIntervalRef.current) {
        clearInterval(countdownIntervalRef.current);
      }
    };
  }, []);

  const handleSwitchChange = (checked) => {
    setIsEnabled(checked);
  };

  const handleCertificationChange = (isValid) => {
    setCertificationValid(isValid);
  };

  const handleInputValidation = (isValid, data) => {
    setInputValid(isValid);
    setFormData(data);
  };

  const steps = [
    {
      title: "CERTIFICATION",
      description: "Import server certification",
      icon: <FontAwesomeIcon icon={faAward} />,
      content: (
        <IpSecCertification
          onValidationChange={handleCertificationChange}
          uploadedFiles={uploadedFiles}
          setUploadedFiles={setUploadedFiles}
          allowSkip={true}
          onSkip={() => setCertificationValid(true)}
        />
      ),
    },
    {
      title: "IP Security Setting",
      description: "Node Specific & General Setting",
      icon: <FontAwesomeIcon icon={faCog} />,
      content: (
        <IpSecProfileSetting onValidationChange={handleInputValidation} />
      ),
    },
    {
      title: "Complete",
      description: "Profile creation finished",
      icon: <FontAwesomeIcon icon={faCheck} />,
      content: <IpSecProgress />,
    },
  ];

  const startCountdown = () => {
    setTimeLeft(28);
    setApiStatus("processing");

    if (countdownIntervalRef.current) {
      clearInterval(countdownIntervalRef.current);
    }

    countdownIntervalRef.current = setInterval(() => {
      setTimeLeft((prevTime) => {
        if (prevTime <= 1) {
          clearInterval(countdownIntervalRef.current);
          return 0;
        }
        return prevTime - 1;
      });
    }, 1000);
  };

  const handleApiResponse = async () => {
    setIsSubmitting(true);
    setIsModalOpen(true);
    startCountdown();

    try {
      // Prepare profile data
      const connectionCount = formData.ConnectionCount || 1;
      const profileData = {
        ProfileName: formData.IpSecProfileName,
        ProfileDescription: formData.IpSecProfileDescription,
        LocalGateway: formData.LocalGatewayIPv4 || formData.LocalGatewayIPv6,
        SubnetLocalGateway:
          formData.SubnetLocalGatewayIPv4 || formData.SubnetLocalGatewayIPv6,

        RemoteGateway:
          connectionCount === 1
            ? formData.RemoteGatewayIPv4 || formData.RemoteGatewayIPv6
            : Array.from(
                { length: connectionCount },
                (_, index) =>
                  formData[`RemoteGatewayIPv4_${index}`] ||
                  formData[`RemoteGatewayIPv6_${index}`]
              )
                .filter(Boolean)
                .join(";"), // Lọc null/undefined và join

        // Tương tự cho SubnetRemoteGateway
        SubnetRemoteGateway:
          connectionCount === 1
            ? formData.SubnetRemoteGatewayIPv4 ||
              formData.SubnetRemoteGatewayIPv6
            : Array.from(
                { length: connectionCount },
                (_, index) =>
                  formData[`SubnetRemoteGatewayIPv4_${index}`] ||
                  formData[`SubnetRemoteGatewayIPv6_${index}`]
              )
                .filter(Boolean)
                .join(";"),

        IKEVersion: formData.IKE2Version,
        Mode: formData.IKEMode,
        ESPAHProtocol: formData.ESPAHProtocol,
        IKEReauthTime: formData.IKEReauthTime
          ? parseInt(formData.IKEReauthTime, 10)
          : null,
        EncryptionAlgorithm: formData.EncryptionAlgorithm,
        HashAlgorithm: formData.HashAlgorithm,
        ReKeyTime: formData.ReKeyTime ? parseInt(formData.ReKeyTime, 10) : null,
        Enable: isEnabled,
        ConnectionCount: connectionCount,
      };

      // Call combined API
      const response = await insertIpSecProfileAndCertificates(
        profileData,
        uploadedFiles
      );

      if (countdownIntervalRef.current) {
        clearInterval(countdownIntervalRef.current);
      }

      if (response.status === 200) {
        setApiStatus("success");
        setTimeout(() => {
          setIsModalOpen(false);
          setCurrent(current + 1);
        }, 1500);
      } else {
        setApiStatus("error");
        setErrorMessage(response.data?.message || "Unknown error occurred"); // Lưu thông điệp lỗi từ backend
      }
    } catch (error) {
      if (countdownIntervalRef.current) {
        clearInterval(countdownIntervalRef.current);
      }
      setApiStatus("error");
      setErrorMessage(
        error.response?.data?.message ||
          error.message ||
          "Unknown error occurred"
      ); // Lưu thông điệp lỗi từ backend
    } finally {
      setIsSubmitting(false);
    }
  };

  const next = async () => {
    if (current === 1 && !inputValid) {
      return;
    }

    if (current === 1) {
      await handleApiResponse();
    } else {
      setCurrent(current + 1);
    }
  };

  const prev = () => {
    setCurrent(current - 1);
  };

  const closeModal = () => {
    if (countdownIntervalRef.current) {
      clearInterval(countdownIntervalRef.current);
    }
    setIsModalOpen(false);
    setApiStatus("idle");
  };

  // Hàm tải lại trang
  const reloadPage = () => {
    window.location.reload();
  };

  const items = steps.map((item) => ({
    key: item.title,
    title: item.title,
    description: item.description,
    icon: item.icon,
  }));

  const progressPercent =
    apiStatus === "processing"
      ? Math.max(0, Math.min(100, ((28 - timeLeft) / 28) * 100))
      : 100;

  const getModalContent = () => {
    switch (apiStatus) {
      case "processing":
        return {
          icon: (
            <SettingOutlined style={{ fontSize: 48, color: "#1890ff" }} spin />
          ),
          title: "Saving IPSec configuration...",
          description: `Time remaining: ${timeLeft}s`,
          progress: true,
          showButton: false,
        };
      case "success":
        return {
          icon: (
            <CheckCircleFilled style={{ fontSize: 48, color: "#52c41a" }} />
          ),
          title: "Configuration Saved Successfully!",
          description: "Your settings have been applied successfully.",
          progress: false,
          showButton: false,
        };
      case "error":
        return {
          icon: (
            <CloseCircleFilled style={{ fontSize: 48, color: "#ff4d4f" }} />
          ),
          title: "Failed to Save Configuration",
          description: errorMessage,
          progress: false,
          showButton: true,
        };
      default:
        return {
          icon: (
            <SettingOutlined style={{ fontSize: 48, color: "#1890ff" }} spin />
          ),
          title: "Saving IPSec configuration...",
          description: `Time remaining: ${timeLeft}s`,
          progress: true,
          showButton: false,
        };
    }
  };

  const modalContent = getModalContent();

  return (
    <div style={{ padding: "5px 15px" }}>
      <Modal
        open={isModalOpen}
        footer={null}
        closable={apiStatus !== "processing"}
        onCancel={closeModal}
        centered
        maskClosable={apiStatus !== "processing"}
        width={450}
      >
        <div style={{ textAlign: "center", padding: 20 }}>
          {modalContent.icon}
          <p style={{ marginTop: 16, fontSize: 18, fontWeight: "bold" }}>
            {modalContent.title}
          </p>
          <p style={{ fontSize: 16, color: "#888", margin: "10px 0" }}>
            {modalContent.description}
          </p>
          {modalContent.progress && (
            <Progress
              percent={progressPercent}
              status="active"
              showInfo={false}
              strokeColor={{
                from: "#108ee9",
                to: "#87d068",
              }}
              style={{ margin: "15px 0" }}
            />
          )}
          {modalContent.showButton && (
            <Button
              type="primary"
              onClick={reloadPage} // Tải lại trang khi ấn Try Again
              style={{ marginTop: 15 }}
            >
              Try Again
            </Button>
          )}
        </div>
      </Modal>

      <Space
        align="center"
        style={{
          marginBottom: "20px",
          display: "flex",
          justifyContent: "space-between",
          width: "100%",
        }}
      >
        <PageTitle
          title="Create IpSec Profile"
          description="Create your own IpSec profile for network protection"
        />
        <Switch
          onChange={handleSwitchChange}
          checked={isEnabled}
          checkedChildren="Enabled"
          unCheckedChildren="Disabled"
        />
      </Space>

      <Steps
        current={current}
        items={items}
        style={{
          display: "flex",
          justifyContent: "center",
          maxWidth: "900px",
          margin: "0 auto",
        }}
      />

      <div style={{ marginTop: 24 }}>{steps[current].content}</div>

      {current < steps.length - 1 && (
        <div style={{ marginTop: 24 }}>
          <Button
            type="primary"
            onClick={next}
            disabled={
              isSubmitting ||
              (current === 0 && !certificationValid) ||
              (current === 1 && !inputValid)
            }
            style={
              (current === 0 && !certificationValid) ||
              (current === 1 && !inputValid)
                ? { opacity: 0.6 }
                : {}
            }
            loading={isSubmitting}
          >
            Next
          </Button>
          {current > 0 && (
            <Button
              style={{ margin: "0 8px" }}
              onClick={prev}
              disabled={isSubmitting}
            >
              Previous
            </Button>
          )}
        </div>
      )}
    </div>
  );
};

export default IpSecSetting;
