const fs = require("fs");
const si = require("systeminformation");
const path = require("path");
const { createCanvas } = require("canvas");

const { format } = require("date-fns");
const { vi } = require("date-fns/locale");

const conf = require("../config/index");

function getFileDetails(directoryPath) {
  try {
    const files = fs.readdirSync(directoryPath);

    let logSize = 0;
    const logDetails = files
      .map((file) => {
        const filePath = path.join(directoryPath, file);
        const stats = fs.statSync(filePath);

        if (stats.isFile()) {
          const ext = path.extname(file).toLowerCase();
          if (ext !== ".log") {
            return null;
          }
          const formattedModifiedTime = format(
            stats.mtime,
            "dd/MM/yyyy HH:mm:ss",
            { locale: vi }
          );
          const formattedCreationTime = format(
            stats.birthtime,
            "dd/MM/yyyy HH:mm:ss",
            { locale: vi }
          );
          logSize += stats.size;
          return {
            filename: file,
            createdAt: formattedCreationTime,
            lastModified: formattedModifiedTime,
            size: stats.size,
          };
        }
      })
      .filter(Boolean);

    return {
      logDetails,
      logSize: logSize,
    };
  } catch (err) {
    return {
      logDetails: [],
      logSize: "0 B",
    };
  }
}

async function getStorageInfo() {
  try {
    const diskPath = conf?.packet?.diskPath || "/";
    const disk = (await si.fsSize()).find((d) => d.mount === diskPath);
    if (!disk) throw new Error(`Disk not found at path: ${diskPath}`);

    return {
      type: disk.mount,
      total: disk.size,
      free: disk.available,
      used: disk.used,
      usedPercentage: parseFloat(((disk.used / disk.size) * 100).toFixed(2)),
    };
  } catch (error) {
    console.error("Error getting storage info:", error);
    return null;
  }
}

const constructIPv6 = (ipv6) => {
  let parts = ipv6.split(":");

  const emptyIndex = parts.indexOf("");
  if (emptyIndex !== -1) {
    const missingSections = 8 - parts.length + 1;
    parts = [
      ...parts.slice(0, emptyIndex),
      ...Array(missingSections).fill("0000"),
      ...parts.slice(emptyIndex + 1),
    ];
  }
  const fullIPv6 = parts.map((part) => part.padStart(4, "0")).join(":");
  return fullIPv6;
};

function checkIPValidity(ip) {
  const ipv4Regex = /^(\d{1,3}\.){3}\d{1,3}$/;
  const ipv6Regex =
    /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$/;

  if (ipv4Regex.test(ip)) {
    return ip
      .split(".")
      .every((num) => parseInt(num) >= 0 && parseInt(num) <= 255)
      ? "Valid IPv4"
      : "Invalid IP";
  }
  if (ipv6Regex.test(ip)) {
    return "Valid IPv6";
  }

  return "Invalid IP";
}

const removeDuplicateIPs = (path) => {
  const fs = require("fs");
  let lines = fs.readFileSync(path, "utf-8").split(/\s+/).filter(Boolean);
  let uniqueLines = Array.from(new Set(lines));

  fs.writeFileSync(path, uniqueLines.join("\n"), "utf-8");
};

const checkIPv4orIPv6 = (ip) => {
  if (ip.includes(":")) {
    return "IPv6";
  } else {
    return "IPv4";
  }
};

const generateProfileImage = (username, name) => {
  let names = name.split(" ");
  let initials = names[0][0].toUpperCase();
  if (names.length > 1) {
    initials += names[names.length - 1][0].toUpperCase();
  }

  const canvas = createCanvas(200, 200);
  const ctx = canvas.getContext("2d");

  // Generate a random background color with good contrast to white text
  function getRandomColor() {
    const letters = "0123456789ABCDEF";
    let color = "#";
    for (let i = 0; i < 6; i++) {
      color += letters[Math.floor(Math.random() * 16)];
    }
    return color;
  }

  function getContrastYIQ(hexcolor) {
    hexcolor = hexcolor.replace("#", "");
    const r = parseInt(hexcolor.substr(0, 2), 16);
    const g = parseInt(hexcolor.substr(2, 2), 16);
    const b = parseInt(hexcolor.substr(4, 2), 16);
    const yiq = (r * 299 + g * 587 + b * 114) / 1000;
    return yiq >= 128 ? "black" : "white";
  }

  let backgroundColor;
  do {
    backgroundColor = getRandomColor();
  } while (getContrastYIQ(backgroundColor) !== "white");

  // Background color
  ctx.fillStyle = backgroundColor;
  ctx.fillRect(0, 0, 200, 200);

  // Text
  ctx.fillStyle = "white";
  ctx.font = "bold 40px Arial";
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  ctx.fillText(initials, 100, 100);

  // Save to file
  const filename = "default.png";
  const imagePath = path.join(
    __dirname,
    "..",
    "public",
    "uploads",
    username,
    filename
  );
  if (!fs.existsSync(path.dirname(imagePath))) {
    fs.mkdirSync(path.dirname(imagePath), { recursive: true });
  }
  const out = fs.createWriteStream(imagePath);
  const stream = canvas.createPNGStream();
  stream.pipe(out);
  out.on("finish", () => {});
};

module.exports = {
  getFileDetails,
  getStorageInfo,
  constructIPv6,
  removeDuplicateIPs,
  checkIPv4orIPv6,
  checkIPValidity,
  generateProfileImage,
};
