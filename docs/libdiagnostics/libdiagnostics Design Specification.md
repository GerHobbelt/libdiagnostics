# `libdiagnostics` Design Specification

- assertions
  - which must be also suitable for use inside testing frameworks: tests should benefit from internal assertions firing on failure to comply)
- logging
  - possibly using a generic, external, logging framework
  - both **human-readable text** log output, **binary output** (plus tooling to process/query this data) and recording-to-**database**[^db] are supported
  - multi-target, configurable
- telemetry / data gathering

[^db]: the latter (storing the log records in a database) is geared towards providing a telemetry solution for application analysis & debugging. SQLite is our preferred db of choice.




## See also

- [[Design Notes]]
- [[Further Design Notes]]
