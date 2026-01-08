import React from "react";
import { Card, Typography } from "antd";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faShieldAlt } from "@fortawesome/free-solid-svg-icons";

const { Text } = Typography;

const AttackCard = ({ filteredNotifications }) => {
  const { Attack, StartTime, State, Target, Stats, EndTime } = attackData;
  const stateLabel = State === 1 ? "Alert" : State === 0 ? "Finished" : "Unknown";
    console.log("attackdata" ,filteredNotifications)
  return (
    <Card style={{ width: 300, margin: "0 10px" }}>
      <div style={{ display: "flex", alignItems: "center", marginBottom: 8 }}>
        <FontAwesomeIcon icon={faShieldAlt} style={{ marginRight: 8 }} />
        <Text strong>{Attack || "Unknown Attack"}</Text>
      </div>
      <p>
        <strong>Start:</strong> {StartTime}
      </p>
      <p>
        <strong>State:</strong> {stateLabel}
      </p>
      <p>
        <strong>IP:</strong> {Target}
      </p>
      <p>
        <strong>Rate:</strong> {Stats}
      </p>
      <p>
        <strong>End:</strong> {EndTime}
      </p>
    </Card>
  );
};

export default AttackCard;
