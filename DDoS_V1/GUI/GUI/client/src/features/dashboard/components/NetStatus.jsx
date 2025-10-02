import { Card, Space, Tag, Flex } from "antd";
import React from "react";

import { useTranslation } from 'react-i18next';

import DangerImage from "@/assets/icons/danger.svg";
import ProcessingImage from "@/assets/icons/processing.svg";

import { byteFormatter, bitFormatter, cntFormatter } from "@/lib/formatter";

import '@/features/dashboard/styles/main.css';

const Processing = ({data}) => {
    const { t, i18n } = useTranslation();

    return (
        <Card bordered={true} className="status-card status-safe" >
            <Flex vertical style={{ width: '100%', height: '100%' }}>
                <div className="status-title" style={{height:'88px'}}>
                    <Space size="small">
                        <img src={ProcessingImage} alt="Attacked" />
                        <span>{t('traffic.status.peace').toUpperCase()}</span>
                    </Space>
                </div>
                <div className="status-info" >
                    <Space size="middle" style={{ width: '100%' }}>
                        <Tag color="#35D11F" style={{color: '#ffffff'}}>RECEIVED</Tag>
                        <span>{bitFormatter(data?.total.bypass.bits ?? 0) + "/s"}</span>
                        <span>|</span>
                        <span>{cntFormatter(data?.total.bypass.packets ?? 0) + "/s"}</span>
                    </Space>
                </div>
            </Flex>
        </Card>
    );
}

const UnderAttack = ({data}) => {
    const { t, i18n } = useTranslation();

    return (
        <Card 
            bordered={true} 
            className="status-card status-danger attack-blink"
            style={{
                animation: 'dangerBlink 1s infinite alternate'
                
            }}
        >
            <Flex vertical style={{ width: '100%', height: '100%' }}>
                <div className="status-title" style={{height:'89px'}}>
                    <Space size="small">
                        <img 
                            src={DangerImage} 
                            alt="Processing" 
                            style={{
                                animation: 'iconPulse 0.8s infinite alternate'
                            }}
                        />
                        <span 
                            style={{
                                animation: 'textFlash 0.6s infinite alternate'
                            }}
                        >
                            {t('traffic.status.attack').toUpperCase()}
                        </span>
                    </Space>
                </div>
                <div className="status-info">
                    <Space size="middle" style={{ width: '100%' }}>
                        <Tag 
                            color="#FF7070" 
                            style={{
                                color: '#ffffff',
                                animation: 'tagBlink 0.7s infinite alternate'
                            }}
                        >
                            DROPPING
                        </Tag>
                        <span 
                            style={{
                                animation: 'textFlash 0.6s infinite alternate'
                            }}
                        >
                            {bitFormatter(data?.total.attack.bits ?? 0) + "/s"}
                        </span>
                        <span 
                            style={{
                                animation: 'textFlash 0.6s infinite alternate'
                            }}
                        >
                            |
                        </span>
                        <span 
                            style={{
                                animation: 'textFlash 0.6s infinite alternate'
                            }}
                        >
                            {cntFormatter(data?.total.attack.packets ?? 0) + "/s"}
                        </span>
                    </Space>
                </div>
            </Flex>
            
            {/* CSS Styles */}
            <style jsx>{`
                @keyframes dangerBlink {
                    0% {
                        background-color: #ff4d4f;
                        border-color: #ff4d4f;
                        box-shadow: 0 0 10px rgba(255, 77, 79, 0.6);
                    }
                    100% {
                        background-color: #1a1a1a;
                        border-color: #1a1a1a;
                        box-shadow: 0 0 10px rgba(26, 26, 26, 0.6);
                    }
                }

                @keyframes iconPulse {
                    0% {
                        transform: scale(1);
                        filter: brightness(1);
                    }
                    100% {
                        transform: scale(1.1);
                        filter: brightness(1.5);
                    }
                }

                @keyframes textFlash {
                    0% {
                        color: #ffffff;
                        text-shadow: 0 0 5px rgba(255, 255, 255, 0.8);
                    }
                    100% {
                        color: #ff7070;
                        text-shadow: 0 0 10px rgba(255, 112, 112, 0.8);
                    }
                }

                @keyframes tagBlink {
                    0% {
                        background-color: #FF7070;
                        transform: scale(1);
                    }
                    100% {
                        background-color: #ff4d4f;
                        transform: scale(1.05);
                    }
                }

                .attack-blink {
                    position: relative;
                    overflow: hidden;
                }

                .attack-blink::before {
                    content: '';
                    position: absolute;
                    top: 0;
                    left: -100%;
                    width: 100%;
                    height: 100%;
                    background: linear-gradient(90deg, transparent, rgba(255, 77, 79, 0.3), transparent);
                    animation: sweep 2s infinite;
                }

                @keyframes sweep {
                    0% {
                        left: -100%;
                    }
                    100% {
                        left: 100%;
                    }
                }

                /* Override any existing styles */
                .status-card.status-danger.attack-blink {
                    transition: all 0.3s ease;
                }

                .status-card.status-danger.attack-blink:hover {
                    animation-play-state: paused;
                }
            `}</style>
        </Card>
    );
}

const   NetStatus = ({psdata, status}) => {
    return (
        status === "Attack" ? <UnderAttack data={psdata} /> : <Processing data={psdata}  /> 
    );
}

export default NetStatus;