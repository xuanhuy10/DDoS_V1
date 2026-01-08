const fs = require('fs');
const path = require('path');
const os = require('os');
const { format } = require('date-fns');
const { vi } = require('date-fns/locale');
const { insertSystemLogToDatabase } = require('../helper/dbo/logs.helper');
const { sendCommandToCProgram } = require('../services/socket.service');
const {
    getNetworkAddressesByAddressType,
    getNetworkAddressesByAddressId,
    insertNetworkAddress,
    deleteNetworkAddress,
    deleteNetworkAddressByAddressAndVersion,
    deleteNetworkAddressByAddressVersionAndPort
} = require('../models/NetworkAddresses.model');
const config = require("../config");
const IMPORT_FOLDER = path.join(__dirname, 'IP');

if (!fs.existsSync(IMPORT_FOLDER)) {
    fs.mkdirSync(IMPORT_FOLDER, { recursive: true });
}

/**
 * Lấy tất cả địa chỉ được bảo vệ
 *
 * Truy vấn cơ sở dữ liệu để lấy danh sách các địa chỉ mạng có loại là 'protected'.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về danh sách địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa danh sách địa chỉ.
 */
exports.getAllProtectedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Get all protected addresses ...', config.COLORS.reset);
    try {
        const addresses = await getNetworkAddressesByAddressType('protected');
        return res.status(200).json({ data: addresses });
    } catch (error) {
        console.log('error ', error);
        return res.status(500).json({ message: 'Get all protected addresses failed' });
    }
};
/**
 * Lấy tất cả địa chỉ bị chặn
 *
 * Truy vấn cơ sở dữ liệu để lấy danh sách các địa chỉ mạng có loại là 'blocked'.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về danh sách địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa danh sách địa chỉ.
 */
exports.getAllBlockedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Get all blocked addresses ...', config.COLORS.reset);
    try {
        const addresses = await getNetworkAddressesByAddressType('blocked');
        return res.status(200).json({ data: addresses });
    } catch (error) {
        console.log('error ', error);
        return res.status(500).json({ message: 'Get all blocked addresses failed' });
    }
};
/**
 * Lấy tất cả địa chỉ được phép VPN
 *
 * Truy vấn cơ sở dữ liệu để lấy danh sách các địa chỉ mạng có loại là 'vpn_white'.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về danh sách địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa danh sách địa chỉ.
 */
exports.getAllVPNAllowedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Get all VPN allowed addresses ...', config.COLORS.reset);
    try {
        const addresses = await getNetworkAddressesByAddressType('vpn_white');
        return res.status(200).json({ data: addresses });
    } catch (error) {
        console.log('error ', error);
        return res.status(500).json({ message: 'Get all VPN allowed addresses failed' });
    }
};
/**
 * Lấy tất cả địa chỉ HTTP bị chặn
 *
 * Truy vấn cơ sở dữ liệu để lấy danh sách các địa chỉ mạng có loại là 'http_black'.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về danh sách địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa danh sách địa chỉ.
 */
exports.getAllHTTPBlockedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Get all HTTP blocked addresses ...', config.COLORS.reset);
    try {
        const addresses = await getNetworkAddressesByAddressType('http_black');
        return res.status(200).json({ data: addresses });
    } catch (error) {
        console.log('error ', error);
        return res.status(500).json({ message: 'Get all HTTP blocked addresses failed' });
    }
};
/**
 * Thêm địa chỉ bị chặn
 *
 * Thêm một địa chỉ mạng vào danh sách bị chặn, gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa thông tin địa chỉ (Address, AddressVersion, Port).
 * @param {object} res - Phản hồi HTTP trả về kết quả thêm địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin địa chỉ đã thêm và tiến trình.
 */
exports.insertBlockedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Insert blocked addresses ...', config.COLORS.reset);
    const address = req.body;
    console.log('address ', address);
    let progressReport = "";
    try {
        address.AddressType = 'blocked';
        address.AddressAddedDate = format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi });
        address.AddressTimeOut = 0;
        let portNumber = '';
        if (address.Port) {
            const match = address.Port.match(/\d+$/);
            portNumber = match ? match[0] : address.Port;
        }
        const version = address.AddressVersion ? address.AddressVersion.toUpperCase() : '';
        const configPackage = `PORT${portNumber}_BLOCK_${version}$${address.Address}$`;
        const configResult = await sendCommandToCProgram(configPackage);
        const result = configResult.split('$');
        for (let i = 0; i < result.length; i++) {
            if (result[i] === 'ERROR') {
                await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
                    `Add network address: ${address.Address} to blocked address list`, 'Failed', 'Add address to blocked address list failed');
                return res.status(500).json({ message: 'Insert blocked address failed' });
            } else {
                progressReport += "mainC updated";
            }
        }
        const dbResult = await insertNetworkAddress(address);
        progressReport += " database updated";
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
            `Add network address: ${address.Address} to block address list`, 'Success', null);
        return res.status(201).json({ data: dbResult, progress: progressReport });
    } catch (error) {
        console.log('error ', error);
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
            `Add network address: ${address.Address} to block address list`, 'Failed', error.message);
        return res.status(500).json({ message: 'Insert blocked address failed', progress: progressReport });
    }
};
/**
 * Thêm địa chỉ được bảo vệ
 *
 * Thêm một địa chỉ mạng vào danh sách được bảo vệ, gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa thông tin địa chỉ (Address, AddressVersion, Port).
 * @param {object} res - Phản hồi HTTP trả về kết quả thêm địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin địa chỉ đã thêm và tiến trình.
 */
