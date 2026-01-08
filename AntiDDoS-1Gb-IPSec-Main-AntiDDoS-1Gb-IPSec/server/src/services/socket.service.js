// /services/tcpService.js
const net = require('net');
const { isProduction } = require('../config/index');
const config = require('../config/index');

function sendCommandToCProgram(command) {
    if (!isProduction) {
        config.progressLog(config.COLORS.spacing,'Fake sending command to C program: ',config.COLORS.magenta,command,config.COLORS.reset);
        const response = 'OK';
        Atomics.wait(new Int32Array(new SharedArrayBuffer(4)), 0, 0, 0); // 1000 = 1 second
        config.progressLog(config.COLORS.spacing,config.COLORS.yellow,'mainC response : ',config.COLORS.magenta,response,config.COLORS.reset);
        return response;
    } else {
        config.progressLog(config.COLORS.spacing,'Sending command to C program: ',config.COLORS.magenta,command,config.COLORS.reset);
        return new Promise((resolve, reject) => {
            const client = new net.Socket();

            client.connect(3367, '127.0.0.1', () => {
                // console.log('Connected to C program');
                client.write(command + 'DONE$1');
            });

            client.on('data', (data) => {
                config.progressLog(config.COLORS.spacing,config.COLORS.yellow,'mainC response : ',config.COLORS.magenta,data.toString(),config.COLORS.reset);
                client.destroy(); // Close the connection
                resolve(data.toString());
            });

            client.on('close', () => {
                // console.log('Connection closed');
                // reject('Connection closed');
            });

            client.on('error', (err) => {
                console.error('Socket error: ', err);
                reject(new Error('Error communicating with Hardware: no connection established'));
            });
        });
    }
}

// Hàm kiểm tra kết nối tới mainC
function checkMainCConnection() {
    if (!isProduction) {
        return Promise.resolve(true);
    }

    return new Promise((resolve) => {
        const client = new net.Socket();
        let isResolved = false;
        client.setTimeout(1000);

        client.connect(3367, '127.0.0.1', () => {
            if (!isResolved) {
                isResolved = true;
                client.destroy();
                resolve(true);
            }
        });

        client.on('error', () => {
            if (!isResolved) {
                isResolved = true;
                resolve(false);
            }
        });

        client.on('timeout', () => {
            if (!isResolved) {
                isResolved = true;
                client.destroy();
                resolve(false);
            }
        });
    });
}


module.exports = { sendCommandToCProgram, checkMainCConnection };