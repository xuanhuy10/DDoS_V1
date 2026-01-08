import React, { useEffect, useState } from 'react';
import { getAttackList } from '../api/NotificationApiTesting';

const AttackEvents = () => {
  const [data, setData] = useState([]);
  const [initLoading, setInitLoading] = useState(true);
  const [loading, setLoading] = useState(false);
  const [hasMore, setHasMore] = useState(true);
  const [error, setError] = useState(null);

  const fetchData = async (offset = 0) => {
    try {
      setError(null);
      const response = await getAttackList(offset);
      const newData = offset === 0 ? response.data : data.concat(response.data);
      setData(newData);
      setHasMore(response.data.length > 0);
    } catch (err) {
      setError('Failed to fetch data. Please try again later. ' + err);
    } finally {
      setInitLoading(false);
      setLoading(false);
    }
  };

  useEffect(() => {
    // Initial load with offset 0
    fetchData(0);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const onLoadMore = async () => {
    setLoading(true);
    await fetchData(data.length);
    // Optionally force a reflow if your layout needs it.
    window.dispatchEvent(new Event('resize'));
  };

  return (
    <div>
      <h2>Attack Events</h2>
      {error && <p style={{ color: 'red' }}>{error}</p>}
      {initLoading ? (
        <p>Loading...</p>
      ) : (
        <>
          <pre>{JSON.stringify(data, null, 2)}</pre>
          {hasMore && !loading && (
            <button onClick={onLoadMore}>Load More</button>
          )}
          {loading && <p>Loading more...</p>}
        </>
      )}
    </div>
  );
};

export default AttackEvents;
