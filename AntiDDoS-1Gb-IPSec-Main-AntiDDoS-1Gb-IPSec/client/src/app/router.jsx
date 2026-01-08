import ReactDOM from "react-dom/client";
import { BrowserRouter, Routes, Route, Navigate } from "react-router-dom";
import { useState } from "react";

import ProtectedRoute from "@/components/common/ProtectedRoute";
//#region layout
import Layout from "@/app/pages/DDoSLayout";
import Login from "@/app/pages/login/Login";
//#endregion

//#region home
import Dashboard from "@/app/pages/home/Dashboard";
//#endregion

//#region authenticated
// import ProtectedRoute from "@/features/isAuthenticator/ProtectedRoute";
//#endregion

//#region defense tabs
import General from "@/app/pages/configuration/interface/General";
import DefenseProfile from "@/app/pages/configuration/interface/DefenseProfile";
import MirrorPort from "@/app/pages/configuration/interface/MirrorPort";
import NetworkProtection from "@/app/pages/configuration/interface/NetworkProtection";
import VpnWhitelist from "@/app/pages/configuration/interface/VpnWhitelist";
import HttpBlacklist from "@/app/pages/configuration/interface/HttpBlacklist";
import ProfileConfig from "@/app/pages/configuration/Profile";
import AttackList from "@/app/pages/configuration/interface/AttackList";
import ConfigSettingPage from "@/app/pages/configuration/interface/ConfigSettingPage";
//#endregion

//#region ipsec tabs
import IpSecProfileList from "./pages/configuration/interface/ipSecProfile";
import IpSecSetting from "./pages/configuration/ipSecSetting";
//#endregion

//#region analyze tabs
import Analyze from "./pages/analyze/Analyze";
//#endregion

//#region information tabs
import Information from "./pages/information/Information";
//#endregion

//#region monitor tabs
import Monitor from "./pages/monitor/Monitor";
import AttackDetails from "./pages/monitor/AttackDetails";
//#endregion

//#region manager tabs
import DeviceManager from "./pages/manager/DeviceManager";
import UsersManager from "./pages/manager/UsersManager";
import LogsManager from "./pages/manager/LogsManager";
//#endregion

//#region user tabs
import Profile from "./pages/user/User";
//#endregion

//#region notification tabs
import Notification from "./pages/notification/Notification";
import Unauthorized from "@/components/error/UnAuthorized";
import NotFound from "@/components/error/NotFound";
//#endregion

export default function Router() {
  return (
    <Routes>
      {/* Public Route */}
      <Route path="/auth/login" element={<Login />} />

      {/* Protected Routes */}
      <Route
        path="/"
        element={
          <ProtectedRoute>
            <Layout />
          </ProtectedRoute>
        }
      >
        <Route index element={<Dashboard />} />
        <Route path="dashboard" element={<Dashboard />} />
        <Route path="defense" element={<General />} />
        <Route path="defense/interface" element={<General />} />
        <Route
          path="defense/interface/config-setting-profile"
          element={<ConfigSettingPage />}
        />
        <Route
          path="defense/interface/defense-profile-list"
          element={<DefenseProfile />}
        />
        <Route
          path="defense/interface/port-mirroring"
          element={<MirrorPort />}
        />
        <Route
          path="defense/interface/network-protect"
          element={<NetworkProtection />}
        />
        <Route
          path="defense/interface/vpn-allowed"
          element={<VpnWhitelist />}
        />
        <Route
          path="defense/interface/http-attacker"
          element={<HttpBlacklist />}
        />
        <Route
          path="defense/interface/ipsec-profile"
          element={<IpSecProfileList />}
        />
        <Route path="defense/ipsec-settings" element={<IpSecSetting />} />
        <Route path="defense/interface/attack-list" element={<AttackList />} />
        <Route path="defense/profile" element={<ProfileConfig />} />
        <Route
          path="manager"
          element={
            <ProtectedRoute roleRequired={"admin"}>
              <UsersManager />
            </ProtectedRoute>
          }
        />
        <Route
          path="manager/users"
          element={
            <ProtectedRoute roleRequired={"admin"}>
              <UsersManager />
            </ProtectedRoute>
          }
        />
        <Route
          path="manager/device"
          element={
            <ProtectedRoute roleRequired={"admin"}>
              <DeviceManager />
            </ProtectedRoute>
          }
        />
        <Route
          path="manager/logs"
          element={
            <ProtectedRoute roleRequired={"admin"}>
              <LogsManager />
            </ProtectedRoute>
          }
        />
        <Route
          path="defense/interface/IpSecProfile"
          element={<IpSecProfileList />}
        />
        <Route path="information" element={<Information />} />
        <Route path="analyze" element={<Analyze />} />

        <Route path="monitor" element={<Monitor />} />
        <Route path="monitor/:attackType" element={<AttackDetails />} />

        <Route path="user" element={<Profile />} />
        <Route path="user/profile" element={<Profile />} />

        <Route path="notification" element={<Notification />} />
        {/* Redirect any unmatched path to the dashboard */}

        {/* <Route path="404" element={<NotFound />} /> */}
        <Route path="401" element={<Unauthorized />} />
        {/* <Route path="test" element={<Test /> } /> */}
      </Route>
      <Route path="*" element={<NotFound />} />
    </Routes>
  );
}
