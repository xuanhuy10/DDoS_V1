import API from "@/utils/api_v2";

export const getAnomalies = async () => {
  try {
    const response = await API.get("/network/anomalies");
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const analyzeNetwork = async (timeStart, timeEnd) => {
  try {
    const response = await API.get("/network/analysis", {
      params: {
        timeStart,
        timeEnd,
      },
    });
    return response.data;
  } catch (error) {
    throw error;
  }
};
