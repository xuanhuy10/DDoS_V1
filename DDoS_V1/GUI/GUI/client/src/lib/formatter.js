const byteFormatter = (bytes, decimals = 2) => {
  try {
    if (isNaN(bytes)) return "0 B";
    if (bytes === 0) return "0 B";
    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ["Bytes", "Kb", "Mb", "Gb", "Tb", "Pb", "Eb", "Zb", "Yb"];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + " " + sizes[i];
  } catch (error) {
    return "Error formatting bytes";
  }
};

const bitFormatter = (bits) => {
  try {
    if (isNaN(bits) || !Number.isFinite(bits)) return "0 b";
    if (bits === 0) return "0 b";

    const isNegative = bits < 0;
    bits = Math.abs(bits);

    const k = 1000;
    const sizes = ["b", "Kb", "Mb", "Gb", "Tb"];

    // Nhỏ hơn 1 bit → giữ đến 6 chữ số thập phân, không dùng scientific
    if (bits < 1) {
      const formatted = bits.toFixed(6).replace(/\.?0+$/, "");
      return (isNegative ? "-" : "") + `${formatted} b`;
    }

    let i = Math.floor(Math.log(bits) / Math.log(k));
    if (i < 0) i = 0;

    const value = bits / Math.pow(k, i);
    const formatted = value.toFixed(2).replace(/\.?0+$/, "");
    return (isNegative ? "-" : "") + `${formatted} ${sizes[i]}`;
  } catch (error) {
    console.error("Error in bitFormatter:", error);
    return "Error formatting bits";
  }
};

const cntFormatter = (count, decimals = 2) => {
  try {
    if (isNaN(count)) return "0 p";
    if (count === 0) return "0 p";
    const k = 1000;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ["Kp", "Mp", "Bp", "Tp"];
    if (count < k) return count + " p";
    const i = Math.floor(Math.log(count) / Math.log(k));
    const sizeIndex = Math.min(i - 1, sizes.length - 1);
    if (sizeIndex < 0) return count + " p";
    const formattedValue = parseFloat((count / Math.pow(k, i)).toFixed(dm));
    return `${formattedValue} ${sizes[sizeIndex]}`;
  } catch (error) {
    return "Error formatting count";
  }
};

const timeFormatter = (timeString) => {
  try {
    if (!timeString) return "Not used yet";
    const time = new Date(parseInt(timeString));
    return new Intl.DateTimeFormat("vn-VN", {
      year: "numeric",
      month: "2-digit",
      day: "2-digit",
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      hour12: false, // đông hồ 24 giờ
      timeZone: "UTC", // Force UTC
    }).format(time);
  } catch (error) {
    return "Invalid time data";
  }
};

const profileTimeFormatter = (timeString) => {
  try {
    let totalDurationMs = 0;
    if (!timeString) {
      return "Not used yet";
    }
    const timeData = JSON.parse(timeString);

    if (timeData.length === 0) {
      return "No data";
    }

    let lastEnd = null;
    for (const { start, end } of timeData) {
      if (end === undefined || end === null) {
        return start ? `In used, since: ${start}` : "In used";
      }

      const startTime = start ? new Date(start) : new Date();
      const endTime = end ? new Date(end) : new Date();

      totalDurationMs += endTime - startTime;
      lastEnd = end;
    }

    const totalSeconds = Math.round(totalDurationMs / 1000);
    const totalMinutes = Math.floor(totalSeconds / 60);
    const totalHours = Math.floor(totalMinutes / 60);
    const totalDays = Math.floor(totalHours / 24);

    if (totalDays > 0) {
      return `${totalDays} days, last used: ${lastEnd}`;
    } else if (totalHours > 0) {
      return `${totalHours} hours, last used: ${lastEnd}`;
    } else if (totalMinutes > 0) {
      return `${totalMinutes} minutes, last used: ${lastEnd}`;
    } else {
      return `${totalSeconds} seconds, last used: ${lastEnd}`;
    }
  } catch (error) {
    return "Invalid time data";
  }
};

const formatDate = (dateInput) => {
  const d = new Date(dateInput);
  const day = String(d.getDate()).padStart(2, "0");
  const month = String(d.getMonth() + 1).padStart(2, "0");
  const year = d.getFullYear();
  return `${day}/${month}/${year}`;
};

export {
  byteFormatter,
  bitFormatter,
  cntFormatter,
  timeFormatter,
  profileTimeFormatter,
  formatDate,
};
