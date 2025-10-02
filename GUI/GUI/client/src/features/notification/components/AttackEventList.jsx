import { useState, useEffect } from 'react';
import { Typography, Select, Row, Col, Card, Spin, Button } from 'antd';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import {
  faClock,
  faCheckCircle,
  faExclamationTriangle,
  faNetworkWired,
  faChartLine,
  faQuestionCircle,
} from '@fortawesome/free-solid-svg-icons';
import dayjs from 'dayjs';
import customParseFormat from 'dayjs/plugin/customParseFormat';
// import { getAttackList } from '../api/NotificationApiTesting';

dayjs.extend(customParseFormat);

const { Text } = Typography;
const { Option } = Select;

const AttackEventList = () => {
  // Fetched data state
  const [data, setData] = useState([]);
  const [initLoading, setInitLoading] = useState(true);
  const [loading, setLoading] = useState(false);
  const [hasMore, setHasMore] = useState(true);
  const [offset, setOffset] = useState(0);
  const pageSize = 8; // Items per fetch

  // Filter states
  const [selectedAttack, setSelectedAttack] = useState('all');
  const [selectedTimeRange, setSelectedTimeRange] = useState('all');
  const [selectedState, setSelectedState] = useState('all');
  const [error, setError] = useState(null);

  // Fetch data from API with the given offset
  const fetchData = async (currentOffset = 0) => {
    try {
      setError(null);
      setLoading(true);
      const response = await getAttackList(currentOffset);
      // Assuming response.data is an array of anomalies
      const newData = currentOffset === 0 ? response.data : data.concat(response.data);
      setData(newData);
      // If the API returns fewer items than the pageSize, we assume it's the end of the data
      if (response.data.length < pageSize) {
        setHasMore(false);
      } else {
        setHasMore(true);
      }
    } catch (err) {
      setError('Failed to fetch data. Please try again later. ' + err.message);
      setHasMore(false);
    } finally {
      setInitLoading(false);
      setLoading(false);
    }
  };

  // Initial fetch
  useEffect(() => {
    fetchData(0);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  // Increase offset and fetch next chunk when Load More is clicked
  const handleLoadMore = () => {
    const newOffset = offset + pageSize;
    setOffset(newOffset);
    fetchData(newOffset);
    window.dispatchEvent(new Event('resize'));
  };

  // Filtering function applied on the fetched data
  const getFilteredAnomalies = () => {
    let filtered = [...data];

    // Filter by attack type
    if (selectedAttack !== 'all') {
      if (selectedAttack === 'unknown') {
        filtered = filtered.filter((item) => !item.Attack || item.Attack.trim() === '');
      } else {
        filtered = filtered.filter((item) => item.Attack === selectedAttack);
      }
    }

    // Filter by time range
    const now = dayjs();
    switch (selectedTimeRange) {
      case 'today':
        filtered = filtered.filter((item) =>
          dayjs(item.StartTime, 'YY/MM/DD HH:mm:ss').isSame(now, 'day')
        );
        break;
      case 'week':
        filtered = filtered.filter((item) =>
          dayjs(item.StartTime, 'YY/MM/DD HH:mm:ss').isSame(now, 'week')
        );
        break;
      case 'month':
        filtered = filtered.filter((item) =>
          dayjs(item.StartTime, 'YY/MM/DD HH:mm:ss').isSame(now, 'month')
        );
        break;
      default:
        break;
    }

    // Filter by state
    if (selectedState !== 'all') {
      filtered = filtered.filter((item) =>
        selectedState === 'alert' ? item.State === 1 : item.State === 0
      );
    }
    return filtered;
  };

  // Helper to display state icon and text
  const getStateDisplay = (state) => {
    return state === 1 ? (
      <>
        <FontAwesomeIcon icon={faExclamationTriangle} style={{ color: 'orange', marginRight: 8 }} />
        <Text strong style={{ color: 'orange' }}>Alert</Text>
      </>
    ) : (
      <>
        <FontAwesomeIcon icon={faCheckCircle} style={{ color: 'green', marginRight: 8 }} />
        <Text strong style={{ color: 'green' }}>Finish</Text>
      </>
    );
  };

  // Get filtered data
  const filteredAnomalies = getFilteredAnomalies();

  return (
    <div style={{ padding: '16px' }}>
      <Row justify="space-between" align="middle" style={{ marginBottom: '16px' }}>
        <Col>
          <Text strong style={{ fontSize: '24px' }}>Attack Events</Text>
        </Col>
      </Row>

      {/* Filters */}
      <Row gutter={16} style={{ marginBottom: '16px' }}>
        <Col>
          <Select
            defaultValue="all"
            style={{ width: 200 }}
            onChange={(value) => setSelectedAttack(value)}
          >
            <Option value="all">All Attack Types</Option>
            <Option value="SYN Flood">SYN Flood</Option>
            <Option value="ICMP Flood">ICMP Flood</Option>
            <Option value="UDP Flood">UDP Flood</Option>
            <Option value="Land Attack">Land Attack</Option>
            <Option value="TCP Fragmentation">TCP Fragmentation</Option>
            <Option value="UDP Fragmentation">UDP Fragmentation</Option>
            <Option value="HTTP Flood">HTTP Flood</Option>
            <Option value="DNS Flood">DNS Flood</Option>
            <Option value="IPSec IKE Flood">IPSec IKE Flood</Option>
            <Option value="unknown">Unknown</Option>
          </Select>
        </Col>
        <Col>
          <Select
            defaultValue="all"
            style={{ width: 200 }}
            onChange={(value) => setSelectedTimeRange(value)}
          >
            <Option value="all">All Time</Option>
            <Option value="today">Today</Option>
            <Option value="week">This Week</Option>
            <Option value="month">This Month</Option>
          </Select>
        </Col>
        <Col>
          <Select
            defaultValue="all"
            style={{ width: 200 }}
            onChange={(value) => setSelectedState(value)}
          >
            <Option value="all">All States</Option>
            <Option value="alert">Alert</Option>
            <Option value="finish">Finish</Option>
          </Select>
        </Col>
      </Row>

      {/* Display anomalies */}
      {loading && offset === 0 ? (
        <Spin size="large" style={{ display: 'block', textAlign: 'center' }} />
      ) : filteredAnomalies.length > 0 ? (
        <>
          <Row gutter={16}>
            {filteredAnomalies.map((item) => (
              <Col key={item.Id} span={6} style={{ marginBottom: '16px' }}>
                <Card
                  title={
                    <Text strong>
                      <FontAwesomeIcon
                        icon={item.Attack ? faNetworkWired : faQuestionCircle}
                        style={{ marginRight: 8 }}
                      />
                      {item.Attack || 'Unknown Attack'}
                    </Text>
                  }
                  style={{ width: '100%' }}
                >
                  <p>
                    <FontAwesomeIcon icon={faClock} style={{ marginRight: 8 }} />
                    <Text strong>Start Time:</Text> {item.StartTime}
                  </p>
                  <p>
                    <FontAwesomeIcon icon={faClock} style={{ marginRight: 8 }} />
                    <Text strong>End Time:</Text> {item.EndTime || 'N/A'}
                  </p>
                  <p>
                    <FontAwesomeIcon icon={faChartLine} style={{ marginRight: 8 }} />
                    <Text strong>Initial Rate:</Text> {item.Stats?.split(',')[0] || 'N/A'}
                  </p>
                  <p>
                    <FontAwesomeIcon icon={faNetworkWired} style={{ marginRight: 8 }} />
                    <Text strong>IP:</Text> {item.Target}
                  </p>
                  <p>{getStateDisplay(item.State)}</p>
                </Card>
              </Col>
            ))}
          </Row>
          {hasMore ? (
            <Row justify="center" style={{ marginTop: '16px' }}>
              <Button type="primary" onClick={handleLoadMore} loading={loading}>
                Load More
              </Button>
            </Row>
          ) : (
            <Row justify="center" style={{ marginTop: '16px' }}>
              <Text type="secondary">No more data to load.</Text>
            </Row>
          )}
        </>
      ) : (
        <Text type="secondary">No anomalies found.</Text>
      )}
      {error && <Text type="danger">{error}</Text>}
    </div>
  );
};

export default AttackEventList;
