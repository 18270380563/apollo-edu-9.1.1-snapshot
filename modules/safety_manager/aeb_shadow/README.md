# AEB Shadow Mode

This module evaluates front collision risk from `/apollo/perception/obstacles`,
`/apollo/canbus/chassis`, and `/apollo/localization/pose`. Map-frame obstacle
coordinates are transformed into the ego vehicle frame before TTC evaluation.
It publishes JSON diagnostic payloads in
`google.protobuf.StringValue` on `/apollo/safety/aeb_shadow_status`.

The module is intentionally shadow-only: it does not publish control commands,
call CAN APIs, or modify vehicle actuation.

It logs the nearest forward target at the configured telemetry interval, including
when vehicle speed is below the AEB activation threshold.

Run it manually:

```bash
/apollo_workspace/modules/safety_manager/aeb_shadow/start_aeb_shadow.sh \
  --config /apollo_workspace/profiles/HQEV/modules/safety_manager/aeb_shadow/conf/aeb_shadow.conf.json
```

Inspect status:

```bash
cyber_channel echo /apollo/safety/aeb_shadow_status
```
