import React, { useEffect, useState } from "react";
import { Tabs, Card, Flex } from "antd";
import SmallAreaChart from "@/features/manager/components/SmallAreaChart";
import DiskChart from "@/features/manager/components/DiskChart";
import DiskPieChart from "@/features/manager/components/DiskPieChart";
import CpuBarChart from "@/features/manager/components/CpuBarChart";
import CpuPieChart from "@/features/manager/components/CpuPieChart";
import MemoryChart from "@/features/manager/components/MemoryChart";
import MemoryPieChart from "@/features/manager/components/MemoryPieChart";
import { getDataSystem } from "@/features/manager/api/Device";
import { ProductFilled } from "@ant-design/icons";

export default function DeviceManager() {
  const [systemData, setSystemData] = useState(null);
  const [loading, setLoading] = useState(false);

   const fetchData = async () => {
      try {
        setLoading(true);
        const data = await getDataSystem();
        console.log('data', data);
        const arrangedData = {
          systemUptime: formatUptime(data.systemUptime),
          totalMemory: parseFloat(data.performanceData.memory.totalMemory),
          nCPUs: {
            model: data.performanceData.cpu.model,
            cores: `${data.performanceData.cpu.cores} cores`,
            speed: data.performanceData.cpu.speed,
            cpuPercentage: `${data.performanceData.cpu.overallUsage}`,

          },
          cpuUsage: data.performanceData.cpu.utilization,
          diskUsage: data.performanceData.storageInfo,
          diskName: data.performanceData.name,
          diskType: data.performanceData.type,
          memoryUsage: {
            freeMemory: data.performanceData.memory.freeMemory,
            heapUsed: data.performanceData.memory.heapUsed,
            totalMemory: data.performanceData.memory.totalMemory,
            committedMemory: data.performanceData.memory.committed,
            cachedMemory: data.performanceData.memory.cached,
          },
        };
  
        setSystemData(arrangedData);
        console.log(arrangedData);
  
      } catch (error) {
        console.error("Failed to fetch system data:", error);
      } finally {
        setLoading(false);
      }
    };
    useEffect(() => {
      fetchData();
  
      const interval = setInterval(() => {
        fetchData();
      }, 1000);
  
      return () => clearInterval(interval);
    }, []);

    const items=[
        {
            key: "CPU",
            label:(
                <>
                <Card><Flex justify="space-between"><ProductFilled />CPU 12 % | 2,4 GHz </Flex></Card>
                </>
            )
        }
    ]

  return <Tabs tabPosition="left" items={items} />;
}
