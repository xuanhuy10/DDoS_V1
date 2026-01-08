const bcrypt = require('bcrypt');
const jwt = require('jsonwebtoken');
const { format } = require('date-fns');
const { vi } = require('date-fns/locale');
const { getUserByUsername, updateUser } = require('../models/Users.model');
const { insertSystemLogToDatabase } = require('../helper/dbo/logs.helper');

/**
 * Đăng nhập người dùng
 *
 * Xác thực người dùng dựa trên tên đăng nhập và mật khẩu, tạo JWT token và refresh token, cập nhật thời gian đăng nhập cuối.
 * @param {object} req - Yêu cầu HTTP chứa username và password.
 * @param {object} res - Phản hồi HTTP trả về token và thông báo.
 * @returns {Promise<object>} - Phản hồi JSON chứa access token và thông báo.
 */
exports.login = async (req, res) => {
    try {
        const { username, password } = req.body;
        if (!username || !password) {
            return res.status(401).json({ message: 'Please enter username and password' });
        }
        const user = await getUserByUsername(username);
        if (!user) {
            await insertSystemLogToDatabase(null, 'Auth', 'System', 'Login', `Login account: ${username}`, 'Failure', 'User not found');
            return res.status(401).json({ message: 'User not found' });
        }
        const match = await bcrypt.compare(password, user.Password);
        if (!match) {
            await insertSystemLogToDatabase(null, 'Auth', 'System', 'Login', `Login account: ${username}`, 'Failure', 'Password is incorrect');
            return res.status(401).json({ message: 'Password is incorrect' });
        }
        user.LastLogin = format(new Date(), 'yy/MM/dd HH:mm:ss', {locale: vi});
        await updateUser(user.UserId, user);
        const payload = {
            Id: user.UserId,
            UserName: username,
            Role: user.Role
        };
        const access_token = jwt.sign({ payload }, process.env.SECRET_TOKEN, { expiresIn: '2h' });
        const refresh_token = jwt.sign({ payload }, process.env.SECRET_REFRESH, { expiresIn: '1d' });
        res.cookie('rnetDef', refresh_token, { httpOnly: true, secure: false, sameSite: 'Lax', maxAge: 24 * 60 * 60 * 1000 });
        await insertSystemLogToDatabase(null, 'Auth', 'System', 'Login', `Login account: ${username}`, 'Success', null);
        return res.status(200).json({ status: 'Success', message: 'Login successful', access_token });
    } catch (error) {
        console.log('error ', error);
        await insertSystemLogToDatabase(null, 'Auth', 'System', 'Login', `Login`, 'Failure', error.message);
        return res.status(500).json({ message: 'Login failed' });
    }
};
/**
 * Làm mới token
 *
 * Xác thực refresh token và tạo access token mới nếu hợp lệ.
 * @param {object} req - Yêu cầu HTTP chứa refresh token trong cookie.
 * @param {object} res - Phản hồi HTTP trả về access token mới.
 * @returns {Promise<object>} - Phản hồi JSON chứa access token và thông báo.
 */
exports.refreshToken = (req, res) => {
    const token = req.cookies.rnetDef;
    if (!token) {
        return res.status(401).json({ message: 'Token is required' });
    }
    jwt.verify(token, process.env.SECRET_REFRESH, (err, user) => {
        if (err && err.name === 'TokenExpiredError') {
            return res.status(403).json({ message: 'Refresh token expired' });
        }
        if (err) {
            return res.status(403).json({ message: 'Invalid refresh token' });
        }
        const access_token = jwt.sign({ payload: user.payload }, process.env.SECRET_TOKEN, { expiresIn: '2h' });
        return res.status(200).json({ status: 'Success', message: 'Token refreshed successfully', access_token });
    });
};