exports.insertProtectedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Insert protected addresses ...', config.COLORS.reset);
    const address = req.body;
    console.log('address ', address);
    let progressReport = "";
    try {
        address.AddressType = 'protected';
        address.AddressAddedDate = format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi });
        address.AddressTimeOut = 0;
        let portNumber = '';
        if (address.Port) {
            const match = address.Port.match(/\d+$/);
            portNumber = match ? match[0] : address.Port;
        }
        const version = address.AddressVersion ? address.AddressVersion.toUpperCase() : '';
        const configPackage = `PORT${portNumber}_ADD_${version}$${address.Address}$`;
        const configResult = await sendCommandToCProgram(configPackage);
        const result = configResult.split('$');
        for (let i = 0; i < result.length; i++) {
            if (result[i] === 'ERROR') {
                await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
                    `Add network address: ${address.Address} to protect address list`, 'Failed', 'Add address to protected address list failed');
                return res.status(500).json({ message: 'Insert protected address failed' });
            } else {
                progressReport += "mainC updated";
            }
        }
        const dbResult = await insertNetworkAddress(address);
        progressReport += " database updated";
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
            `Add network address: ${address.Address} to protect address list`, 'Success', null);
        return res.status(201).json({ data: dbResult, progress: progressReport });
    } catch (error) {
        console.log('error ', error);
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
            `Add network address: ${address.Address} to protect address list`, 'Failed', error.message);
        return res.status(500).json({ message: 'Insert protected address failed', progress: progressReport });
    }
};
/**
 * Thêm địa chỉ được phép VPN
 *
 * Thêm một địa chỉ mạng vào danh sách được phép VPN, gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa thông tin địa chỉ (Address, AddressVersion).
 * @param {object} res - Phản hồi HTTP trả về kết quả thêm địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin địa chỉ đã thêm và tiến trình.
 */
exports.insertVPNAllowedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Insert VPN allowed addresses ...', config.COLORS.reset);
    const address = req.body;
    console.log('address ', address);
    let progressReport = "";
    try {
        address.InterfaceId = null;
        address.AddressType = 'vpn_white';
        address.AddressAddedDate = format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi });
        address.AddressTimeOut = 0;
        let configPackage = '';
        if (address.AddressVersion && address.AddressVersion.toUpperCase() === 'IPV4') {
            configPackage = `ADD_IPV4_VPN$${address.Address}$`;
        } else if (address.AddressVersion && address.AddressVersion.toUpperCase() === 'IPV6') {
            configPackage = `ADD_IPV6_VPN$${address.Address}$`;
        } else {
            return res.status(400).json({ message: 'Invalid IP version', progress: progressReport });
        }
        const configResult = await sendCommandToCProgram(configPackage);
        if (!configResult.includes('OK')) {
            await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
                `Add network address: ${address.Address} to VPN allowed address list`, 'Failed', 'Add address to VPN allowed address list failed');
            return res.status(500).json({ message: 'Insert VPN allowed address failed', progress: progressReport });
        } else {
            progressReport += "mainC updated";
        }
        const dbResult = await insertNetworkAddress(address);
        progressReport += " database updated";
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
            `Add network address: ${address.Address} to VPN allowed address list`, 'Success', null);
        return res.status(201).json({ data: dbResult, progress: progressReport });
    } catch (error) {
        console.log('error ', error);
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
            `Add network address: ${address.Address} to VPN allowed address list`, 'Failed', error.message);
        return res.status(500).json({ message: 'Insert VPN allowed address failed', progress: progressReport });
    }
};
/**
 * Thêm địa chỉ HTTP bị chặn
 *
 * Thêm một địa chỉ mạng vào danh sách HTTP bị chặn, gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa thông tin địa chỉ (Address, AddressVersion).
 * @param {object} res - Phản hồi HTTP trả về kết quả thêm địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin địa chỉ đã thêm và tiến trình.
 */
