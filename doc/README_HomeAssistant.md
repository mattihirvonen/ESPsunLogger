# Some Home Assistant Apps
Home Assistant's own (standard) default history is useful to check sensors's time history. But there are available more sophisticated graphics card to visialize measurement history data. 

## Some Official Apps
Select from left sidebar: *Settings / Devices & services*
- File editor (installed - easy and and in "daily use")
- HACS (installed - HA Community Services "installatiom tool")
- System Monitor (installed - disk free etc... - not yet configured)
  - https://www.home-assistant.io/integrations/systemmonitor/
- ESPhome (installed - ESP CPU application builder)
- Mosquitto Broker (installed, but currently use more generic container/VM in QNAP)
- Let's Encrypt

Following apps are "the must":
- File editor, HACS and System monitor
Optionally is suggested to install also:
- ESPhome

## Graphics Cards (HACS)
History graph card
- https://www.home-assistant.io/dashboards/history-graph/a-better-history-card
- https://www.npmjs.com/package/@kipk/ha-better-history
- http://192.168.1.103:8123/hacs/repository/1235796616
- https://community.home-assistant.io/t/a-better-history-card-ha-history-under-steroids/1010403

Statistics-Graph-Chart-Card (installed)
- https://community.home-assistant.io/t/statistics-graph-chart-card/996225

Advanced History provides a dedicated Home Assistant sidebar panel that keeps the familiar History-page workflow while using Statistics Graph Chart Card for graph rendering. It can also optionally replace the native History graph inside entity More Info dialogs  (installed)
- https://github.com/andyblac/Advanced-History-Integration

Apex Charts
- https://smarthomescene.com/guides/
- https://smarthomescene.com/guides/apexcharts-card-advanced-graphs-for-your-home-assistant-ui/

## Editors (HACS)
Blueprint Studio for Home Assistant
- https://github.com/ha-china/blueprint-studio

Studio Code Server (installed)
- https://github.com/hassio-addons/app-vscode
- https://community.home-assistant.io/t/statistics-graph-chart-card/996225

## Other Interesting APPS (HACS)
- FTP (server, installed)
- InfluxDB v1 (0.6 GB) (installed but stoped, use CPU/memory resources)
- Grafana  (1.9 GB) (installed but stoped, use CPU/memory resources)
- MQTT Explorer (use more generic Windows PC version)
- Node-RED (currently use Docker container in QNAP's Container Station to visualize data - currently no plan to make automation integrations in HA)

## Cleanup Tool(s) (HACS)
Home Assistant Cleanup Tool (to Remove Devices & Entities)
- https://github.com/jamespo/hasscleanup
