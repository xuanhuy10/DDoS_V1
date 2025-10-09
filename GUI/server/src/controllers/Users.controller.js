const bcrypt = require('bcrypt');
const { format } = require('date-fns');
const { vi } = require('date-fns/locale');
const { insertSystemLogToDatabase } = require('../helper/dbo/logs.helper');
const { sendCommandToCProgram } = require('../services/socket.service');
const {
  getAllUsers,
  getUserByUserId,
  getUserByUsername,
  getUserByEmail,
  insertUser,
  updateUser,
  deleteUser
} = require('../models/Users.model');

/**
 * Lấy danh sách tất cả người dùng
 *
 * Truy vấn cơ sở dữ liệu để lấy danh sách tất cả người dùng trong hệ thống.
 * @param {object} req - Yêu cầu HTTP.
 * @param {object} res - Phản hồi HTTP trả về danh sách người dùng.
 * @returns {Promise<object>} - Phản hồi JSON chứa danh sách người dùng.
 */
exports.getAllUsers = async (req, res) => {
  try {
    const users = await getAllUsers();
    return res.status(200).json({ data: users });
  } catch (error) {
    console.log('error ', error);
    return res.status(500).json({ message: 'Get all users failed' });
  }
};
/**
 * Lấy thông tin người dùng theo ID
 *
 * Truy vấn cơ sở dữ liệu để lấy thông tin người dùng dựa trên ID được cung cấp.
 * @param {object} req - Yêu cầu HTTP chứa userId trong params.
 * @param {object} res - Phản hồi HTTP trả về thông tin người dùng.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin người dùng hoặc thông báo lỗi.
 */
exports.getUserById = async (req, res) => {
  const userId = req.params.userId;
  try {
    const user = await getUserByUserId(userId);
    if (!user) {
      return res.status(404).json({ message: 'User not found' });
    }
    return res.status(200).json({ data: user });
  } catch (error) {
    console.log('error ', error);
    return res.status(500).json({ message: 'Get user info failed' });
  }
};
/**
 * Lấy thông tin người dùng đang đăng nhập
 *
 * Truy vấn cơ sở dữ liệu để lấy thông tin người dùng dựa trên ID từ token JWT.
 * @param {object} req - Yêu cầu HTTP chứa thông tin người dùng từ token.
 * @param {object} res - Phản hồi HTTP trả về thông tin người dùng.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin người dùng hoặc thông báo lỗi.
 */
exports.getLoggedInUser = async (req, res) => {
  try {
    const userId = req.user.payload.Id;
    const user = await getUserByUserId(userId);
    if (!user) {
      return res.status(404).json({ message: 'User not found' });
    }
    user.Password = undefined;
    return res.status(200).json({ data: user });
  } catch (error) {
    console.log('error ', error);
    return res.status(500).json({ message: 'Get user info failed' });
  }
};
/**
 * Thêm người dùng mới
 *
 * Thêm một người dùng mới vào hệ thống sau khi kiểm tra các điều kiện (tên không chứa khoảng trắng, mật khẩu khớp, không trùng tên hoặc email, giới hạn người dùng).
 * Gửi lệnh đến chương trình C và lưu vào cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa thông tin người dùng (Username, Password, ConfirmPassword, Email).
 * @param {object} res - Phản hồi HTTP trả về kết quả thêm người dùng.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin người dùng mới và thông báo.
 */
exports.insertUser = async (req, res) => {
  const user = req.body;
  try {
    if (/\s/.test(user.Username)) {
      return res.status(400).json({ message: 'Username must not contain spaces' });
    }
    if (user.Password !== user.ConfirmPassword) {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          'User',
          'System',
          'Create User',
          `Add user: ${user.Username} to system`,
          'Failed',
          'Passwords and confirm password do not match'
      );
      return res.status(400).json({ message: 'Passwords do not match' });
    }
    const existedUser = await getUserByUsername(user.Username);
    if (existedUser) {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          'User',
          'System',
          'Create User',
          `Add user: ${user.Username} to system`,
          'Failed',
          'Username already exists'
      );
      return res.status(400).json({ message: 'Username already exists' });
    }
    const existedEmail = await getUserByEmail(user.Email);
    if (existedEmail) {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          'User',
          'System',
          'Create User',
          `Add user: ${user.Username} to system`,
          'Failed',
          'Email already exists'
      );
      return res.status(400).json({ message: 'Email already exists' });
    }
    const allUsers = await getAllUsers();
    if (allUsers.length >= 6) {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          'User',
          'System',
          'Create User',
          `Add user: ${user.Username} to system`,
          'Failed',
          'User limit reached'
      );
      return res.status(400).json({ message: 'User limit reached' });
    }
    const configPackage = `ADD_USER$${user.Username}$ADD_PW$${user.Password}$`;
    const configResult = await sendCommandToCProgram(configPackage);
    if (configResult.includes('ERROR')) {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          'User',
          'System',
          'Create User',
          `Add user: ${user.Username} to system`,
          'Failed',
          'Add user failed'
      );
      return res.status(500).json({ message: 'Add user failed' });
    }
    user.Password = await bcrypt.hash(user.Password, 10);
    user.Role = 'User';
    user.LastLogin = null;
    user.CreateTime = format(new Date(), 'yyyy/MM/dd HH:mm:ss', { locale: vi });
    user.NotifyNetworkAnomalyDetect = 1;
    user.NotifyDDoSAttackDetect = 1;
    user.NotifyDDoSAttackEnd = 1;
    user.NotifyDiskExceeds = 1;
    const newUser = await insertUser(user);
    await insertSystemLogToDatabase(
        req.user.payload.Id,
        'User',
        'System',
        'Create User',
        `Add user: ${user.Username} to system`,
        'Success',
        null
    );
    return res.status(201).json({ data: newUser, message: 'Insert user successfully' });
  } catch (error) {
    console.log('error ', error);
    await insertSystemLogToDatabase(
        req.user.payload.Id,
        'User',
        'System',
        'Create User',
        `Add user: ${user.Username} to system`,
        'Failed',
        error.message
    );
    return res.status(500).json({ message: 'Insert user failed' });
  }
};
/**
 * Cập nhật thông tin người dùng
 *
 * Cập nhật thông tin người dùng dựa trên ID, bao gồm kiểm tra mật khẩu nếu thay đổi và đồng bộ với chương trình C.
 * @param {object} req - Yêu cầu HTTP chứa userId trong params và thông tin người dùng trong body.
 * @param {object} res - Phản hồi HTTP trả về kết quả cập nhật.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin người dùng đã cập nhật và thông báo.
 */
