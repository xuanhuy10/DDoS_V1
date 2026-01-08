# SERVER API SPECIFICATIONS


<details>
<summary>1. ABOUT SERVER </summary>

### SERVER ARCHITECTURE

    - database   : sqlite3
    - src        :
        - config      : System configurations including database path, port, running environment such as development or production
        - controllers : Delare functions that connect directly to the APIs for calling (the connections will be declare in routes)
        - helpers     : Gwt hardware information (be used in the controllers)
        - middlewares : Contains functions that process requests before they reach the controllers, handling tasks like authentication, logging, error handling, or request validation
        - models      : Defines the data structures and schemas for interacting with the database, typically representing collections and encapsulating database operations.
        - routes      : Defines the API endpoints and their corresponding HTTP methods
        - services    : Contains business logic and external API integrations (be used in the controllers)
        app.js        : The main entry point of the application, where the Express server is initialized, middleware is configured, routes are registered, and the application is set up to listen for requests.
    .env              : A file for storing environment variables, such as database credentials, API keys, or port numbers, which are loaded into the application to manage configuration securely.
    .package.json     : The project manifest file that lists dependencies, scripts, and metadata for the Node.js application, used by npm to manage the project and its dependencies.

</details>


<details>
<summary>2. DATA THREAD LINES </summary>

##### 🔴 ***CAUTION : THE SYSTEM CURRENTLY DOES NOT HAVE ORM SYSTEMS FOR DATABASE INTERACTION***

### DATA PATHS FROM DATABASE TO APIs

     app.js (api prefix) -> | Auth.route.js             (api path) -> Auth.controller.js             (controller functions) -> Users.model.js            (data query functions)  
                            | DefenseProfiles.route.js  (api path) -> DefenseProfiles.controller.js  (controller functions) -> DefenseProfiles.model.js  (data query functions)
                            | DeviceInterfaces.route.js (api path) -> DeviceInterfaces.controller.js (controller functions) -> DeviceInterfaces.model.js (data query functions)
                            | DeviceLogs.route.js       (api path) -> DeviceLogs.controller.js       (controller functions) -> DeviceLogs.model.js       (data query functions)
                            | DeviceSettings.route.js   (api path) -> DeviceSettings.controller.js   (controller functions) -> DeviceSettings.model.js   (data query functions)
                            | NetworkAddress.route.js   (api path) -> NetworkAddress.controller.js   (controller functions) -> NetworkAddress.model.js   (data query functions)
                            | NetworkAnomalies.route.js (api path) -> NetworkAnomalies.controller.js (controller functions) -> NetworkAnomalies.model.js (data query functions)
                            | Notification.route.js     (api path) -> Notification.controller.js     (controller functions) -> Notification.model.js     (data query functions)
                            | Users.route.js            (api path) -> Users.controller.js            (controller functions) -> Users.model.js            (data query functions)




</details>

<details>
<summary>3. SERVER API LIST </summary>

##  [ AUTH ](../../server/src/routes/Auth.route.js)
        [POST]    http://localhost:port/v1/auth/login
        [POST]    http://localhost:port/v1/auth/refresh (unused)


##  [ DEFENSE PROFILES  ](../../server/src/routes/DefenseProfiles.route.js)
        [GET]     http://localhost:port/v1/defense/profiles/all/:offset
        [GET]     http://localhost:port/v1/defense/profiles/all/
        [GET]     http://localhost:port/v1/defense/profiles/active
        [GET]     http://localhost:port/v1/defense/profiles/active/config/:attackType (unused)
        [GET]     http://localhost:port/v1/defense/profiles/user/:userId
        [GET]     http://localhost:port/v1/defense/profiles/active/attacks/rate
        [POST]    http://localhost:port/v1/defense/profiles/
        [PATCH]   http://localhost:port/v1/defense/profiles/:profileId (on process)
        [PUT]     http://localhost:port/v1/defense/profiles/:profileId (on process)
        [POST]    http://localhost:port/v1/defense/profiles/:profileId/apply
        [DELETE]  http://localhost:port/v1/defense/profiles/:profileId


