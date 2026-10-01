# Node-RED
Node-RED "flow(s)" parse MQTT messages and diplay measurered data in time serias graphics display
- *Solar intensity* of 100 W solar panel \[%\]
- *Cumulative Charge* time serie with scale \[x100 Wh\]
- *Cumulative Charge* gauge of 100 Ah Battery's charge \[%\]

## Docker
Use Node-RED docker from "hub.docker.com"
- https://hub.docker.com/r/nodered/node-red/
- https://github.com/node-red/node-red-docker
- https://nodered.org/
- https://nodered.org/docs/getting-started/
- https://nodered.org/docs/getting-started/docker
Bind mount docker container instance's */data* directory so any changes made to flows are persisted.

Read WEB pages and see you tube video(s) which are text(s) in "flows"

## Requider Additional Palettes
From right upper corner "sandswitch" menu
- Settings / Palette / Install (tab)

Install following palettes
-  @flowfuse/node-red-dashboard - (newer "dashboars2")
- node-red-dashboard (older deprecated Angular based dashboard - maintenance will end soon)
- node-red-contrib-uibuilder
- node-red-contrib-multiproject (optional)

## Some Guides
- https://dashboard.flowfuse.com/nodes/widgets/ui-chart#line-chart
- https://nodered.org/docs/user-guide/writing-functions
