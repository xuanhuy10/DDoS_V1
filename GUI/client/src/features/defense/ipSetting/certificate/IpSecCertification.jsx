import React, { useEffect } from "react";
import { Row, Col, Typography, Button, Space, message, Upload } from "antd";
import {
  UploadOutlined,
  CheckCircleOutlined,
  DeleteOutlined,
  InfoCircleOutlined,
} from "@ant-design/icons";

const { Title, Text } = Typography;

const IpSecCertification = ({
  onValidationChange,
  uploadedFiles,
  setUploadedFiles,
  allowSkip = false,
  onSkip,
}) => {
  // Check if all files are uploaded and notify parent
  useEffect(() => {
    const allFilesUploaded = Object.values(uploadedFiles).every(
      (file) => file !== null
    );
    if (onValidationChange) {
      onValidationChange(allFilesUploaded);
    }
  }, [uploadedFiles, onValidationChange]);

  const getFileExtension = (filename) => {
    return filename.split(".").pop().toLowerCase();
  };

  const validateFile = (file, expectedExtensions) => {
    const fileExtension = getFileExtension(file.name);
    return expectedExtensions.includes(fileExtension);
  };

  const handleFileUpload = (file, fileType, expectedExtensions) => {
    if (!validateFile(file, expectedExtensions)) {
      message.error(
        `Please select a valid ${expectedExtensions.join(" or ")} file.`
      );
      return false;
    }

    // Read file as ArrayBuffer for binary data
    const reader = new FileReader();
    reader.onload = (e) => {
      setUploadedFiles((prev) => ({
        ...prev,
        [fileType]: {
          name: file.name,
          size: file.size,
          content: e.target.result, // ArrayBuffer
        },
      }));
    };
    reader.readAsArrayBuffer(file); // Read as binary
    return false; // Prevent default upload behavior
  };

  const handleRemoveFile = (fileType) => {
    setUploadedFiles((prev) => ({
      ...prev,
      [fileType]: null,
    }));
    message.info("File removed successfully");
  };

  const certificationItems = [
    {
      label: "CA Certification File",
      filename: "key.der",
      type: "ca-cert",
      extensions: ["der"],
    },
    {
      label: "Certification File",
      filename: "key.der",
      type: "cert",
      extensions: ["der"],
    },
    {
      label: "Private Key File",
      filename: "key.der",
      type: "private-key",
      extensions: ["der"],
    },
  ];

  const allFilesUploaded = Object.values(uploadedFiles).every(
    (file) => file !== null
  );

  const isUsingExisting = Object.values(uploadedFiles).every(
    (file) => file && file.name === "key.der"
  );

  const handleSkip = () => {
    if (onSkip) {
      onSkip();
    }
    message.info(
      "Using existing certificates. You can still upload new ones if needed."
    );
  };

  return (
    <div
      style={{
        padding: "24px",
        backgroundColor: "#f5f5f5",
        minHeight: "400px",
        opacity: 1,
        transition: "opacity 0.3s ease",
      }}
    >
      <Title level={3} style={{ marginBottom: "32px", color: "#262626" }}>
        IPSec Certification Management
      </Title>

      {allowSkip && isUsingExisting && (
        <div
          style={{
            backgroundColor: "#fff7e6",
            border: "1px solid #ffd591",
            borderRadius: "6px",
            padding: "12px 16px",
            marginBottom: "24px",
            textAlign: "center",
          }}
        >
          <InfoCircleOutlined
            style={{ color: "#fa8c16", marginRight: "8px" }}
          />
          <Text style={{ color: "#fa8c16", fontWeight: "bold" }}>
            Existing certificates are preloaded. You can skip or replace them.
          </Text>
          <Button
            type="link"
            onClick={handleSkip}
            style={{ marginLeft: 8, padding: 0 }}
          >
            Skip & Use Existing
          </Button>
        </div>
      )}

      {/* {allFilesUploaded && (
        <div
          style={{
            backgroundColor: "#f6ffed",
            border: "1px solid #b7eb8f",
            borderRadius: "6px",
            padding: "12px 16px",
            marginBottom: "24px",
            textAlign: "center",
          }}
        >
          <CheckCircleOutlined
            style={{ color: "#52c41a", marginRight: "8px" }}
          />
          <Text style={{ color: "#52c41a", fontWeight: "bold" }}>
            All certificate files uploaded successfully! You can proceed to the
            next step.
          </Text>
        </div>
      )} */}

      <Space direction="vertical" size="large" style={{ width: "100%" }}>
        {certificationItems.map((item, index) => (
          <Row
            key={index}
            align="middle"
            justify="center"
            style={{
              backgroundColor: "#ffffff",
              padding: "20px 24px",
              borderRadius: "8px",
              boxShadow: "0 2px 4px rgba(0,0,0,0.1)",
              border: "1px solid #e8e8e8",
              opacity: 1,
            }}
          >
            <Col flex="1">
              <Text strong style={{ fontSize: "16px", color: "#262626" }}>
                {item.label}
              </Text>
            </Col>

            <Col>
              <Space align="center" size="middle">
                {uploadedFiles[item.type] ? (
                  <Space align="center">
                    <Button
                      type="default"
                      icon={<CheckCircleOutlined />}
                      style={{
                        backgroundColor: "#f6ffed",
                        borderColor: "#b7eb8f",
                        color: "#52c41a",
                        fontWeight: "bold",
                        borderRadius: "6px",
                        height: "36px",
                        width: "120px",
                        paddingLeft: "16px",
                        paddingRight: "16px",
                      }}
                    >
                      Uploaded
                    </Button>
                    <Button
                      type="text"
                      icon={<DeleteOutlined />}
                      onClick={() => handleRemoveFile(item.type)}
                      style={{
                        color: "#ff4d4f",
                        height: "36px",
                      }}
                      title="Remove file"
                    />
                  </Space>
                ) : (
                  <Upload
                    beforeUpload={(file) =>
                      handleFileUpload(file, item.type, item.extensions)
                    }
                    showUploadList={false}
                    accept={item.extensions.map((ext) => `.${ext}`).join(",")}
                  >
                    <Button
                      type="primary"
                      icon={<UploadOutlined />}
                      className="import-file-btn"
                      style={{
                        backgroundColor: "#00ff00",
                        borderColor: "#00ff00",
                        color: "#000000",
                        fontWeight: "bold",
                        borderRadius: "6px",
                        height: "36px",
                        width: "120px",
                        padding: "0 16px",
                        transition: "all 0.3s ease",
                        cursor: "pointer",
                      }}
                      onMouseEnter={(e) => {
                        const btn = e.currentTarget;
                        btn.style.backgroundColor = "#00cc00";
                        btn.style.borderColor = "#00cc00";
                        btn.style.transform = "translateY(-1px)";
                        btn.style.boxShadow = "0 4px 8px rgba(0,204,0,0.3)";
                      }}
                      onMouseLeave={(e) => {
                        const btn = e.currentTarget;
                        btn.style.backgroundColor = "#00ff00";
                        btn.style.borderColor = "#00ff00";
                        btn.style.transform = "translateY(0)";
                        btn.style.boxShadow = "none";
                      }}
                    >
                      Import File
                    </Button>
                  </Upload>
                )}

                <Text
                  style={{
                    color: uploadedFiles[item.type] ? "#52c41a" : "#8c8c8c",
                    fontSize: "14px",
                    minWidth: "100px",
                    fontWeight: uploadedFiles[item.type] ? "bold" : "normal",
                  }}
                >
                  {uploadedFiles[item.type]
                    ? uploadedFiles[item.type].name
                    : item.filename}
                </Text>
              </Space>
            </Col>
          </Row>
        ))}
      </Space>

      <div style={{ marginTop: "32px", textAlign: "center" }}>
        <Text type="secondary" style={{ fontSize: "14px" }}>
          Upload your certification files to configure IPSec authentication
        </Text>
        {!allFilesUploaded && (
          <div style={{ marginTop: "8px" }}>
            <Text type="warning" style={{ fontSize: "12px" }}>
              Please upload all 3 files to proceed to the next step
            </Text>
          </div>
        )}
      </div>
    </div>
  );
};

export default IpSecCertification;
