const mailer = require("nodemailer");
const config = require("../config");
const { getAllUsers } = require("../models/Users.model");
const ejs = require("ejs");
const path = require("path");

const mailTransport = mailer.createTransport({
  service: "gmail",
  auth: {
    user: config.mail.auth.user,
    pass: config.mail.auth.pass,
  },
});

const pathBuild = process.pkg ? process.cwd() : __dirname;

const sendDiskFullAlertEmail = async (diskUsage) => {
  try {
    const users = await getAllUsers();
    const usersToNotify = users.filter((user) => user.NotifyDiskExceeds === 1);

    if (usersToNotify.length === 0) return;

    const warning = {
      message: "Cảnh báo: Dung lượng đĩa đang thấp!",
      diskUsage: diskUsage.toFixed(2),
      siteAddress: `http://${config.ip}:${config.port}`,
      time: new Date().toLocaleString(),
      freeSpace: (100 - diskUsage).toFixed(2),
    };

    const templatePath = path.join(pathBuild, "../public/mails/disk_full.ejs");
    const html = await ejs.renderFile(templatePath, warning);

    const emailPromises = usersToNotify.map((user) => {
      const mailOptions = {
        from: config.mail.auth.user,
        to: user.Email,
        subject: "Sysnet Defender Cảnh báo: Dung lượng đĩa thấp",
        html,
        attachments: [
          {
            filename: "ddos_logo.png",
            path: path.join(pathBuild, "../public/images/ddos_logo.png"),
            cid: "warninglogo",
          },
          {
            filename: "disk-storage.png",
            path: path.join(pathBuild, "../public/images/disk-storage.png"),
            cid: "disklogo",
          },
        ],
      };

      return new Promise((resolve, reject) => {
        mailTransport.sendMail(mailOptions, (err, info) => {
          if (err) {
            console.error("Error sending disk full email:", err);
            reject(err);
          } else {
            resolve(info);
          }
        });
      });
    });

    await Promise.all(emailPromises);
  } catch (error) {
    console.error("Error in sendDiskFullAlertEmail:", error);
  }
};

const sendAlertEmailAttack = async (attack) => {
  try {
    attack.siteAddress = `http://${config.ip}:${config.port}`;
    const html = await ejs.renderFile(
      path.join(pathBuild, "../public/mails/warn.ejs"),
      { attack }
    );

    const mailOptions = {
      from: config.mail.auth.user,
      subject: "Sysnet Defender Warning: Possible Attack Detected",
      html,
      attachments: [
        {
          filename: "ddos_logo.png",
          path: path.join(pathBuild, "../public/images/ddos_logo.png"),
          cid: "acslogo",
        },
      ],
    };

    const users = await getAllUsers();
    console.log(users);

    const emailPromises = users
      .filter((user) => user.NotifyDDoSAttackDetect === 1)
      .map((user) => {
        return new Promise((resolve, reject) => {
          mailOptions.to = user.Mail;
          mailTransport.sendMail(mailOptions, (err, info) => {
            if (err) {
              console.error("Error sending alert email:", err);
              reject(err);
            } else {
              resolve(info);
            }
          });
        });
      });

    await Promise.all(emailPromises);
  } catch (error) {
    console.error("Error in sendAlertEmailAttack:", error);
  }
};

const sendAlertEmailEnd = async (attack) => {
  try {
    attack.siteAddress = `http://${config.ip}:${config.port}`;
    const html = await ejs.renderFile(
      path.join(pathBuild, "../public/mails/end.ejs"),
      { attack }
    );

    const mailOptions = {
      from: config.mail.auth.user,
      subject: "Sysnet Defender Notice: Attack Ended",
      html,
      attachments: [
        {
          filename: "ddos_logo.png",
          path: path.join(pathBuild, "../public/images/ddos_logo.png"),
          cid: "acslogo",
        },
      ],
    };

    const users = await getAllUsers();
    const emailPromises = users
      .filter((user) => user.NotifyDDoSAttackEnd === 1)
      .map((user) => {
        return new Promise((resolve, reject) => {
          mailOptions.to = user.Mail;
          mailTransport.sendMail(mailOptions, (err, info) => {
            if (err) {
              console.error("Error sending end email:", err);
              reject(err);
            } else {
              resolve(info);
            }
          });
        });
      });

    await Promise.all(emailPromises);
  } catch (error) {
    console.error("Error in sendAlertEmailEnd:", error);
  }
};

const sendWarningEmail = async (warning) => {
  try {
    warning.siteAddress = `http://${config.ip}:${config.port}`;
    const html = await ejs.renderFile(
      path.join(pathBuild, "/public/mails/vulnerable.ejs"),
      { warning }
    );

    const mailOptions = {
      from: config.mail.auth.user,
      subject: "Sysnet Defender Alert: System Warning",
      html,
      attachments: [
        {
          filename: "system_warning.png",
          path: path.join(pathBuild, "/public/images/system_warning.png"),
          cid: "warninglogo",
        },
      ],
    };

    const users = await getAllUsers();
    const emailPromises = users
      .filter((user) => user.NotifyNetworkAnomalyDetect === 1)
      .map((user) => {
        return new Promise((resolve, reject) => {
          mailOptions.to = user.Mail;
          mailTransport.sendMail(mailOptions, (err, info) => {
            if (err) {
              console.error("Error sending warning email:", err);
              reject(err);
            } else {
              resolve(info);
            }
          });
        });
      });

    await Promise.all(emailPromises);
  } catch (error) {
    console.error("Error in sendWarningEmail:", error);
  }
};

module.exports = {
  sendAlertEmailAttack,
  sendWarningEmail,
  sendAlertEmailEnd,
  sendDiskFullAlertEmail,
};
