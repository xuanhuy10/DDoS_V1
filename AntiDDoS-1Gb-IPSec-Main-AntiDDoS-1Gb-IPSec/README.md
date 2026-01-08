# GUI_DDoS-1GB 
###### The system is a DDoS Defender device that supports protection based on IPv4 address or physical port interface, allows flexible configuration of independent defense mechanisms, provides real-time monitoring via USB Serial port, and ensures valid connections for essential server services.

## — RUN THE APPLICATION ( LOCAL DEVELOPMENT ENVIRONMENT )
#### Try 'npm run dev' in server terminal then 'npm run dev' in client terminal
###### Or try this cmd commands :
```

wt -w 0 new-tab cmd /k "cd /d D:\FPT_projects\server && npm run dev" --title "SERVER" ; new-tab cmd /k "cd /d D:\FPT_projects\client && npm run dev" --title "CLIENT"

```
#### If the application does not run successfully, check .env files in both server and client folders

## — DOWNLOAD NECESSARY RESOURCES
```

cd server
npm install
npm audit fix --force

cd ..
cd client
npm install
npm audit fix --force
npm run build

```
