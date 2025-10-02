const { check, body } = require('express-validator');

const updateDefenderValidator = [
    // Check if 'Layer' is in the allowed values
    body('Layer')
        .notEmpty()
        .isIn(['network', 'transport', 'application', 'session', 'presentation'])
        .withMessage("Invalid Layer"),

    body('ProtectionMode')
        .notEmpty()
        .isIn(['IP', 'PORT'])
        .withMessage("Protection Mode must be 'IP Address' or 'PORT'"),

    body('IPv4Protected_1')
        .if(body('ProtectionMode').equals('IP'))
        .isInt({ min: 1, max: 255 })
        .withMessage('IPv4 address is invalid'),

    body('IPv4Protected_2')
        .if(body('ProtectionMode').equals('IP'))
        .isInt({ min: 0, max: 255 })
        .withMessage('IPv4 address is invalid'),

    body('IPv4Protected_3')
        .if(body('ProtectionMode').equals('IP'))
        .isInt({ min: 0, max: 255 })
        .withMessage('IPv4 address is invalid'),

    body('IPv4Protected_4')
        .if(body('ProtectionMode').equals('IP'))
        .isInt({ min: 0, max: 255 })
        .withMessage('IPv4 address is invalid'),

    body('IPv6Protected')
        .if(body('ProtectionMode').equals('IP'))
        .isIP(6)
        .withMessage('IPv6 address is invalid'),

    body('AttackDetectionTime')
        .notEmpty()
        .isInt({ min: 1, max: 180 })
        .withMessage('Detection time is invalid'),

    //NETWORK LAYER
    body('ICMPFloodEnable')
        .if(body('Layer').equals('network'))
        .isIn(['0', '1'])
        .withMessage('ICMP Flood must be disabled or enabled'),

    body('ICMPFloodThreshold')
        .if(body('ICMPFloodEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt({ min: 1, max: 65535 })
        .withMessage('ICMP Flood threshold is invalid'),

    body('ICMPFloodRate')
        .if(body('ICMPFloodEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt()
        .withMessage('ICMP Flood rate is invalid'),

    body('TCPFragmentEnable')
        .if(body('Layer').equals('network'))
        .notEmpty()
        .isIn(['0', '1'])
        .withMessage('TCP Fragments must be disabled or enabled'),

    body('UDPFragmentEnable')
        .if(body('Layer').equals('network'))
        .notEmpty()
        .isIn(['0', '1'])
        .withMessage('UDP Fragments must be disabled or enabled'),

    //TRANSPORT LAYER
    body('SYNFloodEnable')
        .if(body('Layer').equals('transport'))
        .notEmpty()
        .isIn(['0', '1'])
        .withMessage('SYN Flood must be disabled or enabled'),

    body('SYNFloodSYNThreshold')
        .if(body('SYNFloodEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt({ min: 1, max: 65535 })
        .withMessage('SYN Flood threshold is invalid'),

    body('SYNFloodACKThreshold')
        .if(body('SYNFloodEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt({ min: 1, max: 65535 })
        .withMessage('SYN Flood rate is invalid'),

    body('SYNFloodTimeout')
        .if(body('SYNFloodEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt({ min: 1, max: 43200 })
        .withMessage('SYN Flood timeout is invalid'),

    body('UDPFloodEnable')
        .if(body('Layer').equals('transport'))
        .notEmpty()
        .isIn(['0', '1'])
        .withMessage('UDP Flood must be disabled or enabled'),

    body('UDPFloodThreshold')
        .if(body('UDPFloodEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt({ min: 1, max: 65535 })
        .withMessage('UDP Flood threshold is invalid'),

    body('UDPFloodRate')
        .if(body('UDPFloodEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt()
        .withMessage('UDP Flood rate is invalid'),

    body('DNSFloodEnable')
        .if(body('Layer').equals('transport'))
        .notEmpty()
        .isIn(['0', '1'])
        .withMessage('DNS Flood must be disabled or enabled'),

    body('DNSFloodThreshold')
        .if(body('DNSFloodEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt({ min: 1, max: 65535 })
        .withMessage('DNS Flood threshold is invalid'),

    body('LandAttackEnable')
        .if(body('Layer').equals('transport'))
        .notEmpty()
        .isIn(['0', '1'])
        .withMessage('Land Attack must be disabled or enabled'),

    //session layer
    body('IPSecIKEEnable')
        .if(body('Layer').equals('session'))
        .notEmpty()
        .isIn(['0', '1'])
        .withMessage('IPSec IKE must be disabled or enabled'),

    body('IPSecIKEThreshold')
        .if(body('IPSecIKEEnable').custom((value) => value.includes('1')))
        .notEmpty()
        .isInt({ min: 1, max: 65535 })
        .withMessage('IPSec IKE threshold is invalid'),

    body('HTTPFloodEnable')
        .if(body('Layer').equals('application'))
        .notEmpty()
        .isIn(['0', '1'])
        .withMessage('HTTP Flood must be disabled or enabled'),

];

const addVPNAddress = [
    body('type')
        .notEmpty()
        .isIn(['IPv4', 'IPv6'])
        .withMessage('Invalid IP Address type'),
    
    body('serverAddress')
        .notEmpty()
        .if(body('type').equals('IPv4'))
        .isIP(4)
        .withMessage('Invalid IPv4 Address'),

    body('serverAddress')
        .notEmpty()
        .if(body('type').equals('IPv6'))
        .isIP(6)
        .withMessage('Invalid IPv6 Address'),
];

const removeVPNAddress = [
    body('serverAddress')
        .notEmpty()
        .isIP()
        .withMessage('Invalid IP Address')
];

const addAttackIP = [
    body('type')
        .notEmpty()
        .isIn(['IPv4', 'IPv6'])
        .withMessage('Invalid IP Address type'),
    
    body('serverAddress')
        .notEmpty()
        .if(body('type').equals('IPv4'))
        .isIP(4)
        .withMessage('Invalid IPv4 Address'),

    body('serverAddress')
        .notEmpty()
        .if(body('type').equals('IPv6'))
        .isIP(6)
        .withMessage('Invalid IPv6 Address'),
];

const removeAttackIP = [
    body('serverAddress')
        .notEmpty()
        .isIP()
        .withMessage('Invalid IP Address')
];

let defenderValidator = {
    update: updateDefenderValidator,
    addVPNAddress: addVPNAddress,
    removeVPNAddress: removeVPNAddress,
    addAttackIP: addAttackIP,
    removeAttackIP:removeAttackIP
};

module.exports = defenderValidator;
