
import React from "react";
import { Card } from "antd";
import "@/features/dashboard/styles/marquee.css";

const InterfaceMarquee = () => {
    return (
        <div className="wrapper">
            <div className="marquee-text fadeout-horizontal">
                <div className="marquee-text-track">
                <p aria-hidden="true">Acronics Solutions</p>
                <p aria-hidden="true">Sysnet Def v1.0 </p> 
                </div>
            </div>
        </div>
    );
};

export default InterfaceMarquee;