exports.insertHTTPBlockedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Insert HTTP blocked addresses ...', config.COLORS.reset);
    const address = req.body;
    console.log('address ', address);
    let progressReport = "";
    try {
        address.InterfaceId = null;
        address.AddressType = 'http_black';
        address.AddressAddedDate = format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi });
        address.AddressTimeOut = 0;
        let configPackage = '';
        if (address.AddressVersion && address.AddressVersion.toUpperCase() === 'IPV4') {
            configPackage = `ADD_IPV4_HTTP$${address.Address}$`;
        } else if (address.AddressVersion && address.AddressVersion.toUpperCase() === 'IPV6') {
            configPackage = `ADD_IPV6_HTTP$${address.Address}$`;
        } else {
            return res.status(400).json({ message: 'Invalid IP version' });
        }
        const configResult = await sendCommandToCProgram(configPackage);
        if (!configResult.includes('OK')) {
            await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
                `Add network address: ${address.Address} to HTTP blocked address list`, 'Failed', 'Add address to HTTP blocked address list failed');
            return res.status(500).json({ message: 'Insert HTTP blocked address failed', progress: progressReport });
        } else {
            progressReport += "mainC updated";
        }
        const dbResult = await insertNetworkAddress(address);
        progressReport += " database updated";
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
            `Add network address: ${address.Address} to HTTP blocked address list`, 'Success', null);
        return res.status(201).json({ data: dbResult, progress: progressReport });
    } catch (error) {
        console.log('error ', error);
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Create Network Address',
            `Add network address: ${address.Address} to HTTP blocked address list`, 'Failed', error.message);
        return res.status(500).json({ message: 'Insert HTTP blocked address failed', progress: progressReport });
    }
};
/**
 * Nhập hàng loạt địa chỉ được bảo vệ
 *
 * Nhập danh sách địa chỉ được bảo vệ từ danh sách IP, lưu vào file CSV, gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion, Port).
 * @param {object} res - Phản hồi HTTP trả về kết quả nhập và danh sách địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo và danh sách địa chỉ.
 */
exports.importBulkProtectedAddresses = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : Import bulk protected addresses ...', config.COLORS.reset);
    try {
        const ipList = req.body;
        console.log('ipList ', ipList);
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        const portGroups = {};
        ipList.forEach(ip => {
            const port = ip.Port || '1';
            if (!portGroups[port]) portGroups[port] = [];
            portGroups[port].push(ip);
        });
        let cmds = [];
        for (const port in portGroups) {
            const portNum = port.replace(/^eth/, '');
            const ipv4List = portGroups[port].filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV4');
            const ipv6List = portGroups[port].filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV6');
            const toCsvContent = (list) => list.map(ip => `${ip.Address}`).join(os.EOL);
            const ipv4FileName = `protected_ipv4_port${port}.csv`;
            const ipv6FileName = `protected_ipv6_port${port}.csv`;
            const ipv4FilePath = path.join(IMPORT_FOLDER, ipv4FileName);
            const ipv6FilePath = path.join(IMPORT_FOLDER, ipv6FileName);
            fs.writeFileSync(ipv4FilePath, toCsvContent(ipv4List), 'utf8');
            fs.writeFileSync(ipv6FilePath, toCsvContent(ipv6List), 'utf8');
            cmds.push(`PORT${portNum}_SERVER_IPV4_ADD$${ipv4FilePath}$PORT${portNum}_SERVER_IPV6_ADD$${ipv6FilePath}$`);
        }
        const cmd = cmds.join('');
        const resCmd = await sendCommandToCProgram(cmd);
        if (!resCmd.includes('OK') && !resCmd.includes('DONE')) {
            return res.status(500).json({ message: `MainC Protected import failed: ${resCmd}` });
        }
        for (const ip of ipList) {
            await insertNetworkAddress({
                Address: ip.Address,
                AddressVersion: ip.AddressVersion,
                AddressType: 'protected',
                InterfaceId: 1,
                Port: ip.Port,
                AddressAddedDate: format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi }),
                AddressTimeOut: 0
            });
        }
        const addresses = await getNetworkAddressesByAddressType('protected');
        return res.status(201).json({ message: `Imported protected IPs from file successfully.`, data: addresses });
    } catch (error) {
        return res.status(500).json({ message: 'Import bulk protected IPs failed', error: error.message });
    }
};
/**
 * Xóa hàng loạt địa chỉ được bảo vệ
 *
 * Xóa danh sách địa chỉ được bảo vệ từ danh sách IP, lưu vào file CSV, gửi lệnh đến chương trình C và xóa khỏi cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion, Port).
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa và danh sách địa chỉ còn lại.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo và danh sách địa chỉ.
 */
