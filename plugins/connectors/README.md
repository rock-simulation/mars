# Connectors Plugin

## What is this?
This plugin mates male and female connectors that are close to each other. Parameters for proximity thresholds are set for distance and angle in a connectors YAML of a model's smurf export.

## Sample configuration YAML:

```yaml
    connectors:
        autoconnect: true
        types:
        - name: transterra
          distance: 0.15
          angle: 3.1415
          maxforce: 100

        connectors:
        - gender: male
          link: male_connector
          mating: automatic
          name: payload_male_connector
          type: transterra
        - gender: female
          link: female_connector
          mating: automatic
          name: payload_female_connector
          type: transterra
```

## GUI

### Control
Connector connections can be manually enabled or disabled from the plugin's interface:
 - Control > Connect available connectors.
 - Control > Disconnect all connectors.


### Properties
Global autoconnect can be set from the Plugin properties interface.

## Mating requirements

There are 2 cases that will trigger checking for connections:
  1. When it is forced from the Control GUI: Control > Connect available connectors
  2. During plugin update(): **If** autoconnect is globally set to true **or if** male **and** female connectors' mating properties are both set to `automatic`.

## Truth table

  | case | autoconnect | male mating | female mating | connected |
  |------|-------------|-------------|---------------|-----------|
  | 1 | F | automatic | - | F |
  | 2 | F | - | automatic | F |
  | 3 | F | - | - | F |
  | 4 | F | automatic | automatic | T |
  | 5 | T | automatic | - | T |
  | 6 | T | - | automatic | T |
  | 7 | T | - | - | T |
  | 8 | T | automatic | automatic | T |

## Known issue
Female mating is always set as 'automatic' no matter the actual value set in the YAML. This results in a failure of truth table case 1.
This can be replicated with the sample YAML featured in this README.
