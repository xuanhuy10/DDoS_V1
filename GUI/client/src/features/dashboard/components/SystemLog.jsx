import React, { useEffect, useState, useMemo } from "react";
import { Card, Flex, message, Space, Table, Tag } from "antd";
import { SyncOutlined, ExportOutlined } from "@ant-design/icons";
import { Link } from "react-router-dom";

import { getAllDeviceLogs } from "@/features/api/DeviceLogs";

const SystemLog = () => {
    const [loading, setLoading] = useState(false);
    const [activities, setActivities] = useState([]);

    const [pagination, setPagination] = useState({
        current: 1,
        pageSize: 5,
        total: 0,
    });

    const fetchSystemLogs = async () => {
        try {
            setLoading(true);
            const response = await getAllDeviceLogs(); // API này trả toàn bộ logs
            const allLogs = response.data || [];
            setActivities(allLogs);
            setPagination(prev => ({
                ...prev,
                total: allLogs.length,
            }));
        } catch (error) {
            console.error(error);
            message.error(error.response?.data?.message || "Failed to fetch system logs. Please try again later.");
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchSystemLogs();
    }, []);

    // Slice dữ liệu theo trang hiện tại
    const paginatedData = useMemo(() => {
        const start = (pagination.current - 1) * pagination.pageSize;
        const end = start + pagination.pageSize;
        return activities.slice(start, end);
    }, [activities, pagination]);

    const columns = [
        {
            title: 'Time',
            dataIndex: 'LogActionTime',
            key: 'time',
        },
        {
            title: 'Activity',
            dataIndex: 'LogActionContent',
            key: 'activity',
            render: (text, record) => (
                <span>{record.Username ? record.Username + ": " : ""}{text}</span>
            )
        },
        {
            title: 'Status',
            dataIndex: 'LogActionResult',
            key: 'status',
            render: (text) => (
                text === 'Success' ? <Tag color="green">Success</Tag> : <Tag color="red">Failed</Tag>
            )
        },
    ];

    return (
        <Card bordered={false} className="small-card">
            <Flex justify="space-between" align="middle">
                <span className="title">System activities</span>
                <Space>
                    <SyncOutlined onClick={fetchSystemLogs} />
                    <Link to="/manager/logs">
                        <ExportOutlined className="icon" />
                    </Link>
                </Space>
            </Flex>
            <Table
                columns={columns}
                loading={loading}
                dataSource={paginatedData}
                rowKey={record => record.LogId || record.key}
                pagination={{
                    current: pagination.current,
                    pageSize: pagination.pageSize,
                    total: pagination.total,
                    showSizeChanger: true,
                    onChange: (page, pageSize) => {
                        setPagination(prev => ({ ...prev, current: page, pageSize }));
                    },
                }}
                expandable={{
                    expandedRowRender: record => (
                        <span style={{ margin: 0, width: 100, wordBreak: 'break-word' }}>{record.LogActionDetail}</span>
                    ),
                    rowExpandable: record => !!record.LogActionDetail,
                }}
            />
        </Card>
    );
};

export default SystemLog;