exports.importDeleteBulkProtectedAddresses = async (req, res) => {
    try {
        const ipList = req.body;
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        const portGroups = {};
        ipList.forEach(ip => {
            const port = ip.Port || '1';
            if (!portGroups[port]) portGroups[port] = [];
            portGroups[port].push(ip);
        });
        let cmds = [];
        for (const port in portGroups) {
            const portNum = port.replace(/^eth/, '');
            const ipv4List = portGroups[port].filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV4');
            const ipv6List = portGroups[port].filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV6');
            const toCsvContent = (list) => list.map(ip => `${ip.Address}`).join(os.EOL);
            const ipv4FileName = `protected_ipv4_port${port}_delete.csv`;
            const ipv6FileName = `protected_ipv6_port${port}_delete.csv`;
            const ipv4FilePath = path.join(IMPORT_FOLDER, ipv4FileName);
            const ipv6FilePath = path.join(IMPORT_FOLDER, ipv6FileName);
            fs.writeFileSync(ipv4FilePath, toCsvContent(ipv4List), 'utf8');
            fs.writeFileSync(ipv6FilePath, toCsvContent(ipv6List), 'utf8');
            cmds.push(`PORT${portNum}_SERVER_IPV4_REMOVE$${ipv4FilePath}$PORT${portNum}_SERVER_IPV6_REMOVE$${ipv6FilePath}$`);
        }
        const cmd = cmds.join('');
        const resCmd = await sendCommandToCProgram(cmd);
        if (!resCmd.includes('OK') && !resCmd.includes('DONE')) {
            return res.status(500).json({ message: `MainC Protected import-delete failed: ${resCmd}` });
        }
        for (const ip of ipList) {
            await deleteNetworkAddressByAddressVersionAndPort(ip.Address, ip.AddressVersion, ip.Port);
        }
        const addresses = await getNetworkAddressesByAddressType('protected');
        return res.status(200).json({ message: `Bulk deleted protected IPs from file successfully.`, data: addresses });
    } catch (error) {
        return res.status(500).json({ message: 'Import delete bulk protected IPs failed', error: error.message });
    }
};
/**
 * Nhập hàng loạt địa chỉ HTTP bị chặn
 *
 * Nhập danh sách địa chỉ HTTP bị chặn từ danh sách IP, lưu vào file CSV, gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion, InterfaceId).
 * @param {object} res - Phản hồi HTTP trả về kết quả nhập và danh sách địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo và danh sách địa chỉ.
 */
exports.importBulkHTTPBlockedAddresses = async (req, res) => {
    console.log('==> importBulkHTTPBlockedAddresses called');
    try {
        const ipList = req.body;
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        const ipv4List = ipList.filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV4');
        const ipv6List = ipList.filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV6');
        const toCsvContent = (list) => list.map(ip => `${ip.Address}`).join(os.EOL);
        const ipv4FileName = 'http_black_ipv4.csv';
        const ipv6FileName = 'http_black_ipv6.csv';
        const ipv4FilePath = path.join(IMPORT_FOLDER, ipv4FileName);
        const ipv6FilePath = path.join(IMPORT_FOLDER, ipv6FileName);
        fs.writeFileSync(ipv4FilePath, toCsvContent(ipv4List), 'utf8');
        fs.writeFileSync(ipv6FilePath, toCsvContent(ipv6List), 'utf8');
        const cmd = `HTTP_TABLE_IPV4_ADD$${ipv4FilePath}$HTTP_TABLE_IPV6_ADD$${ipv6FilePath}$`;
        const resCmd = await sendCommandToCProgram(cmd);
        if (!resCmd.includes('OK') && !resCmd.includes('DONE')) {
            return res.status(500).json({ message: `MainC HTTP import failed: ${resCmd}` });
        }
        for (const ip of ipv4List) {
            await insertNetworkAddress({
                Address: ip.Address,
                AddressVersion: 'IPV4',
                AddressType: 'http_black',
                InterfaceId: ip.InterfaceId || 1,
                AddressAddedDate: format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi }),
                AddressTimeOut: 0
            });
        }
        for (const ip of ipv6List) {
            await insertNetworkAddress({
                Address: ip.Address,
                AddressVersion: 'IPV6',
                AddressType: 'http_black',
                InterfaceId: ip.InterfaceId || 1,
                AddressAddedDate: format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi }),
                AddressTimeOut: 0
            });
        }
        const addresses = await getNetworkAddressesByAddressType('http_black');
        return res.status(201).json({
            message: `Imported HTTP blocked IPs from file successfully.`,
            data: addresses
        });
    } catch (error) {
        console.error('Error importing bulk HTTP blocked addresses:', error);
        return res.status(500).json({ message: 'Import bulk HTTP blocked IPs failed', error: error.message });
    }
};
/**
 * Xóa hàng loạt địa chỉ HTTP bị chặn
 *
 * Xóa danh sách địa chỉ HTTP bị chặn từ danh sách IP, lưu vào file CSV, gửi lệnh đến chương trình C và xóa khỏi cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion).
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa và danh sách địa chỉ còn lại.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo và danh sách địa chỉ.
 */
