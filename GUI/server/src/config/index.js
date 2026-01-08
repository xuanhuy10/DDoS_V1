let fs = require('fs');
const path = require("path");
require('dotenv').config();
const os = require('os');
let ifaces = os.networkInterfaces();
let ip = null;
const pathBuild = (process.pkg) ? process.cwd() : __dirname;

let logPath = null,
    diskPath = null,
    diskThresholdPath = null,
    logPathNormal = null,
    configAutoManualPath = null,
    ipSecCertificatePath = null;

Object.keys(ifaces).forEach(function (ifname) {
    let alias = 0;
    ifaces[ifname].forEach(function (iface) {
        if ('IPv4' !== iface.family || iface.internal !== false) {
            return;
        }
        ip = iface.address;
    });
});

if (process.env.NODE_OS === 'linux') {
    logPath = '/home/antiddos/DDoS_V1/Log/Log_Flood/';
    logPathNormal = '/home/antiddos/DDoS_V1/Log/Log_Normal/';
    ipSecCertificatePath = '/zynq_lib/etc/ipsec.d/';

    diskThresholdPath = '/home/antiddos/DDoS_V1/Setting/threshold_logfile.conf';
    configAutoManualPath = '/home/antiddos/DDoS_V1/Setting/config_auto_manual.conf';
    diskPath = '/';
} else {
    logPath = path.join(pathBuild, '../../testattack/');
    logPathNormal = path.join(pathBuild, '../../test/');
    ipSecCertificatePath = path.join(pathBuild, '../../test/certificates/');

    diskThresholdPath = path.join(pathBuild, '../../test/threshold_logfile.txt');
    diskPath = 'C:';
    configAutoManualPath = path.join(pathBuild, '../../test/config_auto_manual.txt');
}

// Tạo biến isProduction
const isProduction = process.env.NODE_ENV === 'production';

// Parse PROGRESS_LOG thành boolean
const progressLogEnabled = String(process.env.PROGRESS_LOG).toLowerCase() === 'true';

// ANSI color codes
const COLORS = {
    reset:    "\x1b[0m",
    blue:     "\x1b[34m", // DISPLAY
    green:    "\x1b[32m", // DISPLAY SYSTEM ACTION
    yellow:   "\x1b[33m", // DISPLAY DATA
    red:      "\x1b[31m", // DISPLAY ERROR
    cyan:     "\x1b[36m", // DISPLAY TASK
    magenta:  "\x1b[35m", // DISPLAY MAINC THREAD DATA
    spacing: '---------------: ',
};

function progressLog(...parts) {
    if (progressLogEnabled) {
        console.log(...parts);
    }
}

function cProgressLog(...parts) {
    if (progressLogEnabled) {
        const msg = parts.join('') + COLORS.reset;
        console.log(msg);
    }
}

module.exports = {
    port: process.env.PORT || 3000,
    ip: ip || 'localhost',
    isProduction,
    progressLogEnabled,
    progressLog,
    cProgressLog,
    COLORS,
    dblite: {
        path: process.env.DB_PATH || path.join(__dirname, '../../database/sysnetdef.db'),
        path2: process.env.DB_PATH2 || path.join(__dirname, '../../database/sysnetdef.db')
    },
    mail: {
        host: 'smtp.gmail.com',
        port: 587,
        secure: false,
        auth: {
            user: process.env.EMAIL_USER,
            pass: process.env.EMAIL_PASSWORD,
        }
    },
    packet: {
        logPath: logPath,
        logPathNormal: logPathNormal,
        diskThresholdPath: diskThresholdPath,
        diskPath: diskPath,
        udsSocketPath: '/tmp/defender',
        configAutoManualPath: configAutoManualPath,
        ipSecCertificatePath: ipSecCertificatePath,
    },
};
