import React, { useEffect, useState } from "react";
import {
  Form,
  Col,
  Row,
  Statistic,
  Slider,
  InputNumber,
  Card,
  Switch,
  Button,
  Modal,
  message,
  Flex,
  Spin,
} from "antd";
import { CheckOutlined, ReloadOutlined } from "@ant-design/icons";

import { byteFormatter, formatDate } from "@/lib/formatter";
import {
  getDeviceDiskUsage,
  getDeviceDiskSetting,
  updateDeviceDiskSetting,
  updateDeviceSettings,
} from "@/features/api/DeviceSettings";
import isEqual from "lodash/isEqual";

const DiskStatistic = ({ onActivitySettingsSaved }) => {
  const [diskInfo, setDiskInfo] = useState(null);
  const [logForm] = Form.useForm();
  const [activityForm] = Form.useForm();
  const [inputValue, setInputValue] = useState(1);
  const [initialLogValues, setInitialLogValues] = useState({});
  const [initialActivityValues, setInitialActivityValues] = useState({});
  const [logFormChanged, setLogFormChanged] = useState(false);
  const [activityFormChanged, setActivityFormChanged] = useState(false);
  const [customMarks, setCustomMarks] = useState({});
  const [reloading, setReloading] = useState(false);

  const fetchDiskInfo = async () => {
    try {
      const response = await getDeviceDiskUsage();
      setDiskInfo(response.data);
    } catch (error) {
      console.error("Failed to fetch disk statistic:", error);
    }
  };

  const fetchSettings = async () => {
    try {
      const response = await getDeviceDiskSetting();
      const logValues = {
        DeviceAutoDeleteLogThreshold: Number(
          response.data.DeviceAutoDeleteLogThreshold
        ),
        DeviceAutoDeleteLogEnable: Boolean(
          response.data.DeviceAutoDeleteLogEnable
        ),
        DeviceAutoRotationLogs: Boolean(response.data.DeviceAutoRotationLogs),
      };
      const activityValues = {
        DeviceAutoDeleteActivity: Boolean(
          response.data.DeviceAutoDeleteActivity
        ),
        DeviceAutoDeleteActivityInterval: Number(
          response.data.DeviceAutoDeleteActivityInterval
        ),
      };

      logForm.setFieldsValue(logValues);
      activityForm.setFieldsValue(activityValues);
      setInitialLogValues(logValues);
      setInitialActivityValues(activityValues);
      setInputValue(logValues.DeviceAutoDeleteLogThreshold);
      setLogFormChanged(false);
      setActivityFormChanged(false);
    } catch (error) {
      console.error("Failed to fetch disk settings:", error);
    }
  };
  const handleReload = async () => {
    setReloading(true);
    await Promise.all([fetchDiskInfo(), fetchSettings()]);
    setReloading(false);
  };

  const handleSaveAndSubmit = (formType) => {
    let extraMessage = "";
    if (formType === "activity") {
      const auto = activityForm.getFieldValue("DeviceAutoDeleteActivity");
      const days = activityForm.getFieldValue(
        "DeviceAutoDeleteActivityInterval"
      );
      if (auto) {
        if (days) {
          const deleteBefore = new Date();
          deleteBefore.setDate(deleteBefore.getDate() - days);
          extraMessage = `<br/>Activity logs will be deleted before <b>${formatDate(
            deleteBefore
          )}</b>.`;
        }
      } else {
        extraMessage = `<br/>Auto clean activity is <b>disabled</b>. No activity logs will be deleted automatically.`;
      }
    }
    if (formType === "log") {
      const auto = logForm.getFieldValue("DeviceAutoDeleteLogEnable");
      const threshold = logForm.getFieldValue("DeviceAutoDeleteLogThreshold");
      if (auto) {
        extraMessage = `<br/>Logs will be deleted when disk usage exceeds <b>${threshold}%</b>.`;
      } else {
        extraMessage = `<br/>Auto clean logs is <b>disabled</b>. No logs will be deleted automatically.`;
      }
    }

    Modal.confirm({
      title: "Confirm",
      content: (
        <span>
          Are you sure you want to save the {formType} settings?
          <span dangerouslySetInnerHTML={{ __html: extraMessage }} />
        </span>
      ),
      onOk: async () => {
        if (formType === "log") {
          // Lưu mốc mới
          console.log("Saving mark:", inputValue);
          setCustomMarks({
            [inputValue]: {
              style: { color: getColor(inputValue) },
              label: <strong>{inputValue}%</strong>,
            },
          });
        }

        // Lưu cài đặt
        const values =
          formType === "log"
            ? logForm.getFieldsValue(true)
            : activityForm.getFieldsValue(true);

        try {
          if (formType === "log") {
            await updateDeviceDiskSetting(values); // Chỉ gọi khi Save log
          } else if (formType === "activity") {
            await updateDeviceSettings(values); // Chỉ gọi khi Save activity
          }
          message.success(
            `${
              formType.charAt(0).toUpperCase() + formType.slice(1)
            } settings saved successfully!`
          );
          fetchDiskInfo();
          fetchSettings();

          if (formType === "log") {
            setLogFormChanged(false);
          } else if (formType === "activity") {
            setActivityFormChanged(false);
            onActivitySettingsSaved();
          }
        } catch (error) {
          message.error(
            error.response?.data?.message ||
              `Failed to save ${formType} settings, please try again later.`
          );
        }
      },
    });
  };

  useEffect(() => {
    fetchDiskInfo();
    fetchSettings();
  }, []);

  const usedPercentage = diskInfo
    ? ((diskInfo.storageInfo.used / diskInfo.storageInfo.total) * 100).toFixed(
        2
      )
    : 0;

  const getColor = (percentage) => {
    if (percentage > 75) return "#f50";
    if (percentage > 50) return "#fa8c16";
    return "#52c41a";
  };

  const marks = {
    0: "0%",
    100: { label: <strong>100%</strong> },
    ...customMarks, // Kết hợp các mốc tùy chỉnh
  };

  return (
    <Flex vertical style={{ width: "100%" }} gap={8}>
      <div
        style={{
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
        }}
      >
        <h2 className="disk-title">Log Usage</h2>
        <Spin spinning={reloading}>
          <ReloadOutlined
            style={{ fontSize: 22, cursor: "pointer" }}
            onClick={handleReload}
            // onClick={() => {
            //   fetchDiskInfo();
            //   fetchSettings();
            // }}
          />
        </Spin>
      </div>
      <Spin spinning={reloading}>
        <Row gutter={16} style={{ marginBottom: "1rem" }}>
          <Col span={5}>
            <Card>
              <Statistic
                title="Total Capacity"
                value={byteFormatter(diskInfo?.storageInfo.total)}
                valueStyle={{ color: "#52c41a", fontWeight: 600 }}
                titleStyle={{ color: "#52c41a" }}
              />
            </Card>
          </Col>
          <Col span={5}>
            <Card>
              <Statistic
                title="Used"
                value={byteFormatter(diskInfo?.storageInfo.used)}
                valueStyle={{ color: "#fa8c16", fontWeight: 600 }}
                titleStyle={{ color: "#fa8c16" }}
              />
            </Card>
          </Col>
          <Col span={5}>
            <Card>
              <Statistic
                title="Free"
                value={byteFormatter(diskInfo?.storageInfo.free)}
                valueStyle={{ color: "#f50", fontWeight: 600 }}
                titleStyle={{ color: "#f50" }}
              />
            </Card>
          </Col>
          <Col span={4}>
            <Card>
              <Statistic
                title="Total Logs"
                value={diskInfo?.logsCount + " Logs"}
                valueStyle={{ color: "#722ed1", fontWeight: 600 }}
                titleStyle={{ color: "#722ed1" }}
              />
            </Card>
          </Col>
          <Col span={5}>
            <Card>
              <Statistic
                title="Total Logs Size"
                value={byteFormatter(diskInfo?.totalLogSize)}
                valueStyle={{ color: "#1890ff", fontWeight: 600 }}
                titleStyle={{ color: "#1890ff" }}
              />
            </Card>
          </Col>
        </Row>
      </Spin>
      <h2 className="disk-title">Activities & Logs Settings</h2>
      <Row gutter={16}>
        <Col span={12}>
          <Form
            colon={false}
            form={logForm}
            initialValues={initialLogValues}
            onValuesChange={(_, allValues) => {
              setInputValue(allValues.DeviceAutoDeleteLogThreshold);
              const hasChanges = !isEqual(allValues, initialLogValues);
              setLogFormChanged(hasChanges);
            }}
          >
            <Card
              title="Network Logs Retention"
              extra={
                <Button
                  type="primary"
                  icon={<CheckOutlined />}
                  onClick={() => handleSaveAndSubmit("log")}
                  disabled={!logFormChanged}
                >
                  Save
                </Button>
              }
            >
              <Form.Item name="DeviceAutoDeleteLogThreshold" noStyle>
                <Slider
                  min={1}
                  max={100}
                  marks={marks}
                  tooltip={{ formatter: (value) => `${value}%` }}
                  value={inputValue}
                  onChange={(value) => {
                    setInputValue(value);
                    logForm.setFieldValue(
                      "DeviceAutoDeleteLogThreshold",
                      value
                    );
                  }}
                />
              </Form.Item>

              <Form.Item
                label="Logs usage limit"
                name="DeviceAutoDeleteLogThreshold"
              >
                <InputNumber
                  min={1}
                  max={100}
                  addonAfter="%"
                  style={{ width: "30%" }}
                />
              </Form.Item>

              <Form.Item
                label="Auto clean logs"
                name="DeviceAutoDeleteLogEnable"
                valuePropName="checked"
              >
                <Switch checkedChildren="Auto" unCheckedChildren="Manual" />
              </Form.Item>

              <Form.Item
                label="Logs file rotation"
                name="DeviceAutoRotationLogs"
                valuePropName="checked"
              >
                <Switch checkedChildren="Auto" unCheckedChildren="Manual" />
              </Form.Item>
            </Card>
          </Form>
        </Col>

        <Col span={12}>
          <Form
            colon={false}
            form={activityForm}
            initialValues={initialActivityValues}
            onValuesChange={(_, allValues) => {
              const hasChanges = !isEqual(allValues, initialActivityValues);
              setActivityFormChanged(hasChanges);
            }}
          >
            <Card
              title="Activity Settings"
              extra={
                <Button
                  type="primary"
                  icon={<CheckOutlined />}
                  onClick={() => handleSaveAndSubmit("activity")}
                  disabled={!activityFormChanged}
                >
                  Save
                </Button>
              }
            >
              <Form.Item
                label="Auto clean activity"
                name="DeviceAutoDeleteActivity"
                valuePropName="checked"
              >
                <Switch checkedChildren="Auto" unCheckedChildren="Manual" />
              </Form.Item>

              <Form.Item
                label="Clean activity older than"
                name="DeviceAutoDeleteActivityInterval"
              >
                <InputNumber
                  min={1}
                  max={365}
                  addonAfter="Days"
                  style={{ width: "40%" }}
                />
              </Form.Item>
            </Card>
          </Form>
        </Col>
      </Row>
    </Flex>
  );
};

export default DiskStatistic;
