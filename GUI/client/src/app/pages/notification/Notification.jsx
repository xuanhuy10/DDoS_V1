import React, { useState, useEffect } from "react";
import { Tabs, List, Checkbox, Typography, Spin, Card, Button, Popconfirm } from "antd";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
  faSyncAlt,
  faNetworkWired,
  faCogs,
  faShieldAlt,
  faRedo,
  faTrash,
} from "@fortawesome/free-solid-svg-icons";
import AttackEventList from "@/features/notification/components/AttackEventList";
import { getAllNotification } from "@/features/notification/api/notificationApi";
  
const { Text } = Typography;

export default function Notification() {
  const [notificationData, setNotificationData] = useState([]);
  const [loading, setLoading] = useState(false);
  const [refreshing, setRefreshing] = useState(false);
  const [activeTab, setActiveTab] = useState("Main");
  const [selectedItems, setSelectedItems] = useState(new Set());

  useEffect(() => {
    fetchData();
  }, []);

  const fetchData = async () => {
    try {
      setLoading(true);
      const data = await getAllNotification();
      // console.log("Fetched Data:", data);

      // Merge notifications from various parts of the API response
      const allNotifications = [
        ...(data?.notification?.noti_Info || []),
        ...(data?.notification?.anomalies || []),
        ...(data?.notification?.system || []),
        ...(data?.notification?.config || []),
      ];

      setNotificationData(Array.isArray(allNotifications) ? allNotifications : []);
      // console.log("allNotifcation", allNotifications);
    } catch (error) {
      console.error("Failed to fetch notifications:", error);
      setNotificationData([]);
    } finally {
      setLoading(false);
    }
  };

  const handleRefresh = async () => {
    setRefreshing(true);
    await fetchData();
    setTimeout(() => setRefreshing(false), 500);
  };

  // Mapping API notification types to display names.
  const categoryNames = {
    anomalies: "Network Anomalies",
    system: "System Activities",
    config: "Defense",
  };

  const getNotificationIcon = (category) => {
    switch (category) {
      case "anomalies":
        return <FontAwesomeIcon icon={faNetworkWired} />;
      case "system":
        return <FontAwesomeIcon icon={faCogs} />;
      case "config":
        return <FontAwesomeIcon icon={faShieldAlt} />;
      default:
        return <FontAwesomeIcon icon={faSyncAlt} />;
    }
  };

  // Filter notifications based on active tab.
  const filteredNotifications = notificationData.filter((notification) => {
    if (activeTab === "Main") return true;
    return categoryNames[notification.NotificationType] === activeTab;
  });

  // Use the correct property to filter anomalies.
  const anomaliesData = notificationData.filter(
    (notification) => notification.NotificationType === "anomalies"
  );

  const toggleSelection = (id) => {
    setSelectedItems((prev) => {
      const newSet = new Set(prev);
      newSet.has(id) ? newSet.delete(id) : newSet.add(id);
      return newSet;
    });
  };

  const toggleSelectAll = () => {
    setSelectedItems(
      selectedItems.size === filteredNotifications.length
        ? new Set()
        : new Set(filteredNotifications.map((item) => item.NotificationId))
    );
  };

  const markAsRead = (id) => {
    setNotificationData((prevData) =>
      prevData.map((item) =>
        item.NotificationId === id ? { ...item, IsRead: 1 } : item
      )
    );
  };

  const handleDelete = (ids) => {
    // console.log("Delete IDs:", ids);
  };

  // Render the notifications list.
  const renderNotificationList = () =>
    loading ? (
      <Spin size="large" />
    ) : (
      <Card>
        <div style={{ display: "flex", justifyContent: "space-between", marginBottom: 10 }}>
          <Checkbox
            onChange={toggleSelectAll}
            checked={selectedItems.size === filteredNotifications.length}
            indeterminate={
              selectedItems.size > 0 && selectedItems.size < filteredNotifications.length
            }
          >
            Select All
          </Checkbox>
          <Button
            type="primary"
            icon={<FontAwesomeIcon icon={faRedo} />}
            onClick={handleRefresh}
            loading={refreshing}
          >
            Refresh
          </Button>
        </div>
        <List
          dataSource={filteredNotifications}
          pagination={{ defaultCurrent: 1 }}
          renderItem={(item) => (
            <List.Item
              style={{
                display: "flex",
                alignItems: "center",
                backgroundColor: item.IsRead ? "whitesmoke" : "white",
                cursor: "pointer",
              }}
              onClick={() => markAsRead(item.NotificationId)}
            >
              <Checkbox
                checked={selectedItems.has(item.NotificationId)}
                onChange={() => toggleSelection(item.NotificationId)}
                style={{ marginRight: 12 }}
              />
              <span style={{ width: 30, textAlign: "center" }}>
                {getNotificationIcon(item.NotificationType)}
              </span>
              <Text strong style={{ minWidth: 140 }}>
                {categoryNames[item.NotificationType] || "Unknown"}
              </Text>
              <Text style={{ flex: 1, fontSize: "14px", textAlign: "center" }}>
                {item.Notificate}
              </Text>
              <Text type="secondary" style={{ fontSize: "14px" }}>
                User ID: {item.UserId}
              </Text>
              <Text
                type="secondary"
                style={{ minWidth: 150, fontSize: "14px", textAlign: "right" }}
              >
                {item.NotificationTime}
              </Text>
              <Popconfirm
                title="Delete this notification?"
                onConfirm={() => handleDelete([item.NotificationId])}
                okText="Yes"
                cancelText="No"
              >
                <Button type="link" danger icon={<FontAwesomeIcon icon={faTrash} />} />
              </Popconfirm>
            </List.Item>
          )}
        />
      </Card>
    );

  // Define tabs for notifications.
  const tabItems = [
    {
      key: "Main",
      label: (
        <>
          <FontAwesomeIcon icon={faSyncAlt} style={{ marginRight: 8 }} />
          Main
        </>
      ),
      children: renderNotificationList(),
    },
    {
      key: "Network Anomalies",
      label: (
        <>
          <FontAwesomeIcon icon={faNetworkWired} style={{ marginRight: 8 }} />
          Network Anomalie
        </>
      ),
      children: renderNotificationList(),
    },
    {
      key: "System Activities",
      label: (
        <>
          <FontAwesomeIcon icon={faCogs} style={{ marginRight: 8 }} />
          System Activities
        </>
      ),
      children: renderNotificationList(),
    },
    {
      key: "Defense",
      label: (
        <>
          <FontAwesomeIcon icon={faShieldAlt} style={{ marginRight: 8 }} />
          Defense
        </>
      ),
      children: renderNotificationList(),
    },
  ];
  // console.log('anomaliesData', anomaliesData)
  return (
    <>
      {/* Attack events section */}
      <div style={{ marginTop: "3vh",background:'white',padding:'5px 5px' }} >
        <AttackEventList />
        </div>
      {/* Notifications section */}
      <Card style={{ marginTop: "3vh" }}>
        <Tabs activeKey={activeTab} onChange={setActiveTab} items={tabItems} />
      </Card>
    </>
  );
}