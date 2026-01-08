import React, { useEffect, useState } from "react";
import { Card, Flex, Space, Table, Tag, message } from "antd";
import { Link } from "react-router-dom";

import { AlertFilled, SyncOutlined, ExportOutlined, IssuesCloseOutlined } from "@ant-design/icons";

import { getAllNetworkAnomalies} from "@/features/api/NetworkAnomalies";

//FIXME: DELETE

const TrafficLog = () => {
    const [loading, setLoading] = useState(false);
    const [anomalies, setAnomalies] = useState([]);

    const fetchAnomalies = async () => {
        try {
            setLoading(true);
            // const response = await getAnomalies();
            const response = await getAllNetworkAnomalies();
            setAnomalies(response.data);
        } catch (error) {
            message.error(error.response?.data?.message || "Failed to fetch anomalies. Please try again later.");
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchAnomalies();
    }
    , []);

    const columns = [
        {
            title: 'State',
            dataIndex: 'AnomaliesStatus',
            key: 'state',
            align: 'center',
            render: (AnomaliesStatus) => (
                AnomaliesStatus === 'active' ? <AlertFilled style={{ color: 'red' }} /> : <IssuesCloseOutlined style={{ color: 'green' }} />
            )
        },
        {
            title: 'Anomaly',
            dataIndex: 'Anomalies',
            key: 'attack',
        },
        {
            title: 'Target',
            dataIndex: 'AnomaliesTargetIp',
            key: 'target',
        },
        {
            title: 'Statistic',
            dataIndex: 'AnomaliesStats',
            key: 'stats',
        },
        {
            title: 'Start time',
            dataIndex: 'AnomaliesStart',
            key: 'start_time',
        },
        {
            title: 'End time',
            dataIndex: 'AnomaliesEnd',
            key: 'end_time',
            render: (text) => (
                text === null ? <Tag color="#f50">Ongoing</Tag> : text
            )
        },
    ];

    useEffect(() => {
        fetchAnomalies();
    }
    , []);


    return (
        <Card bordered={false} className="small-card">
            <Flex justify="space-between" align="middle">
                <span className="title">
                    Network anomalies
                </span>
                <Space>
                    <SyncOutlined onClick={fetchAnomalies} />
                    <Link to="/monitor">
                        <ExportOutlined className="icon" />
                    </Link>
                </Space>
            </Flex>
            <Table loading={loading} columns={columns} dataSource={anomalies} rowKey={(record) => record.AnomaliesId || record.key} />
        </Card>
    );
}

export default TrafficLog;