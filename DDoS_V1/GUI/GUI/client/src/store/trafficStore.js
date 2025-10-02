// store/trafficStore.js
import { create } from 'zustand';
import { persist } from 'zustand/middleware';

const dataPoints = 300;

// Khởi tạo template ban đầu cho timeseriesData (tương tự LineChart.jsx)
const initialTimeseriesTemplate = {
  timestamps: Array(dataPoints).fill(0),
  total: {
    bypass: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    attack: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
  },
  attacks: {
    icmpFlood: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    udpFlood: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    dnsFlood: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    udpFrag: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    tcpFrag: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    synFlood: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    land: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    httpFlood: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    httpsFlood: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    ipsec: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    unknown: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
  },
  protocol: {
    icmp: {
      bypass: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
      attack: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    },
    udp: {
      bypass: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
      attack: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    },
    tcp: {
      bypass: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
      attack: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    },
    http: {
      bypass: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
      attack: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    },
    dns: {
      bypass: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
      attack: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    },
    esp: {
      bypass: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
      attack: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    },
    unknown: {
      bypass: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
      attack: { bits: Array(dataPoints).fill(0), packets: Array(dataPoints).fill(0) },
    },
  },
};

// Tạo store với Zustand
export const useTrafficStore = create(
  persist(
    (set, get) => ({
      timeseriesData: initialTimeseriesTemplate,
      trafficData: null,
      chartSumData: null,
      pause: false,

      // Cập nhật trafficData
      setTrafficData: (summary, onsec, status) => {
        const trafixData = { summary, onsec, status };
        set({ trafficData: trafixData });
        // Tự động cập nhật chartSumData khi trafficData thay đổi
        if (trafixData) {
          const chartData = processData(trafixData.summary);
          set({ chartSumData: chartData });
        }
      },

      // Cập nhật timeseriesData
      updateTimeseriesData: (psData) => {
        const { timeseriesData, pause } = get();
        if (!pause && psData) {
          const updateData = (oldData, newData) => oldData.slice(1).concat(newData);

          const updatedData = {
            timestamps: updateData(timeseriesData.timestamps, psData.timeStamp),
            total: {
              bypass: {
                bits: updateData(timeseriesData.total.bypass.bits, psData.total.bypass.bits),
                packets: updateData(timeseriesData.total.bypass.packets, psData.total.bypass.packets),
              },
              attack: {
                bits: updateData(timeseriesData.total.attack.bits, psData.total.attack.bits),
                packets: updateData(timeseriesData.total.attack.packets, psData.total.attack.packets),
              },
            },
            protocol: Object.keys(timeseriesData.protocol).reduce((acc, key) => {
              acc[key] = {
                bypass: {
                  bits: updateData(timeseriesData.protocol[key].bypass.bits, psData.protocol[key].bypass.bits),
                  packets: updateData(timeseriesData.protocol[key].bypass.packets, psData.protocol[key].bypass.packets),
                },
                attack: {
                  bits: updateData(timeseriesData.protocol[key].attack.bits, psData.protocol[key].attack.bits),
                  packets: updateData(timeseriesData.protocol[key].attack.packets, psData.protocol[key].attack.packets),
                },
              };
              return acc;
            }, {}),
            attacks: Object.keys(timeseriesData.attacks).reduce((acc, key) => {
              acc[key] = {
                bits: updateData(timeseriesData.attacks[key].bits, psData.attack[key].bits),
                packets: updateData(timeseriesData.attacks[key].packets, psData.attack[key].packets),
              };
              return acc;
            }, {}),
          };

          set({ timeseriesData: updatedData });
        }
      },

      // Chuyển đổi trạng thái pause
      togglePause: () => set((state) => ({ pause: !state.pause })),

      // Reset toàn bộ dữ liệu
      resetData: () =>
        set({
          timeseriesData: initialTimeseriesTemplate,
          trafficData: null,
          chartSumData: null,
          pause: false,
        }),
    }),
    {
      name: 'traffic-storage', // Tên key trong localStorage
      getStorage: () => localStorage, // Lưu vào localStorage
    }
  )
);

// Hàm processData (tương tự dashboard.jsx)
const processData = (data) => {
  try {
    const getData = (category) => {
      if (category === "total") {
        return {
          bits: {
            received: data.total.bypass.bits,
            drop: -data.total.attack.bits,
            total: data.total.bypass.bits + data.total.attack.bits,
          },
          packets: {
            received: data.total.bypass.packets,
            drop: -data.total.attack.packets,
            total: data.total.bypass.packets + data.total.attack.packets,
          },
        };
      }

      const categoryData = data.protocol[category];
      return {
        bits: {
          received: categoryData.bypass.bits,
          drop: -categoryData.attack.bits,
          total: categoryData.bypass.bits + categoryData.attack.bits,
        },
        packets: {
          received: categoryData.bypass.packets,
          drop: -categoryData.attack.packets,
          total: categoryData.bypass.packets + categoryData.attack.packets,
        },
      };
    };

    return {
      total: getData("total"),
      icmp: getData("icmp"),
      udp: getData("udp"),
      tcp: getData("tcp"),
      http: getData("http"),
      esp: getData("esp"),
      unknown: getData("unknown"),
    };
  } catch (error) {
    console.error(error);
    return null;
  }
};