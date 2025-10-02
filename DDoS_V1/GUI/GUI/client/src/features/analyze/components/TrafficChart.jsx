import {
  useEffect,
  useState,
  useRef,
  useImperativeHandle,
  forwardRef,
  memo,
  useCallback,
} from "react";
import * as echarts from "echarts";
import ReactEcharts from "echarts-for-react";
import { bitFormatter, timeFormatter } from "@/lib/formatter";
import { Tooltip } from "antd";
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

// Helper function to generate default empty data
const getDefaultEmptyData = (columnCount = 10) => {
  const now = dayjs();
  const timestamps = Array.from({ length: columnCount }).map((_, i) =>
    now.subtract(5 * (4 - i), "hour").format("YYYY-MM-DD HH:mm:ss")
  );
  return [
    ["Type", ...timestamps],
    ["Normal", ...Array(timestamps.length).fill(0)],
    ["Attack", ...Array(timestamps.length).fill(0)],
  ];
};

const TrafficChart = forwardRef(
  ({ data = getDefaultEmptyData(), title = "Traffic Chart" }, ref) => {
    const chartRef = useRef(null);
    const [zoomState, setZoomState] = useState({ start: 85, end: 100 });

    // Store chart instance in ref to track disposal
    const chartInstanceRef = useRef(null);

    useImperativeHandle(ref, () => ({
      getCanvasElement: () => {
        const instance = chartRef.current?.getEchartsInstance();
        if (!instance || instance.isDisposed()) return null;
        return instance.getDom().querySelector("canvas");
      },
      resetZoom: () => {
        const instance = chartRef.current?.getEchartsInstance();
        if (instance && !instance.isDisposed()) {
          instance.dispatchAction({
            type: "dataZoom",
            start: 0,
            end: 100,
          });
        }
      },
    }));

    useEffect(() => {
      const instance = chartRef.current?.getEchartsInstance();
      if (instance && !instance.isDisposed()) {
        chartInstanceRef.current = instance;
        const handleDataZoom = (params) => {
          const { start, end } = params.batch ? params.batch[0] : params;
          setZoomState({ start, end });
        };
        instance.on("dataZoom", handleDataZoom);
        return () => {
          if (instance && !instance.isDisposed()) {
            instance.off("dataZoom", handleDataZoom);
          }
          chartInstanceRef.current = null;
        };
      }
      return () => {
        chartInstanceRef.current = null;
      };
    }, []);

    useEffect(() => {
      const instance = chartRef.current?.getEchartsInstance();
      if (instance && !instance.isDisposed()) {
        const handleLegendToggle = (params) => {
          const toggled = params.selected;
          const isNormalVisible = toggled["Normal"] !== false;
          const isAttackVisible = toggled["Attack"] !== false;
          if (!isNormalVisible && !isAttackVisible) {
            instance.dispatchAction({
              type: "legendToggleSelect",
              name: "Normal",
            });
            instance.dispatchAction({
              type: "legendToggleSelect",
              name: "Attack",
            });
          }
        };
        instance.on("legendselectchanged", handleLegendToggle);
        return () => {
          if (instance && !instance.isDisposed()) {
            instance.off("legendselectchanged", handleLegendToggle);
          }
        };
      }
    }, []);

    // Ensure data is valid
    const validData =
      Array.isArray(data) &&
      data.length > 0 &&
      Array.isArray(data[0]) &&
      data[0].length > 0
        ? data
        : getDefaultEmptyData();

    const hasData =
      validData.length > 1 &&
      validData.slice(1).some((row) => row[1] > 0 || row[2] > 0);

    const trend_option = {
      tooltip: {
        trigger: "axis",
        axisPointer: {
          type: "cross",
          animation: false,
          label: {
            backgroundColor: "#f2ebeb",
            borderColor: "#aaa",
            borderWidth: 1,
            shadowBlur: 0,
            shadowOffsetX: 0,
            shadowOffsetY: 0,
            color: "#222",
            formatter: (params) => {
              if (params.axisDimension === "x") {
                return timeFormatter(params.value);
              }
              if (params.axisDimension === "y") {
                return bitFormatter(params.value);
              }
              return params.value;
            },
          },
        },
        formatter: (params) => {
          if (!params || !Array.isArray(params) || params.length === 0)
            return "";
          const timestamp = params[0]?.axisValue;
          if (!timestamp) return "";
          const formattedTime = timeFormatter(timestamp);
          let result = `${formattedTime}<br/>`;
          params.forEach((param) => {
            const yValue = param.value?.[param.seriesIndex + 1] ?? 0;
            if (yValue !== 0) {
              result += `
              <div style="display:flex; justify-content:space-between; min-width:150px; max-width:100%; gap:10px;">
                <span style="display:flex; align-items:center; white-space:nowrap;">
                  <span style="width:10px; height:10px; border-radius:50%; background-color:${
                    param.color
                  }; margin-right:5px;"></span>
                  ${param.seriesName}:
                </span>
                <span style="white-space:nowrap; font-weight: bold;">${bitFormatter(
                  yValue
                )}</span>
              </div>`;
            }
          });
          return result || "No data";
        },
      },
      color: ["#0143DD", "#DD0101"],
      legend: {
        data: ["Normal", "Attack"],
        textStyle: { fontSize: 12 },
        selectedMode: true,
        icon: "roundRect",
        itemWidth: 24,
        itemHeight: 10,
        top: 0,
      },
      grid: {
        left: "0.5%",
        right: "0.5%",
        top: "10%",
        bottom: "5%",
        containLabel: true,
      },
      dataset: {
        source: hasData
          ? validData
          : getDefaultEmptyData(validData[0]?.length - 1 || 5),
      },
      xAxis: {
        type: "category",
        boundaryGap: false,
        axisLabel: {
          formatter: timeFormatter,
          interval: "auto",
          rotate: 0,
          showMaxLabel: true,
          fontWeight: "bold",
        },
        axisLine: { lineStyle: { color: "#888" } },
        axisTick: { alignWithLabel: true },
      },
      yAxis: {
        type: "value",
        min: 0,
        axisLabel: { formatter: bitFormatter },
        splitLine: {
          show: true,
          lineStyle: { type: "dashed", color: "#eee" },
        },
      },
      dataZoom: [
        {
          type: "inside",
          start: zoomState.start,
          end: zoomState.end,
          throttle: 50,
          minSpan: 1,
        },
      ],
      series: hasData
        ? [
            {
              name: "Normal",
              type: "line",
              stack: "total",
              symbol: "none",
              smooth: true,
              sampling: "average",
              animation: false,
              seriesLayoutBy: "row",
              z: 2,
              emphasis: {
                focus: "series",
                itemStyle: { shadowBlur: 10, shadowColor: "#9DD3E8" },
              },
              areaStyle: {
                opacity: (params) => (params.data[1] > 0 ? 0.8 : 0),
                color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                  { offset: 0, color: "rgba(1, 67, 221, 0.8)" },
                  { offset: 1, color: "rgba(1, 67, 221, 0.6)" },
                ]),
              },
              lineStyle: { width: 0, color: "#0143DD" },
            },
            {
              name: "Attack",
              type: "line",
              stack: "total",
              symbol: "none",
              smooth: true,
              sampling: "average",
              animation: false,
              seriesLayoutBy: "row",
              z: 1,
              emphasis: {
                focus: "series",
                itemStyle: { shadowBlur: 10, shadowColor: "#F49494" },
              },
              areaStyle: {
                opacity: (params) => (params.data[2] > 0 ? 0.8 : 0),
                color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
                  { offset: 0, color: "rgba(221, 1, 1, 0.8)" },
                  { offset: 1, color: "rgba(221, 1, 1, 0.6)" },
                ]),
              },
              lineStyle: { width: 0, color: "#DD0101" },
            },
          ]
        : [
            {
              name: "Normal",
              type: "line",
              stack: "total",
              symbol: "none",
              smooth: true,
              data: Array(getDefaultEmptyData()[0].length - 1).fill(0),
              areaStyle: { opacity: 0 },
              lineStyle: { width: 0 },
            },
            {
              name: "Attack",
              type: "line",
              stack: "total",
              symbol: "none",
              smooth: true,
              data: Array(getDefaultEmptyData()[0].length - 1).fill(0),
              areaStyle: { opacity: 0 },
              lineStyle: { width: 0 },
            },
          ],
      noDataLoadingOption: {
        text: "No data",
        effect: "bubble",
        effectOption: {
          effect: {
            n: 0,
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
          ref={chartRef}
          option={trend_option}
          style={{ width: "75%", height: "250px" }}
          opts={{ renderer: "canvas" }}
          notMerge={true}
          lazyUpdate={true}
        />
      </>
    );
  }
);

export default TrafficChart;
