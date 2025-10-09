// const { updateConfigMode } = require('../modelsv2/ModeConfig.model');

const mapConfig = new Map();
mapConfig.set('PROTECT', 'ProtectionMode');
mapConfig.set('PROTECT_IP', 'IPv4Protected');
mapConfig.set('PROTECT_IPV6', 'IPv6Protected');
mapConfig.set('PROTECT_PORT', 'PortProtected');
mapConfig.set('TIME_DETECT_ATTACK', 'DetectionTime');
mapConfig.set('ICMP_EN_DIS', 'ICMPFloodEnable');
mapConfig.set('ICMP_THR', 'ICMPFloodThreshold');
mapConfig.set('ICMP_THR_PS', 'ICMPFloodRate');
mapConfig.set('UDP_FRA_EN_DIS', 'UDPFragmentEnable');
mapConfig.set('TCP_FRA_EN_DIS', 'TCPFragmentEnable');
mapConfig.set('SYN_EN_DIS', 'SYNFloodEnable');
mapConfig.set('SYN_THR', 'SYNFloodSYNThreshold');
mapConfig.set('ACK_THR', 'SYNFloodACKThreshold');
mapConfig.set('TIME_WHITE_LIST', 'SYNFloodWhiteListTimeOut');
mapConfig.set('UDP_EN_DIS', 'UDPFloodEnable');
mapConfig.set('UDP_THR', 'UDPFloodThreshold');
mapConfig.set('UDP_THR_PS', 'UDPFloodRate');
mapConfig.set('DNS_EN_DIS', 'DNSFloodEnable');
mapConfig.set('DNS_THR', 'DNSFloodThreshold');
mapConfig.set('LAND_EN_DIS', 'LandAttackEnable');
mapConfig.set('IPSEC_IKE_EN_DIS', 'IPSecIKEEnable');
mapConfig.set('IPSEC_IKE_THR', 'IPSecIKEThreshold');
mapConfig.set('HTTP_EN_DIS', 'HTTPFloodEnable');
mapConfig.set('HTTPS_EN_DIS', 'HTTPSFloodEnable');

//get key from value
function getKeyByValue(value) {
    for (let [key, val] of mapConfig.entries()) {
        if (val === value) return key;
    }
}

//add get key bu value function to mapConfig
mapConfig.getKeyByValue = getKeyByValue;

module.exports = {
    mapConfig,
}