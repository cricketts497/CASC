# CASC
CASC is a new model for a CRIS data aquisition system using C++ QT. It is designed to be simple, keeping all the control widgets in a single window surrounding a graph of incoming data. CASC uses a config file (config.txt) to locate device hosts, then can run those devices locally or remotely. The config file has the format:
```deviceName,hostAddress,hostDevicePort```
The listener port is given by a line:
```listener,listenPort```

Devices are controlled by push buttons in the left hand toolbar, while widgets such as the TOF histogram and scanners are opened using the upper toolbar.