##  [ DEVICE INTERFACES  ](../../server/src/routes/DeviceInterfaces.route.js)
        [GET]     http://localhost:port/v1/defense/interfaces
        [GET]     http://localhost:port/v1/defense/interfaces/mirroring
        [PATCH]   http://localhost:port/v1/defense/interfaces/:deviceInterfaceId
        [PATCH]   http://localhost:port/v1/defense/interfaces//mirroring/:MirrorInterfaceId
        [POST]    http://localhost:port/v1/defense/interfaces/mirroring
        [DELETE]  http://localhost:port/v1/defense/interfaces/mirroring/:MirrorInterfaceId

##  [ DEVICE LOGS  ](../../server/src/routes/DeviceLogs.route.js)
        [GET]     http://localhost:port/v1/logs`
        [GET]     http://localhost:port/v1/logs/system/export/:logType/time-range
        [GET]     http://localhost:port/v1/logs/system/:logType
        [GET]     http://localhost:port/v1/logs/system/export/:logType/:ids
        [DELETE]  http://localhost:port/v1/logs/system/:logType/time-range
        [DELETE]  http://localhost:port/v1/logs//system/export/:logType/:ids

##  [ DEVICE SETTINGS  ](../../server/src/routes/DeviceSettings.route.js)
        [GET]     http://localhost:port/v1/device/resource/usage
        [GET]     http://localhost:port/v1/device/disk/usage
        [GET]     http://localhost:port/v1/device/disk/setting
        [POST]    http://localhost:port/v1/device/disk/setting
        [POST]    http://localhost:port/v1/device/reset


##  [ NETWORK ADDRESSES  ](../../server/src/routes/NetworkAddresses.route.js)
        [GET]     http://localhost:port/v1/defense/address/common/white
        [GET]     http://localhost:port/v1/defense/address/common/black
        [GET]     http://localhost:port/v1/defense/address/vpm/white
        [GET]     http://localhost:port/v1/defense/address/http/black
        [POST]    http://localhost:port/v1/defense/address/common/white
        [POST]    http://localhost:port/v1/defense/address/common/black
        [POST]    http://localhost:port/v1/defense/address/vpm/white
        [POST]    http://localhost:port/v1/defense/address/http/black
        [POST]    http://localhost:port/v1/defense/address/common/white/import
        [POST]    http://localhost:port/v1/defense/address/bulk-delete
        [POST]    http://localhost:port/v1/defense/address/bulk-delete-by-file
        [POST]    http://localhost:port/v1/defense/address/http/black/import
        [POST]    http://localhost:port/v1/defense/address/http/black/import-delete
        [POST]    http://localhost:port/v1/defense/address/vpn/white/import
        [POST]    http://localhost:port/v1/defense/address/vpn/white/import-delete
        [POST]    http://localhost:port/v1/defense/address/blocked/import
        [POST]    http://localhost:port/v1/defense/address/blocked/import-delete
        [POST]    http://localhost:port/v1/defense/address/common/white/import-delete

##  [ NETWORK ANOMALIES  ](../../server/src/routes/NetworkAnomalies.route.js)
        [GET]     http://localhost:port/v1/network/anomalies
        [GET]     http://localhost:port/v1/network/analysis


##  [ NOTIFICATION  ](../../server/src/routes/Notification.route.js)



##  [ USERS  ](../../server/src/routes/Users.route.js)
        [GET]     http://localhost:port/v1/users
        [GET]     http://localhost:port/v1/users/:userId
        [GET]     http://localhost:port/v1/users/session/me
        [POST]    http://localhost:port/v1/users (failed to return user data)
        [PATCH]   http://localhost:port/v1/users/:userId
        [DELETE]  http://localhost:port/v1/users/:userId


</details>