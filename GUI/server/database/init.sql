-- Active: 1745285098017@@127.0.0.1@3306
CREATE TABLE DefenseProfiles
(
    DefenseProfileId INTEGER PRIMARY KEY,
    UserId INTEGER,
    DefenseProfileName TEXT NOT NULL,
    DefenseProfileDescription TEXT NOT NULL,
    DefenseProfileCreateTime DATETIME NOT NULL,
    DefenseProfileLastModified DATETIME NOT NULL,
    DefenseProfileUsingTime DATETIME,
    DefenseProfileType TEXT NOT NULL,
    DefenseProfileStatus TEXT NOT NULL,
    DetectionTime INTEGER NOT NULL DEFAULT 1,
    DefenseMode TEXT NOT NULL DEFAULT 'Aggregate',
    ICMPFloodEnable BOOLEAN NOT NULL DEFAULT 1,
    ICMPFloodThreshold INTEGER NOT NULL DEFAULT 1000,
    ICMPFloodRate INTEGER NOT NULL DEFAULT 1000,
    SYNFloodEnable BOOLEAN NOT NULL DEFAULT 1,
    SYNFloodSYNThreshold INTEGER NOT NULL DEFAULT 1000,
    SYNFloodACKThreshold INTEGER NOT NULL DEFAULT 1000,
    SYNFloodWhiteListTimeOut INTEGER NOT NULL DEFAULT 20,
    UDPFloodEnable BOOLEAN NOT NULL DEFAULT 1,
    UDPFloodThreshold INTEGER NOT NULL DEFAULT 1000,
    UDPFloodRate INTEGER NOT NULL DEFAULT 1000,
    DNSFloodEnable BOOLEAN NOT NULL DEFAULT 1,
    DNSFloodThreshold INTEGER NOT NULL DEFAULT 1000,
    LandAttackEnable BOOLEAN NOT NULL DEFAULT 1,
    IPSecIKEEnable BOOLEAN NOT NULL DEFAULT 1,
    IPSecIKEThreshold INTEGER NOT NULL DEFAULT 1000,
    TCPFragmentEnable BOOLEAN NOT NULL DEFAULT 1,
    UDPFragmentEnable BOOLEAN NOT NULL DEFAULT 1,
    HTTPFloodEnable BOOLEAN NOT NULL DEFAULT 1,
    HTTPSFloodEnable BOOLEAN NOT NULL DEFAULT 1,
    FOREIGN KEY (UserId) REFERENCES Users (UserId)
);

CREATE TABLE Users
(
    UserId INTEGER PRIMARY KEY,
    UserFullName TEXT NOT NULL,
    Username TEXT NOT NULL,
    Password TEXT NOT NULL,
    Role TEXT NOT NULL,
    LastLogin DATETIME,
    CreateTime DATETIME NOT NULL,
    Email TEXT NOT NULL,
    NotifyNetworkAnomalyDetect BOOLEAN NOT NULL DEFAULT 1,
    NotifyDDoSAttackDetect BOOLEAN NOT NULL DEFAULT 1,
    NotifyDDoSAttackEnd BOOLEAN NOT NULL DEFAULT 1,
    NotifyDiskExceeds BOOLEAN NOT NULL DEFAULT 1
);

CREATE TABLE NetworkAddresses
(
    AddressId INTEGER PRIMARY KEY,
    InterfaceId INTEGER,
    Address TEXT NOT NULL,
    AddressVersion TEXT NOT NULL,
    AddressType TEXT NOT NULL,
    AddressAddedDate DATETIME NOT NULL,
    AddressTimeOut DATETIME NOT NULL,
    FOREIGN KEY (InterfaceId) REFERENCES DeviceInterfaces (InterfaceId)
);

CREATE TABLE NetworkAttack
(
    AttackDescriptionId INTEGER PRIMARY KEY,
    AttackName TEXT NOT NULL,
    AttackType TEXT NOT NULL,
    AttackProtocol TEXT NOT NULL,
    AttackDescription TEXT
);

CREATE TABLE DeviceInterfaces
(
    InterfaceId INTEGER PRIMARY KEY,
    DefenseProfileId INTEGER,
    InterfaceName TEXT NOT NULL,
    InterfaceType TEXT NOT NULL,
    InterfaceStatus TEXT NOT NULL DEFAULT 'Up',
    InterfaceDescription TEXT,
    InterfaceProtectionMode TEXT NOT NULL DEFAULT 'Port',
    InterfaceIsMonitoring BOOLEAN NOT NULL DEFAULT 0,
    InterfaceIsMirroring BOOLEAN NOT NULL DEFAULT 0,
    InterfaceToMonitorInterfaceId INTEGER,
    InterfaceMirrorSetting TEXT,
    FOREIGN KEY (DefenseProfileId) REFERENCES DefenseProfiles (DefenseProfileId),
    FOREIGN KEY (InterfaceToMonitorInterfaceId) REFERENCES DeviceInterfaces (InterfaceId)
);