exports.importDeleteBulkHTTPBlockedAddresses = async (req, res) => {
    try {
        const ipList = req.body;
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        const ipv4List = ipList.filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV4');
        const ipv6List = ipList.filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV6');
        const toCsvContent = (list) => list.map(ip => `${ip.Address}`).join(os.EOL);
        const ipv4FileName = 'http_black_ipv4_delete.csv';
        const ipv6FileName = 'http_black_ipv6_delete.csv';
        const ipv4FilePath = path.join(IMPORT_FOLDER, ipv4FileName);
        const ipv6FilePath = path.join(IMPORT_FOLDER, ipv6FileName);
        fs.writeFileSync(ipv4FilePath, toCsvContent(ipv4List), 'utf8');
        fs.writeFileSync(ipv6FilePath, toCsvContent(ipv6List), 'utf8');
        const cmd = `HTTP_TABLE_IPV4_REMOVE$${ipv4FilePath}$HTTP_TABLE_IPV6_REMOVE$${ipv6FilePath}$`;
        const resCmd = await sendCommandToCProgram(cmd);
        if (!resCmd.includes('OK') && !resCmd.includes('DONE')) {
            return res.status(500).json({ message: `MainC HTTP import-delete failed: ${resCmd}` });
        }
        for (const ip of ipList) {
            try {
                await deleteNetworkAddressByAddressAndVersion(ip.Address, ip.AddressVersion);
            } catch (err) {
                console.error(`[ERROR] Failed to delete HTTP IP ${ip.Address} from DB:`, err);
            }
        }
        const addresses = await getNetworkAddressesByAddressType('http_black');
        return res.status(200).json({
            message: `Bulk deleted HTTP blocked IPs from file successfully.`,
            data: addresses
        });
    } catch (error) {
        console.error('Error import-delete bulk HTTP blocked addresses:', error);
        return res.status(500).json({ message: 'Import delete bulk HTTP blocked IPs failed', error: error.message });
    }
};
/**
 * Nhập hàng loạt địa chỉ được phép VPN
 *
 * Nhập danh sách địa chỉ được phép VPN từ danh sách IP, lưu vào file CSV, gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion, InterfaceId).
 * @param {object} res - Phản hồi HTTP trả về kết quả nhập và danh sách địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo và danh sách địa chỉ.
 */
exports.importBulkVPNAllowedAddresses = async (req, res) => {
    try {
        const ipList = req.body;
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        const ipv4List = ipList.filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV4');
        const ipv6List = ipList.filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV6');
        const toCsvContent = (list) => list.map(ip => `${ip.Address}`).join(os.EOL);
        const ipv4FileName = 'vpn_white_ipv4.csv';
        const ipv6FileName = 'vpn_white_ipv6.csv';
        const ipv4FilePath = path.join(IMPORT_FOLDER, ipv4FileName);
        const ipv6FilePath = path.join(IMPORT_FOLDER, ipv6FileName);
        fs.writeFileSync(ipv4FilePath, toCsvContent(ipv4List), 'utf8');
        fs.writeFileSync(ipv6FilePath, toCsvContent(ipv6List), 'utf8');
        const cmd = `VPN_TABLE_IPV4_ADD$${ipv4FilePath}$VPN_TABLE_IPV6_ADD$${ipv6FilePath}$`;
        const resCmd = await sendCommandToCProgram(cmd);
        if (!resCmd.includes('OK') && !resCmd.includes('DONE')) {
            return res.status(500).json({ message: `MainC VPN import failed: ${resCmd}` });
        }
        for (const ip of ipList) {
            await insertNetworkAddress({
                Address: ip.Address,
                AddressVersion: ip.AddressVersion,
                AddressType: 'vpn_white',
                InterfaceId: ip.InterfaceId || 1,
                AddressAddedDate: format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi }),
                AddressTimeOut: 0
            });
        }
        const addresses = await getNetworkAddressesByAddressType('vpn_white');
        return res.status(201).json({ message: `Imported VPN allowed IPs from file successfully.`, data: addresses });
    } catch (error) {
        return res.status(500).json({ message: 'Import bulk VPN allowed IPs failed', error: error.message });
    }
};
/**
 * Xóa hàng loạt địa chỉ được phép VPN
 *
 * Xóa danh sách địa chỉ được phép VPN từ danh sách IP, lưu vào file CSV, gửi lệnh đến chương trình C và xóa khỏi cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion).
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa và danh sách địa chỉ còn lại.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo và danh sách địa chỉ.
 */
