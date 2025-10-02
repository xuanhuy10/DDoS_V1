const cron = require("node-cron");
const { spawn } = require("child_process");
const { getDDoS } = require('../models/DDoS.model');

if(process.env.NODE_OS === 'linux') {
      const batFilePath = "/home/antiddos/DDoS_V2/gui/server/Setting/delete_old_logs.sh";
} else {
      const batFilePath = "D:\\delete-logs\\delete_old_logs.bat";
}
const settime = "30 * * * *";


//       cron.schedule(settime, () => {
//       console.log(`Running script at ${settime}`);
//       const bat = spawn("cmd.exe", ["/c", batFilePath]);

//       bat.stdout.on("data", (data) => {
//           console.log(`Output: ${data}`);
//       });

//       bat.stderr.on("data", (data) => {
//           console.error(`Error: ${data}`);
//       });

//       bat.on("exit", (code) => {
//           console.log(`Process exited with code ${code}`);
//       });
//   }, {
//       timezone: "Asia/Ho_Chi_Minh"
//         });

async function autoLogSystem() {
      try {
            const task = await getDDoS(); // Chờ dữ liệu trả về

            if (task[0].AutoLogSystem === 1) {
                  if (!cron.validate(settime)) {
                        console.error("Invalid cron format:", settime);
                        return;
                  }

                  cron.schedule(settime, () => {
                        console.log(`Running script at ${settime}`);
                        const bat = spawn("cmd.exe", ["/c", batFilePath]);

                        bat.stdout.on("data", (data) => console.log(`Output: ${data}`));
                        bat.stderr.on("data", (data) => console.error(`Error: ${data}`));
                        bat.on("exit", (code) => console.log(`Process exited with code ${code}`));
                  }, {
                        timezone: "Asia/Ho_Chi_Minh"
                  });

                  console.log(" Cron job scheduled successfully!");
            } else {
                  console.log(" AutoLogSystem is disabled.");
            }
      } catch (error) {
            console.error("Error fetching task:", error);
      }
}

module.exports = { autoLogSystem };
// (async () => {
//     await startCronJob();
// })();

// // Gọi hàm khởi động cron job khi app chạy
// setInterval(startCronJob, 200000);