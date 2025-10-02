import React from "react";
import { Carousel, Typography } from "antd";
import AttackCard from "./AttackCard"; // Adjust the import path as needed

const { Text } = Typography;

const AttackCarousel = ({ notifications }) => {
  const anomalies = notifications.filter(
    (item) => item.NotificationType === "anomalies"
  );

  if (!anomalies.length) {
    return <Text>No attack events to display</Text>;
  }
  console.log("atkCarousel", notifications);
  return (
    <div style={{ margin: "20px 0" }}>
      <Carousel arrows dots={false} draggable>
        {anomalies.map((attack) => (
          <div key={attack.NotificationId} style={{ display: "flex", justifyContent: "center" }}>
            <AttackCard attackData={attack} />
          </div>
        ))}
      </Carousel>
    </div>
  );
};

export default AttackCarousel;