CREATE TABLE DeviceLogs
(
    LogId INTEGER PRIMARY KEY,
    UserId INTEGER,
    LogSource TEXT NOT NULL,
    LogType TEXT NOT NULL,
    LogActionTime DATETIME,
    LogActionContent TEXT NOT NULL,
    LogActionDetail TEXT NOT NULL,
    LogActionResult TEXT NOT NULL,
    LogActionResultDetail TEXT,
    FOREIGN KEY (UserId) REFERENCES Users (UserId)
);

CREATE TABLE DeviceSettings
(
    DeviceSettingId INTEGER PRIMARY KEY,
    DeviceMaxUser INTEGER NOT NULL DEFAULT 6,
    DeviceAutoDeleteLogThreshold INTEGER NOT NULL,
    DeviceAutoDeleteLogEnable BOOLEAN NOT NULL DEFAULT 1,
    DeviceAutoRotationLogs BOOLEAN NOT NULL DEFAULT 1,
    DeviceAutoDeleteActivityInterval INTEGER NOT NULL,
    DeviceAutoDeleteActivity BOOLEAN NOT NULL DEFAULT 1
);

CREATE TABLE NetworkAnomalies
(
    AnomaliesId INTEGER PRIMARY KEY,
    Anomalies TEXT NOT NULL,
    AnomaliesTargetIp TEXT NOT NULL,
    AnomaliesTargetPort INTEGER NOT NULL,
    AnomaliesStats TEXT NOT NULL,
    AnomaliesStart DATETIME NOT NULL,
    AnomaliesEnd DATETIME,
    AnomaliesStatus BOOLEAN NOT NULL DEFAULT 1
);

CREATE TABLE Notifications
(
    NotificationId INTEGER PRIMARY KEY,
    UserId INTEGER,
    NotificationType TEXT NOT NULL,
    NotificationContent TEXT NOT NULL,
    NotificationTime DATETIME NOT NULL,
    NotificationIsRead BOOLEAN NOT NULL DEFAULT 1,
    FOREIGN KEY (UserId) REFERENCES Users (UserId)
);


INSERT INTO "DeviceInterfaces"
VALUES
    (1, NULL, 'eth1', 'inbound', 'Up', 'Main network interface', 'Port', 0, 1, 6, NULL),
    (2, NULL, 'eth2', 'outbound', 'Up', 'Secondary network interface', 'Port', 0, 0, NULL, NULL),
    (3, NULL, 'eth3', 'inbound', 'Down', 'Backup network interface', 'Port', 0, 0, NULL, NULL),
    (4, NULL, 'eth4', 'outbound', 'Up', 'Management network interface', 'Port', 0, 0, NULL, NULL),
    (5, NULL, 'eth5', 'inbound', 'Up', 'DMZ network interface', 'Port', 0, 0, NULL, NULL),
    (6, NULL, 'eth6', 'outbound', 'Down', 'VPN network interface', 'Port', 1, 0, NULL, NULL),
    (7, NULL, 'eth7', 'outbound', 'Down', 'VPN network interface', 'Port', 0, 0, NULL, NULL),
    (8, NULL, 'eth8', 'outbound', 'Down', 'VPN network interface', 'Port', 0, 0, NULL, NULL);

INSERT INTO "Users"
VALUES
    (1, 'admin', 'admin', '$2a$10$N6jbzwGIFQ7uPWT/JOU8aeWudjCc8s4EBnWW.QSo99MiJMlgxqPGq', 'admin', NULL, '2023/11/01 10:00:00', 'cthien3602@gmail.com', 1, 1, 1, 1);

INSERT INTO "DefenseProfiles"
VALUES
    (1, NULL, 'DEFAULT', 'Default profile for DDoS device', '2023/11/01 10:00:00', '2023/11/01 10:00:00',
        NULL, 'SystemProfile', 'Active', 1, 'Aggregate', 1, 1000, 1000, 1, 1000, 1000, 20, 1, 1000, 1000, 1, 1000, 1, 1, 1000, 1, 1, 1, 1);

INSERT INTO "DeviceSettings"
VALUES
    (1, 6, 50, 1, 1, 1, 1);