exports.updateUser = async (req, res) => {
  const userId = req.params.userId;
  const user = req.body;
  try {
    const existedUser = await getUserByUserId(userId);
    if (!existedUser) {
      return res.status(404).json({ message: 'User not found' });
    }
    if (user.Password) {
      if (!user.CurrentPassword) {
        return res.status(400).json({ message: 'Current password is required' });
      }
      const isMatch = await bcrypt.compare(user.CurrentPassword, existedUser.Password);
      if (!isMatch) {
        return res.status(400).json({ message: 'Current password is incorrect' });
      }
      if (user.Password !== user.ConfirmPassword) {
        await insertSystemLogToDatabase(
            req.user.payload.Id,
            'User',
            'System',
            'Update User',
            `Update user: ${user.Username} to system`,
            'Failed',
            'Passwords and confirm password do not match'
        );
        return res.status(400).json({ message: 'Passwords do not match' });
      }
      const configPackage = `CHANGE_USER$${existedUser.Username}$CHANGE_PASS$${user.Password}$`;
      const configResult = await sendCommandToCProgram(configPackage);
      if (configResult.includes('ERROR')) {
        await insertSystemLogToDatabase(
            req.user.payload.Id,
            'User',
            'System',
            'Update User',
            `Update user: ${user.Username} to system`,
            'Failed',
            'Update user failed'
        );
        return res.status(500).json({ message: 'Update user failed' });
      }
      user.Password = await bcrypt.hash(user.Password, 10);
    } else {
      user.Password = existedUser.Password;
    }
    user.Username = user.Username || existedUser.Username;
    user.Email = user.Email || existedUser.Email;
    user.Role = user.Role || existedUser.Role;
    user.NotifyNetworkAnomalyDetect = user.NotifyNetworkAnomalyDetect ?? existedUser.NotifyNetworkAnomalyDetect;
    user.NotifyDDoSAttackDetect = user.NotifyDDoSAttackDetect ?? existedUser.NotifyDDoSAttackDetect;
    user.NotifyDDoSAttackEnd = user.NotifyDDoSAttackEnd ?? existedUser.NotifyDDoSAttackEnd;
    user.NotifyDiskExceeds = user.NotifyDiskExceeds ?? existedUser.NotifyDiskExceeds;
    user.LastLogin = existedUser.LastLogin || null;
    user.CreateTime = existedUser.CreateTime || null;
    const updatedUser = await updateUser(userId, user);
    await insertSystemLogToDatabase(
        req.user.payload.Id,
        'User',
        'System',
        'Update User',
        `Update user: ${user.Username} to system`,
        'Success',
        null
    );
    return res.status(200).json({ data: updatedUser, message: 'Update user successfully' });
  } catch (error) {
    console.log('error ', error);
    await insertSystemLogToDatabase(
        req.user.payload.Id,
        'User',
        'System',
        'Update User',
        `Update user: ${user.Username} to system`,
        'Failed',
        error.message
    );
    return res.status(500).json({ message: 'Update user failed' });
  }
};
/**
 * Xóa người dùng
 *
 * Xóa người dùng dựa trên ID, gửi lệnh đến chương trình C và xóa khỏi cơ sở dữ liệu.
 * @param {object} req - Yêu cầu HTTP chứa userId trong params.
 * @param {object} res - Phản hồi HTTP trả về kết quả xóa.
 * @returns {Promise<object>} - Phản hồi JSON chứa thông tin người dùng đã xóa và thông báo.
 */
exports.deleteUser = async (req, res) => {
  const userId = req.params.userId;
  try {
    const existedUser = await getUserByUserId(userId);
    if (!existedUser) {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          'User',
          'System',
          'Delete User',
          `Delete user: ${userId} from system`,
          'Failed',
          'User not found'
      );
      return res.status(404).json({ message: 'User not found' });
    }
    const configPackage = `REMOVE_USER$${existedUser.Username}$`;
    const configResult = await sendCommandToCProgram(configPackage);
    if (configResult.includes('ERROR')) {
      await insertSystemLogToDatabase(
          req.user.payload.Id,
          'User',
          'System',
          'Remove User',
          `Remove user: ${existedUser.Username} from system`,
          'Failed',
          'Remove user failed'
      );
      return res.status(500).json({ message: 'Remove user failed' });
    }
    const deletedUser = await deleteUser(userId);
    await insertSystemLogToDatabase(
        req.user.payload.Id,
        'User',
        'System',
        'Delete User',
        `Delete user: ${userId} from system`,
        'Success',
        null
    );
    return res.status(200).json({ data: deletedUser, message: 'Delete user successfully' });
  } catch (error) {
    console.log('error ', error);
    await insertSystemLogToDatabase(
        req.user.payload.Id,
        'User',
        'System',
        'Delete User',
        `Delete user: ${userId} from system`,
        'Failed',
        error.message
    );
    return res.status(500).json({ message: 'Delete user failed' });
  }
};