exports.importDeleteBulkVPNAllowedAddresses = async (req, res) => {
    try {
        const ipList = req.body;
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        const ipv4List = ipList.filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV4');
        const ipv6List = ipList.filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV6');
        const toCsvContent = (list) => list.map(ip => `${ip.Address}`).join(os.EOL);
        const ipv4FileName = 'vpn_white_ipv4_delete.csv';
        const ipv6FileName = 'vpn_white_ipv6_delete.csv';
        const ipv4FilePath = path.join(IMPORT_FOLDER, ipv4FileName);
        const ipv6FilePath = path.join(IMPORT_FOLDER, ipv6FileName);
        fs.writeFileSync(ipv4FilePath, toCsvContent(ipv4List), 'utf8');
        fs.writeFileSync(ipv6FilePath, toCsvContent(ipv6List), 'utf8');
        const cmd = `VPN_TABLE_IPV4_REMOVE$${ipv4FilePath}$VPN_TABLE_IPV6_REMOVE$${ipv6FilePath}$`;
        const resCmd = await sendCommandToCProgram(cmd);
        if (!resCmd.includes('OK') && !resCmd.includes('DONE')) {
            return res.status(500).json({ message: `MainC VPN import-delete failed: ${resCmd}` });
        }
        for (const ip of ipList) {
            await deleteNetworkAddressByAddressAndVersion(ip.Address, ip.AddressVersion);
        }
        const addresses = await getNetworkAddressesByAddressType('vpn_white');
        return res.status(200).json({ message: `Bulk deleted VPN allowed IPs from file successfully.`, data: addresses });
    } catch (error) {
        return res.status(500).json({ message: 'Import delete bulk VPN allowed IPs failed', error: error.message });
    }
};
/**
 * Nhập hàng loạt địa chỉ bị chặn
 *
 * Nhập danh sách địa chỉ bị chặn từ danh sách IP, lưu vào file CSV, gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion, Port).
 * @param {object} res - Phản hồi HTTP trả về kết quả nhập và danh sách địa chỉ.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo và danh sách địa chỉ.
 */
exports.importBulkBlockedAddresses = async (req, res) => {
    try {
        const ipList = req.body;
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        const portGroups = {};
        ipList.forEach(ip => {
            const port = ip.Port || '1';
            if (!portGroups[port]) portGroups[port] = [];
            portGroups[port].push(ip);
        });
        let cmds = [];
        for (const port in portGroups) {
            const portNum = port.replace(/^eth/, '');
            const ipv4List = portGroups[port].filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV4');
            const ipv6List = portGroups[port].filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV6');
            const toCsvContent = (list) => list.map(ip => `${ip.Address}`).join(os.EOL);
            const ipv4FileName = `blocked_ipv4_port${port}.csv`;
            const ipv6FileName = `blocked_ipv6_port${port}.csv`;
            const ipv4FilePath = path.join(IMPORT_FOLDER, ipv4FileName);
            const ipv6FilePath = path.join(IMPORT_FOLDER, ipv6FileName);
            fs.writeFileSync(ipv4FilePath, toCsvContent(ipv4List), 'utf8');
            fs.writeFileSync(ipv6FilePath, toCsvContent(ipv6List), 'utf8');
            cmds.push(`PORT${portNum}_BLOCK_IPV4_ADD$${ipv4FilePath}$PORT${portNum}_BLOCK_IPV6_ADD$${ipv6FilePath}$`);
        }
        const cmd = cmds.join('');
        const resCmd = await sendCommandToCProgram(cmd);
        if (!resCmd.includes('OK') && !resCmd.includes('DONE')) {
            return res.status(500).json({ message: `MainC Blocked import failed: ${resCmd}` });
        }
        for (const ip of ipList) {
            await insertNetworkAddress({
                Address: ip.Address,
                AddressVersion: ip.AddressVersion,
                AddressType: 'blocked',
                InterfaceId: 1,
                Port: ip.Port,
                AddressAddedDate: format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi }),
                AddressTimeOut: 0
            });
        }
        const addresses = await getNetworkAddressesByAddressType('blocked');
        return res.status(201).json({ message: `Imported blocked IPs from file successfully.`, data: addresses });
    } catch (error) {
        return res.status(500).json({ message: 'Import bulk blocked IPs failed', error: error.message });
    }
};
/**
 * Xóa hàng loạt địa chỉ bị chặn
 *
 * Xóa danh sách địa chỉ bị chặn từ danh sách IP, lưu vào file CSV, gửi lệnh đến chương trình C và xóa khỏi cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion, Port).
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa và danh sách địa chỉ còn lại.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo và danh sách địa chỉ.
 */
