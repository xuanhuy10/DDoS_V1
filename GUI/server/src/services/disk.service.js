// const { getDDoS } = require('../models/DDoS.model');
// const { createNotification } = require('../models/Notification.model');

const { getStorageInfo, getFileDetails } = require("../helper/utils.helper");
const fs = require("fs");
const path = require("path");
const config = require("../config/index");
const { sendDiskFullAlertEmail } = require("./email.service");

//create a function to check the disk space and delete the log files if the disk space is less than the threshold in 30 minutes interval
async function autoDeleteLog(io) {
  try {
    // const { TrafficAutoDelete, TrafficAutoDeleteThreshold } = await getDDoS();
    const get = await getDDoS();
    console.log("getDDoS", get);
    const TrafficAutoDelete = get[0].TrafficAutoDelete;
    const TrafficAutoDeleteThreshold = get[0].TrafficAutoDeleteThreshold;
    console.log(
      "TrafficAutoDelete: ",
      TrafficAutoDelete,
      "TrafficAutoDeleteThreshold: ",
      TrafficAutoDeleteThreshold
    );
    let disk = await getStorageInfo();
    console.log("disk", disk);
    if (disk.usedPercentage > TrafficAutoDeleteThreshold) {
      // console.log('Disk space threshold exceeded. Starting email alert process...');
      try {
        await sendDiskFullAlertEmail(disk.usedPercentage);
        // console.log('Disk full alert email sent successfully');
      } catch (emailError) {
        console.error("Failed to send disk full alert email:", emailError);
      }

      if (TrafficAutoDelete) {
        //console.log('Auto delete is enabled. Starting log cleanup...');
        let logNormal = getFileDetails(config.packet.logPathNormal);
        let logFlood = getFileDetails(config.packet.logPath);

        //sendDiskFullAlertEmail(disk.usedPercentage);

        let logNormalDetails = logNormal.logDetails;
        let logDetails = logFlood.logDetails;
        // console.log('logNormalDetails', logNormalDetails);
        // console.log('logDetails', logDetails);

        // while (disk.usedPercentage > TrafficAutoDeleteThreshold) {
        //     let deleted = false;

        //     if (logNormalDetails.length > 1) {
        //         logNormalDetails.sort((a, b) => a.lastModified - b.lastModified);
        //         fs.unlinkSync(path.join(config.packet.logPathNormal, logNormalDetails[0].filename));
        //         console.log('logNormalDetails', logNormalDetails);
        //         logNormalDetails.shift();
        //         deleted = true;
        //     } else if (logNormalDetails.length === 1) {
        //         fs.writeFileSync(path.j oin(config.packet.logPathNormal, logNormalDetails[0].filename), '');
        //         logNormalDetails = []; // ÄÃ¡nh dáº¥u Ä‘Ã£ xÃ³a háº¿t
        //         deleted = true;
        //     }

        //     // Náº¿u khÃ´ng cÃ²n log normal Ä‘á»ƒ xÃ³a, chuyá»ƒn sang log flood
        //     if (!deleted && logDetails.length > 1) {
        //         logDetails.sort((a, b) => a.lastModified - b.lastModified);
        //         fs.unlinkSync(path.join(config.packet.logPath, logDetails[0].filename));
        //         logDetails.shift();
        //         deleted = true;
        //     } else if (!deleted && logDetails.length === 1) {
        //         fs.writeFileSync(path.join(config.packet.logPath, logDetails[0].filename), '');
        //         logDetails = []; // ÄÃ¡nh dáº¥u Ä‘Ã£ xÃ³a háº¿t
        //         deleted = true;
        //     }

        //     // Náº¿u khÃ´ng cÃ³ file nÃ o Ä‘á»ƒ xÃ³a, dá»«ng vÃ²ng láº·p
        //     if (deleted === false && logNormalDetails.length === 0 && logDetails.length === 0) {
        //         console.log('No more log files to delete');
        //         break;
        //     }

        //     // Cáº­p nháº­t láº¡i thÃ´ng tin dung lÆ°á»£ng sau khi xÃ³a file
        //     disk = await getStorageInfo();
        // }

        // Gá»­i thÃ´ng bÃ¡o sau khi xÃ³a

        while (disk.usedPercentage > TrafficAutoDeleteThreshold) {
          console.log("start");
          if (logNormalDetails.length > 1) {
            console.log("normal");
            logNormalDetails.sort(
              (a, b) => new Date(a.lastModified) - new Date(b.lastModified)
            );
            fs.unlinkSync(
              path.join(
                config.packet.logPathNormal,
                logNormalDetails[0].filename
              )
            );
            logNormalDetails.shift();
          } else if (logNormalDetails.length === 1) {
            fs.writeFileSync(
              path.join(
                config.packet.logPathNormal,
                logNormalDetails[0].filename
              ),
              ""
            );
            logNormalDetails = []; // ÄÃ¡nh dáº¥u Ä‘Ã£ xÃ³a háº¿t
          } else {
            break;
          }
          if (logDetails.length > 1) {
            console.log("flood");
            logDetails.sort(
              (a, b) => new Date(a.lastModified) - new Date(b.lastModified)
            );
            fs.unlinkSync(
              path.join(config.packet.logPath, logDetails[0].filename)
            );
            logDetails.shift();
            disk = await getStorageInfo();
          } else if (logDetails.length === 1) {
            fs.writeFileSync(
              path.join(config.packet.logPath, logDetails[0].filename),
              ""
            );
            logDetails = []; // ÄÃ¡nh dáº¥u Ä‘Ã£ xÃ³a háº¿t
          } else {
            break;
          }
        }
        const now = new Date();
        now.setHours(now.getHours() + 7); // UTC+7 (Viá»‡t Nam)
        let notification = {
          NotificationType: "Warning",
          Notificate:
            `Disk space is running low. Deleted old log files to free up space. ` +
            `Current disk space: ${disk.usedPercentage.toFixed(2)}%`,
          NotificationTime: now
            .toISOString()
            .replace("T", " ")
            .replace("Z", "")
            .split(".")[0],
          UserId: 1,
          IsRead: 0,
        };
        await createNotification(notification);
        io.emit("diskFull", disk.usedPercentage.toFixed(2));
      } else {
        // Náº¿u khÃ´ng báº­t auto-delete, chá»‰ gá»­i thÃ´ng bÃ¡o
        const now = new Date();
        now.setHours(now.getHours() + 7);
        let notification = {
          NotificationType: "Warning",
          Notificate:
            `Disk space is running low. Please delete log files to free up space. ` +
            `Current disk space: ${disk.usedPercentage.toFixed(2)}%`,
          NotificationTime: now
            .toISOString()
            .replace("T", " ")
            .replace("Z", "")
            .split(".")[0],
          UserId: 1,
          IsRead: 0,
        };
        await createNotification(notification);
        io.emit("diskFull", disk.usedPercentage.toFixed(2));
      }
    }
  } catch (err) {
    console.error("Error deleting log files: ", err);
  }
}

module.exports = { autoDeleteLog };
