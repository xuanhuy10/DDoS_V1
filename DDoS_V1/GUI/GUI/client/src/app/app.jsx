import Router from "@/app/router";

import { LoadingOutlined } from "@ant-design/icons";
import { ConfigProvider, Spin } from "antd";
const spinIcon = <LoadingOutlined style={{fontSize:'24px'}} spin />
Spin.setDefaultIndicator(spinIcon);
const theme = {
  token: {
    // Protocol colors
    tcpColor: '#EE6666',
    udpColor: '#FC8452',
    httpColor: '#73C0DE',
    icmpColor: '#5470C6',
    dnsColor: '#91CC75',
    espColor: '#3BA272',
    unknownColor: '#BFBFBF',

    // Attack colors
    synFloodColor: '#EE6666',
    udpFloodColor: '#FC8452',
    icmpFloodColor: '#5470C6',
    httpFloodColor: '#73C0DE',
    httpsFloodColor: '#15008B',
    dnsFloodColor: '#91CC75',

    ipsecColor: '#FFCD29',
    tcpFragColor: '#9D1F1F',
    udpFragColor: '#F44C06',
    landColor: '#2FFF9E',

    normal: '#AACC07',
  },
  components: {
    Layout: {
      siderBg: '#090A21',
      triggerBg: '#090A21',
    },
    Menu: {
      darkItemBg: '#090A21',
      darkPopupBg: '#090A21',
      darkSubMenuItemBg: '#090A21', 
      // subMenuItemSelectedColor: '#FFFFFF'
    },
    Collapse: {
      headerBg: '#ffffff',
    }
  },  
};

export const App = () => {
  
  
  return (
    <ConfigProvider theme={theme}>
        <Router />
    </ConfigProvider>
  );
};