exports.importDeleteBulkBlockedAddresses = async (req, res) => {
    try {
        const ipList = req.body;
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        const portGroups = {};
        ipList.forEach(ip => {
            const port = ip.Port || '1';
            if (!portGroups[port]) portGroups[port] = [];
            portGroups[port].push(ip);
        });
        let cmds = [];
        for (const port in portGroups) {
            const portNum = port.replace(/^eth/, '');
            const ipv4List = portGroups[port].filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV4');
            const ipv6List = portGroups[port].filter(ip => ip.AddressVersion && ip.AddressVersion.toUpperCase() === 'IPV6');
            const toCsvContent = (list) => list.map(ip => `${ip.Address}`).join(os.EOL);
            const ipv4FileName = `blocked_ipv4_port${port}_delete.csv`;
            const ipv6FileName = `blocked_ipv6_port${port}_delete.csv`;
            const ipv4FilePath = path.join(IMPORT_FOLDER, ipv4FileName);
            const ipv6FilePath = path.join(IMPORT_FOLDER, ipv6FileName);
            fs.writeFileSync(ipv4FilePath, toCsvContent(ipv4List), 'utf8');
            fs.writeFileSync(ipv6FilePath, toCsvContent(ipv6List), 'utf8');
            cmds.push(`PORT${portNum}_BLOCK_IPV4_REMOVE$${ipv4FilePath}$PORT${portNum}_BLOCK_IPV6_REMOVE$${ipv6FilePath}$`);
        }
        const cmd = cmds.join('');
        const resCmd = await sendCommandToCProgram(cmd);
        if (!resCmd.includes('OK') && !resCmd.includes('DONE')) {
            return res.status(500).json({ message: `MainC Blocked import-delete failed: ${resCmd}` });
        }
        for (const ip of ipList) {
            await deleteNetworkAddressByAddressVersionAndPort(ip.Address, ip.AddressVersion, ip.Port);
        }
        const addresses = await getNetworkAddressesByAddressType('blocked');
        return res.status(200).json({ message: `Bulk deleted blocked IPs from file successfully.`, data: addresses });
    } catch (error) {
        console.error('importDeleteBulkBlockedAddresses error:', error);
        return res.status(500).json({ message: 'Import delete bulk blocked IPs failed', error: error.message });
    }
};
/**
 * Xóa địa chỉ mạng theo danh sách ID
 *
 * Xóa các địa chỉ mạng dựa trên danh sách ID, gửi lệnh đến chương trình C và xóa khỏi cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách addressIds.
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa và danh sách ID đã xóa.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo, danh sách ID đã xóa và tiến trình.
 */
