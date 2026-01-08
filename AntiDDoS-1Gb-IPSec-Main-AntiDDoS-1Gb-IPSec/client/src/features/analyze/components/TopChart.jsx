import * as echarts from "echarts";
import { memo, useCallback } from "react";
import ReactEcharts from "echarts-for-react";
import { Tooltip } from "antd";
import { bitFormatter, timeFormatter } from "@/lib/formatter";
import { InfoCircleOutlined } from "@ant-design/icons";
import dayjs from "dayjs";

const ChartInfo = memo(({ title }) => {
  const handleMouseEnter = useCallback((e) => {
    e.target.style.opacity = "1";
  }, []);

  const handleMouseLeave = useCallback((e) => {
    e.target.style.opacity = "0.7";
  }, []);

  return (
    <div style={{ position: "absolute", top: 5, right: 8, zIndex: 10 }}>
      <Tooltip title={title} placement="topRight">
        <InfoCircleOutlined
          style={{
            fontSize: "16px",
            color: "#666",
            cursor: "pointer",
            opacity: 0.7,
            transition: "opacity 0.2s",
          }}
          onMouseEnter={handleMouseEnter}
          onMouseLeave={handleMouseLeave}
        />
      </Tooltip>
    </div>
  );
});

ChartInfo.displayName = "ChartInfo";

const TopChart = ({ data, title = "Top Chart IP Address", ipColors = {} }) => {
  // Extract time labels from the first row, excluding the header
  const timeLabels =
    Array.isArray(data) && data[0] ? data[0].slice(1) : getDefaultTimeLabels();

  const hasData =
    data.length > 1 &&
    data.slice(1).some((row) => row.slice(1).some((value) => value > 0));

  const top_option = {
    tooltip: {
      trigger: "axis",
      axisPointer: {
        type: "cross",
        link: { xAxisIndex: "all" },
        label: {
          backgroundColor: "#6a7985",
          formatter: (params) => {
            if (params.axisDimension === "y") {
              return bitFormatter(params.value);
            }
            if (params.axisDimension === "x") {
              return timeFormatter(params.value);
            }
            return params.value;
          },
        },
      },
      formatter: (params) => {
        const time = timeFormatter(params[0]?.axisValue || "");
        let tooltipContent = `<div style="font-size: 14px; font-weight: bold; margin-bottom: 5px; padding: 5px;">${time}</div>`;

        // Filter IPs with non-zero values using param.value
        const nonZeroParams = params.filter((param) => {
          const value =
            param.value !== undefined && typeof param.value !== "string"
              ? param.value
              : 0;
          return value > 0;
        });

        if (!params.length || nonZeroParams.length === 0) {
          tooltipContent += `<div style="color: #888; font-style: italic;">No active IPs at this point</div>`;
        } else {
          nonZeroParams.forEach((param) => {
            const ip = param.seriesName;
            const value = param.value;
            const color = ipColors[ip] || "#888";
            tooltipContent += `
          <div style="display: flex; align-items: center; margin: 2px 0;">
            <span style="display: inline-block; width: 10px; height: 10px; border-radius: 50%; background-color: ${color}; margin-right: 8px;"></span>
            <span style="color: ${color};">${ip}</span>: <strong style="margin-left: auto;">${bitFormatter(
              value
            )}</strong>
          </div>`;
          });
        }
        return tooltipContent;
      },
    },
    toolbox: {
      feature: {
        dataZoom: {
          yAxisIndex: "none",
        },
        restore: {},
      },
    },
    dataZoom: [
      {
        type: "inside",
        throttle: 50,
      },
    ],
    dataset: {
      source: hasData ? data : getEmptyIpList(),
    },
    grid: {
      left: "0.6%",
      right: "2%",
      bottom: "0.2%",
      top: "6%",
      containLabel: true,
    },
    xAxis: {
      type: "category",
      showGrid: false,
      boundaryGap: false,
      data: timeLabels.length > 0 ? timeLabels : getDefaultTimeLabels(), // Fallback to default labels if none
      axisLabel: {
        formatter: timeFormatter,
      },
    },
    yAxis: {
      type: "value",
      splitLine: {
        show: false,
      },
      axisLabel: {
        formatter: bitFormatter,
      },
    },
    series: hasData
      ? data.slice(1).map((item) => ({
          name: item[0],
          type: "line",
          symbol: "none",
          data: item.slice(1),
          lineStyle: {
            width: 1,
            color: ipColors[item[0]] || "#888",
          },
        }))
      : [
          {
            name: "No Data",
            type: "line",
            symbol: "none",
            data: Array(timeLabels.length).fill(0),
            lineStyle: { width: 0 }, // Hide line
            itemStyle: { opacity: 0 }, // Hide points
          },
        ],
    // Handle no data display
    noDataLoadingOption: {
      text: "No data",
      effect: "bubble",
      effectOption: {
        effect: {
          n: 0, // Disable animation
        },
      },
      textStyle: {
        fontSize: 16,
        color: "#888",
      },
    },
  };

  return (
    <>
      <ChartInfo title={title} />
      <ReactEcharts
        option={top_option}
        style={{ width: "70%", height: "100%" }}
      />
    </>
  );
};

const getDefaultTimeLabels = () => {
  const now = dayjs();
  const timestamps = getEmptyTimeRange(now.subtract(1, "week"), now).map(
    ([time]) => time
  );
  return timestamps.reverse();
};
const getEmptyTimeRange = (from, to) => {
  const timestamps = [];
  const start = from.unix();
  const end = to.unix();
  const step = Math.floor((end - start) / 10) || 1;
  for (let i = start; i <= end; i += step) {
    timestamps.push([dayjs.unix(i).format("YYYY-MM-DD HH:mm:ss"), 0]);
  }
  return timestamps;
};

const getEmptyIpList = () => {
  const timestamps = getEmptyTimeRange(
    dayjs().subtract(1, "week"),
    dayjs()
  ).map(([time]) => time);
  return [
    ["Source", ...timestamps],
    ["192.168.0.1", ...Array(timestamps.length).fill(0)],
    ["192.168.0.2", ...Array(timestamps.length).fill(0)],
  ];
};
export default TopChart;
