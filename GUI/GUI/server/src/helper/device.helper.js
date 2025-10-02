const si = require('systeminformation');
const conf = require('../config/index');
const fs = require('fs');
const path = require('path');

async function getStorageInfo() {
    try {
        const diskPath = conf?.packet?.diskPath || "/";
        const fsList = await si.fsSize();

        const disk = fsList.find(d => d.mount.toLowerCase().includes(diskPath.toLowerCase()));

        if (!disk) {
            console.warn(`Disk not found for path: ${diskPath}`);
            console.warn('Available mounts:', fsList.map(d => d.mount));
            throw new Error(`Disk not found at path: ${diskPath}`);
        }

        return {
            type: disk.mount,
            total: disk.size,
            free: disk.available,
            used: disk.used,
            usedPercentage: parseFloat(((disk.used / disk.size) * 100).toFixed(2))
        };
    } catch (error) {
        console.error('Error getting storage info:', error);
        return null;
    }
}


async function getMemoryInfo() {
    try {
        const memory = await si.mem();
        return {
            total: memory.total,
            free: memory.free,
            used: memory.used,
            usedPercentage: parseFloat(((memory.used / memory.total) * 100).toFixed(2))
        };
    } catch (error) {
        console.error('Error getting memory info:', error);
        return null;
    }
}

async function getCpuInfo() {
    try {
        const cpu = await si.cpu();
        return {
            model: cpu.model,
            cores: cpu.cores,
            speed: cpu.speed + ' GHz',
            manufacturer: cpu.manufacturer
        };
    } catch (error) {
        console.error('Error getting CPU info:', error);
        return null;
    }
}

async function getCpuUsage() {
    try {
        const cpuLoad = await si.currentLoad();
        return {
            avgLoad: cpuLoad.avgLoad,
            currentLoad: parseFloat(cpuLoad.currentLoadSystem.toFixed(2)),
            coresLoad: cpuLoad.cpus.map((core, index) => ({
                core: index + 1,
                load: parseFloat(core.load.toFixed(2))
            }))
        };
    } catch (error) {
        console.error('Error getting CPU usage:', error);
        return null;
    }
}

// async function getNetworkInfo() {
//     try {
//         const [networkInterfaces, networkStats] = await Promise.all([
//             si.networkInterfaces(),
//             si.networkStats()
//         ]);

//         return networkInterfaces.map(interface => {
//             const stats = networkStats.find(s => s.iface === interface.iface);
//             return {
//                 name: interface.iface,
//                 ip4: interface.ip4,
//                 ip6: interface.ip6,
//                 mac: interface.mac,
//                 speed: interface.speed,
//                 rx_bytes: stats?.rx_bytes || 0,
//                 tx_bytes: stats?.tx_bytes || 0
//             };
//         });
//     } catch (error) {
//         console.error('Error getting network info:', error);
//         return null;
//     }
// }

module.exports = {
    getStorageInfo,
    getMemoryInfo,
    getCpuInfo,
    getCpuUsage,
    // getNetworkInfo
};