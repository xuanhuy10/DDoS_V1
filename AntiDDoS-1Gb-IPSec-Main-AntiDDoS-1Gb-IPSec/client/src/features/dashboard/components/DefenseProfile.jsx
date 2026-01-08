import React, { useEffect, useState } from "react";
import { Card, Flex, Descriptions, Space, message } from "antd";
import { Link } from "react-router-dom";
import {
  ProfileOutlined,
  SettingOutlined,
  SyncOutlined,
} from "@ant-design/icons";

import "@/features/dashboard/styles/main.css";

import { profileTimeFormatter } from "@/lib/formatter";

// import { getActiveProfile } from "@/features/defense/defenseprofile/api";
import { getAllActiveDefenseProfiles } from "@/features/api/DefenseProfiles";
import { getLoggedInUser } from "@/features/api/Users";

const DefenseProfile = () => {
  const [loading, setLoading] = useState(false);
  const [activeProfiles, setActiveProfiles] = useState([]);
  const [userData, setUserData] = useState(null);

  const fetchActiveProfile = async () => {
    try {
      setLoading(true);
      const profile = await getAllActiveDefenseProfiles();
      const response = await getLoggedInUser();
      setUserData(response.data);
      setActiveProfiles(profile.data);
    } catch (error) {
      message.error(
        error.response?.data?.message ||
          "Failed to fetch active profile, please try again later."
      );
      console.error(error);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchActiveProfile();
  }, []);

  const generateItems = (profiles) => {
    if (!profiles || profiles.length === 0) return [];
    return profiles
      .map((profile, index) => [
        {
          key: `name-${index}`,
          label: <ProfileOutlined />,
          labelStyle: { display: "none" },
          children: (
            <span className="title">{profile.DefenseProfileName || "N/A"}</span>
          ),
          span: 3,
        },
        {
          key: `user-${index}`,
          label: "User",
          children: userData.UserFullName || "N/A",
          span: 3,
        },
        {
          key: `time-${index}`,
          label: "Active Time",
          children:
            profileTimeFormatter(profile.DefenseProfileUsingTime) || "N/A",
          span: 3,
        },
      ])
      .flat();
  };
  const items = generateItems(activeProfiles);

  return (
    <Card bordered={false} className="small-card" loading={loading}>
      <Flex justify="space-between" align="middle">
        <span className="title">Active defense profile</span>
        <Space>
          <SyncOutlined onClick={fetchActiveProfile} />
          <Link to="/defense/interface/defense-profile-list">
            <SettingOutlined className="icon" />
          </Link>
        </Space>
      </Flex>
      <Descriptions bordered size="small" items={items} />
    </Card>
  );
};

export default DefenseProfile;
