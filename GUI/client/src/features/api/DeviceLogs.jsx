import API from "@/utils/api_v2";
import axios from "axios";

export const getAllDeviceLogs = async () => {
  try {
    const response = await API.get("logs/");
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const getDeviceLogByLogType = async (logType) => {
  try {
    const response = await API.get(`logs/system/${logType}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const getTrafficLogByLogType = async (logType) => {
  try {
    const response = await API.get(`logs/traffic/${logType}`);
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const deleteLogTrafficbyLogTypeAndLogName = async (
  logType,
  logNames
) => {
  try {
    const response = await API.delete(
      `logs/traffic/${logType}/${logNames.join(",")}`
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const exportLogTrafficbyLogTypeAndLogName = async (
  logType,
  logNames,
  format = "zip"
) => {
  try {
    // Nếu là zip-excel hoặc zip-txt hoặc zip hoặc nhiều file, nhận về zip
    if (format.startsWith("zip") || logNames.length > 1) {
      const response = await API.get(
        `logs/traffic/export/${logType}/${logNames.join(",")}?format=${format}`,
        { responseType: "blob" }
      );
      if (response.status !== 200) {
        throw new Error(`Failed to download file: ${response.statusText}`);
      }
      const fileName = `Sysnetdef_Logs_${logType}_${new Date()
        .toISOString()
        .slice(0, 10)}.zip`;
      const url = window.URL.createObjectURL(new Blob([response.data]));
      const link = document.createElement("a");
      link.href = url;
      link.setAttribute("download", fileName);
      document.body.appendChild(link);
      link.click();
      link.remove();
      setTimeout(() => window.URL.revokeObjectURL(url), 0);
    } else {
      // Nếu chỉ chọn 1 file, tải về đúng định dạng và tên log
      const file = logNames[0];
      const ext = format === "xlsx" ? "xlsx" : "txt";
      const fileName = `Sysnetdef_Logs_${logType}_${file}.${ext}`;
      const response = await API.get(
        `logs/traffic/export/${logType}/${file}?format=${format}`,
        { responseType: "blob" }
      );
      if (response.status !== 200) {
        throw new Error(`Failed to download file: ${response.statusText}`);
      }
      const url = window.URL.createObjectURL(new Blob([response.data]));
      const link = document.createElement("a");
      link.href = url;
      link.setAttribute("download", fileName);
      document.body.appendChild(link);
      link.click();
      link.remove();
      setTimeout(() => window.URL.revokeObjectURL(url), 0);
    }
  } catch (error) {
    console.error("Error in downloadLogs:", error);
    throw error;
  }
};

export const deleteDeviceLogByLogTypeAndIds = async (logType, logIds) => {
  try {
    const response = await API.delete(
      `logs/system/${logType}/${logIds.join(",")}`
    );
    return response.data;
  } catch (error) {
    throw error;
  }
};

export const exportDeviceLogByLogTypeAndIds = async (logType, logIds) => {
  try {
    const response = await axios.get(
      `/logs/system/export/${logType}/${logIds.join(",")}`,
      { responseType: "blob" }
    );
    if (response.status !== 200)
      throw new Error(`Failed to download file: ${response.statusText}`);
    const fileName = `device_logs_${logType}_${new Date()
      .toISOString()
      .slice(0, 10)}.xlsx`;
    const url = window.URL.createObjectURL(new Blob([response.data]));
    const link = document.createElement("a");
    link.href = url;
    link.setAttribute("download", fileName);
    document.body.appendChild(link);
    link.click();
    link.remove();
    setTimeout(() => window.URL.revokeObjectURL(url), 0);
  } catch (error) {
    console.error("Error in exportDeviceLogByLogTypeAndIds:", error);
    throw error;
  }
};

export const exportDeviceLogByTypeAndTimeRange = async (logType, from, to) => {
  try {
    const response = await axios.get(
      `/logs/system/export/${logType}/time-range?from=${encodeURIComponent(
        from
      )}&to=${encodeURIComponent(to)}`,
      { responseType: "blob" }
    );

    if (response.status !== 200) {
      throw new Error(`Failed to download file: ${response.statusText}`);
    }

    // Không cần kiểm tra content-type nếu BE đã đúng
    const fileName = `device_logs_${logType}_${from}_to_${to}.xlsx`;
    const url = window.URL.createObjectURL(new Blob([response.data]));
    const link = document.createElement("a");
    link.href = url;
    link.setAttribute("download", fileName);
    document.body.appendChild(link);
    link.click();
    link.remove();
    setTimeout(() => window.URL.revokeObjectURL(url), 0);
  } catch (error) {
    console.error("Error in exportDeviceLogByTypeAndTimeRange:", error);
    throw error;
  }
};

// Xóa logs theo timerange
export const deleteDeviceLogByTypeAndTimeRange = async (logType, from, to) => {
  const response = await API.delete(
    `logs/system/${logType}/time-range?from=${encodeURIComponent(
      from
    )}&to=${encodeURIComponent(to)}`
  );
  return response.data;
};

/*
// Export traffic logs by time range
export const exportLogTrafficByTypeAndTimeRange = async (logType, from, to) => {
    try {
        const response = await API.get(
            `logs/traffic/export/${logType}/time-range?from=${encodeURIComponent(from)}&to=${encodeURIComponent(to)}`,
            { responseType: 'blob' }
        );
        if (response.status !== 200) {
            throw new Error(`Failed to download file: ${response.statusText}`);
        }
        const fileName = `Netdef_Logs_${logType}_${from}_to_${to}.zip`;
        const url = window.URL.createObjectURL(new Blob([response.data]));
        const link = document.createElement('a');
        link.href = url;
        link.setAttribute('download', fileName);
        document.body.appendChild(link);
        link.click();
        link.remove();
        setTimeout(() => window.URL.revokeObjectURL(url), 0);
    } catch (error) {
        console.error('Error in exportLogTrafficByTypeAndTimeRange:', error);
        throw error;
    }
};

 // Delete traffic logs by time range
export const deleteLogTrafficByTypeAndTimeRange = async (logType, from, to) => {
    try {
        const response = await API.delete(
            `logs/traffic/${logType}/time-range?from=${encodeURIComponent(from)}&to=${encodeURIComponent(to)}`
        );
        return response.data;
    } catch (error) {
        throw error;
    }
}; */
