# Open Source City Scanner (Flatburn)

This is the official repository for FLATBURN: an Open Source City Scanner

### Flatburn Overview
<img src="https://github.com/MIT-Senseable-City-Lab/OSCS/blob/main/flatburn-images/cover.jpeg" width="400px"><img src="https://github.com/MIT-Senseable-City-Lab/OSCS/blob/main/flatburn-images/Flatburn-design.png" width="300px">

Part of the [City Scanner project](https://senseable.mit.edu/cityscanner/), Flatburn is an open-source mobile sensing platform that collects various environmental data which generate a new real-time map of sensed data, building on existing fleets.

## Build

 - [/hardware](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Build/Hardware): The fabrication files for Flatburn include files for [3D printing](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Build/Hardware/Hardware%20enclosure/To%20Print), [laser cutting](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Build/Hardware/Hardware%20enclosure/To%20lasercut), and a CAD model of the full [assembly](https://github.com/MIT-Senseable-City-Lab/OSCS/blob/main/Build/Hardware/Hardware%20enclosure/Flatburn_assembly.step). A bill of materials listing all the required [mechanical components](https://docs.google.com/spreadsheets/d/1oa0ZC6CXszNmvcmob7ju2rJUDLLGSCP4pCBNqtu63Sk/edit?usp=sharing) is also included.
The [hardware schematics](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Build/Hardware/Hardware%20schematics) folder contains `.brb` files for the main-board and the sensor-board which can be viewed using [Cadence](https://www.cadence.com/en_US/home/tools/pcb-design-and-analysis/allegro-downloads-start.html) free viewer, schematics of the PCB in `.pdf` format,`.csv` file with bill of materials for the electronics components.
 - [/firmware](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Build/Firmware): The flatburn main firmware code base, developed using [Particle.io](https://www.particle.io/workbench/) workbench.
 - [/fabrication handbook](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Build/Handbook): The assembly guide with detailed instructions on how to build flatburn.


## Explore

- [/datasets](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Explore/Datasets): The folder contains `.csv` Calibrated air quality data of the the cityscanner. 
- [/data collection document](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Explore/Data%20collection%20document): Open-source manual and code for data collection


## Learn
- [/coding exercise](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Learn/Coding%20Exercise): The coding activity written in Python to introduce users to air quality and environmental sensing methods by working with CityScanner Data, exploring time series analysis, geospatial analysis, and pollution hotspot analysis.
- [/facilitator handbook](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Learn/Facilitator%20Handbook): The guide and educational slide gives users background information on air pollutants, environmental sensing, and an overview of the CityScanner device. 