exports.deleteNetworkAddressesByIds = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : delete network ip addresses by ids ...', config.COLORS.reset);
    let addressIds = [];
    let progressReport = "";
    console.log('body ', req.body);
    if (Array.isArray(req.body.addressIds)) {
        addressIds = req.body.addressIds;
    } else {
        return res.status(400).json({ message: 'No addressId(s) provided', progress: progressReport });
    }
    try {
        let fullCmd = '';
        let deletedIds = [];
        for (const addressId of addressIds) {
            const deletingAddress = await getNetworkAddressesByAddressId(addressId);
            if (!deletingAddress) continue;
            let cmd = '';
            if (deletingAddress.AddressType === 'http_black') {
                if (deletingAddress.AddressVersion && deletingAddress.AddressVersion.toUpperCase() === 'IPV4') {
                    cmd = `REMOVE_IPV4_HTTP$${deletingAddress.Address}$`;
                } else if (deletingAddress.AddressVersion && deletingAddress.AddressVersion.toUpperCase() === 'IPV6') {
                    cmd = `REMOVE_IPV6_HTTP$${deletingAddress.Address}$`;
                }
            } else if (deletingAddress.AddressType === 'vpn_white') {
                if (deletingAddress.AddressVersion && deletingAddress.AddressVersion.toUpperCase() === 'IPV4') {
                    cmd = `REMOVE_IPV4_VPN$${deletingAddress.Address}$`;
                } else if (deletingAddress.AddressVersion && deletingAddress.AddressVersion.toUpperCase() === 'IPV6') {
                    cmd = `REMOVE_IPV6_VPN$${deletingAddress.Address}$`;
                }
            } else if (deletingAddress.AddressType === 'blocked') {
                let portNumber = '';
                if (deletingAddress.Port) {
                    const match = deletingAddress.Port.match(/\d+$/);
                    portNumber = match ? match[0] : deletingAddress.Port;
                }
                let version = deletingAddress.AddressVersion ? deletingAddress.AddressVersion.toUpperCase() : '';
                cmd = `PORT${portNumber}_REMOVE_BLOCK_${version}$${deletingAddress.Address}$`;
            } else if (deletingAddress.AddressType === 'protected') {
                let portNumber = '';
                if (deletingAddress.Port) {
                    const match = deletingAddress.Port.match(/\d+$/);
                    portNumber = match ? match[0] : deletingAddress.Port;
                }
                let version = deletingAddress.AddressVersion ? deletingAddress.AddressVersion.toUpperCase() : '';
                cmd = `PORT${portNumber}_REMOVE_${version}$${deletingAddress.Address}$`;
            }
            if (cmd) {
                fullCmd += cmd;
                deletedIds.push(addressId);
            }
        }
        if (!fullCmd) {
            return res.status(404).json({ message: 'No valid addresses found to delete', progress: progressReport });
        }
        const result = await sendCommandToCProgram(fullCmd);
        const isError = config.isProduction ? result.includes("ERROR") : result.includes("ERROR");
        if (isError) {
            return res.status(500).json({ message: 'Failed to execute command in mainC', error: result });
        } else {
            progressReport += "mainC updated";
        }
        for (const addressId of deletedIds) {
            await deleteNetworkAddress(addressId);
        }
        progressReport += " database updated";
        return res.status(200).json({ message: 'Delete address(es) successfully', deleted: deletedIds, progress: progressReport });
    } catch (error) {
        return res.status(500).json({ message: 'Delete address failed', error: error.message, progress: progressReport });
    }
};
/**
 * Xóa địa chỉ mạng theo danh sách địa chỉ và phiên bản
 *
 * Xóa các địa chỉ mạng dựa trên danh sách địa chỉ và phiên bản IP, gửi lệnh đến chương trình C và xóa khỏi cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa danh sách IP (Address, AddressVersion).
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa và số lượng địa chỉ đã xóa.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông báo, số lượng đã xóa và tiến trình.
 */
exports.deleteNetworkAddressesByAddressAndVersionList = async (req, res) => {
    config.progressLog(config.COLORS.cyan, '[PROGRESS] : delete network ip address by address and version list ...', config.COLORS.reset);
    let progressReport = "";
    try {
        const ipList = req.body;
        console.log('ipList ', ipList);
        if (!Array.isArray(ipList) || ipList.length === 0) {
            return res.status(400).json({ message: 'Invalid IP list' });
        }
        let configPackage = '';
        const validIps = [];
        for (const ip of ipList) {
            if (!ip.Address || !ip.AddressVersion) {
                await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Bulk Delete Network Address',
                    `Invalid IP: ${ip.Address || 'undefined'} (${ip.AddressVersion || 'undefined'})`, 'Failed', 'Missing Address or AddressVersion');
                continue;
            }
            if (ip.AddressVersion.toUpperCase() === 'IPV4') {
                configPackage += `REMOVE_IPV4_VPN$${ip.Address}$`;
            } else if (ip.AddressVersion.toUpperCase() === 'IPV6') {
                configPackage += `REMOVE_IPV6_VPN$${ip.Address}$`;
            } else {
                await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Bulk Delete Network Address',
                    `Invalid IP version: ${ip.Address} (${ip.AddressVersion})`, 'Failed', 'Invalid AddressVersion');
                continue;
            }
            validIps.push(ip);
        }
        if (!configPackage) {
            return res.status(400).json({ message: 'No valid IPs to delete' });
        }
        const result = await sendCommandToCProgram(configPackage);
        const isError = config.isProduction ? result.includes("ERROR") : result.includes("ERROR");
        if (isError) {
            await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Bulk Delete Network Address',
                `Delete network addresses`, 'Failed', `C error: ${result}`);
            return res.status(500).json({ message: 'Failed to execute command in mainC', error: result, progress: progressReport });
        } else {
            progressReport += "mainC updated";
        }
        for (const ip of validIps) {
            await deleteNetworkAddressByAddressAndVersion(ip.Address, ip.AddressVersion);
            await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Bulk Delete Network Address',
                `Delete network address: ${ip.Address} (${ip.AddressVersion})`, 'Success', null);
        }
        progressReport += " database updated";
        return res.status(200).json({ message: 'Bulk delete by Address+Version success', deleted: validIps.length, progress: progressReport });
    } catch (error) {
        console.error('Bulk delete by Address+Version failed:', error);
        await insertSystemLogToDatabase(req.user.payload.Id, 'NetworkAddress', 'Config', 'Bulk Delete Network Address',
            `Delete network addresses`, 'Failed', error.message);
        return res.status(500).json({ message: 'Bulk delete by Address+Version failed', error: error.message, progress: progressReport });
    }
};