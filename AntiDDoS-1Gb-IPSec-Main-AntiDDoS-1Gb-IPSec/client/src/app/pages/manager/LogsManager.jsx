import React, { useState, useCallback } from "react";
import { Tabs } from "antd";

import "@/features/manager/logs/styles/main.css";

import PageTitle from "@/components/common/PageTitle";
import LogList from "@/features/manager/logs/components/LogList";
import ActivityList from "@/features/manager/logs/components/ActivityList";
import DiskStatistic from "@/features/manager/logs/components/DiskStatistic";

export default function LogsManager() {
  const [refreshKey, setRefreshKey] = useState(0);

  // Auto refresh
  const handleActivitySettingsSaved = useCallback(() => {
    setRefreshKey((prev) => prev + 1);
  }, []);

  return (
    <>
      <PageTitle
        title="Activities & Logs Manager"
        description="View and manage your device activities and setting logs"
      />
      <Tabs
        tabPosition="left"
        className="log-container"
        items={[
          {
            key: "disk",
            label: "Log Settings",
            children: (
              <DiskStatistic
                onActivitySettingsSaved={handleActivitySettingsSaved}
              />
            ),
          },
          {
            key: "traffic",
            label: "Traffic Log",
            children: <LogList type="traffic" refreshKey={refreshKey} />,
          },
          {
            key: "threat",
            label: "Threat Log",
            children: <LogList type="threat" refreshKey={refreshKey} />,
          },
          {
            key: "config",
            label: "Config Activity",
            children: <ActivityList type="Config" refreshKey={refreshKey} />,
          },
          {
            key: "system",
            label: "System Activity",
            children: <ActivityList type="System" refreshKey={refreshKey} />,
          },
        ]}
      ></Tabs>
    </>
  );
}
