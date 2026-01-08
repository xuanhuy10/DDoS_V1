// environment variables
require('dotenv').config();

const express = require('express');
const { createServer } = require('http');
const { Server } = require('socket.io');
const cookieParser = require('cookie-parser');

const session = require('cookie-session');
const compression = require('compression');
const cors = require('cors');

const app = express();
const httpServer = createServer(app); // Tạo server HTTP từ Express
const io = new Server(httpServer, {
      cors: {
            origin: "*",
            methods: ["GET", "POST"]
      }
});
io.listen(3002);

app.use(cors(
      {
            credentials: true,
            origin: ['http://localhost:5173', 'http://172.20.52.48:5173', 'http://172.20.233.197:5173','http://10.3.0.113:5173', 'http://10.3.0.124:5173']
      }
));

app.use(express.urlencoded({ extended: true }));
app.use(express.json());
app.use(cookieParser());
app.use(session({
      secret: process.env.SESSION_SECRET, 
      resave: true,
      saveUninitialized: true
}));
app.use(compression());

const config = require('./config');

const { packetCapture } = require('./services/packetCap.service');

//routes
const authRoutes = require('./routes/Auth.route');
const defenseProfilesRoutes = require('./routes/DefenseProfiles.route');
const deviceInterfacesRoutes = require('./routes/DeviceInterfaces.route');
const deviceLogsRoutes = require('./routes/DeviceLogs.route');
const deviceSettingsRoutes = require('./routes/DeviceSettings.route');
const ipsecRoutes = require('./routes/IPSec.route');
const networkAddressesRoutes = require('./routes/NetworkAddresses.route');
const networkAnomaliesRoutes = require('./routes/NetworkAnomalies.route');
// const notificationRoutes = require('./routes/Notification.route');
const usersRoutes = require('./routes/Users.route');

app.use('/v1/auth', authRoutes);
app.use('/v1/defense/profiles', defenseProfilesRoutes);
app.use('/v1/defense/interfaces', deviceInterfacesRoutes);
app.use('/v1/logs', deviceLogsRoutes);
app.use('/v1/device', deviceSettingsRoutes);
app.use('/v1/defense/ipsec', ipsecRoutes);
app.use('/v1/defense/address', networkAddressesRoutes);
app.use('/v1/network', networkAnomaliesRoutes);
// app.use('/v1/notification', notificationRoutes);
app.use('/v1/users', usersRoutes);

//Start continuous packet processing
packetCapture(io);

//Start continuous disk space monitoring
// setInterval(async () => {
//       await autoDeleteLog(io);
// }, 35 * 60 * 1000); // 15 minutes

const PORT = config.port || 3000;
httpServer.listen(PORT, () => {
      console.log(`Server is running on: http://localhost:${PORT}`);
});
            