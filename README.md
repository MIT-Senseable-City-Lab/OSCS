# FLATBURN: the open source cit scanner

[Project website](https://senseable.mit.edu/flatburn).

### Flatburn Overview
<img src="https://github.com/MIT-Senseable-City-Lab/OSCS/blob/main/flatburn-images/cover.jpeg" width="400px"><img src="https://github.com/MIT-Senseable-City-Lab/OSCS/blob/main/flatburn-images/Flatburn-design.png" width="300px">

Part of the [City Scanner](https://senseable.mit.edu/cityscanner/)project at [MIT Senseable City Lab](https://senseable.mit.edu), *Flatburn* is an open-source, solar-powered, modular sensing platform that can be easily deployed or road vehciles to collect various environmental data. This repository containst all information to [Build](Build) Flatburn, [Explore](Explore) datasets collected during City Scanner deployments and [Learn](Learn) how to perform environemental analyses and use the data to contribute to the development healtier cities.

## Build
 - [/hardware](Build/Hardware): The fabrication files for Flatburn include files for [3D printing](Build/Hardware/Hardware%20enclosure/To%20Print), [laser cutting](Build/Hardware/Hardware%20enclosure/To%20lasercut), and a CAD model of the full [assembly](Build/Hardware/Hardware%20enclosure/Flatburn_assembly.step). A bill of materials listing all the required [mechanical components](https://docs.google.com/spreadsheets/d/1oa0ZC6CXszNmvcmob7ju2rJUDLLGSCP4pCBNqtu63Sk/edit?usp=sharing) is also included.
The [hardware schematics](Build/Hardware/Hardware%20schematics) folder contains `.brb` files for the main-board and the sensor-board which can be viewed using [Cadence](https://www.cadence.com/en_US/home/tools/pcb-design-and-analysis/allegro-downloads-start.html) free viewer, schematics of the PCB in `.pdf` format,`.csv` file with bill of materials for the electronics components.
 - [/firmware](Build/Firmware): The flatburn main firmware code base, developed using [Particle.io](https://www.particle.io/workbench/) workbench.
 - [/fabrication handbook](Build/Handbook): The assembly guide with detailed instructions on how to build flatburn.


## Explore
- [/datasets](Explore/Datasets): The folder contains `.csv` calibrated air quality data collected during City Scanner deployments worldwide. 
- [/data collection document](Explore/Calibration Manual): The folder contains a guide for validation and calibration of the data collected by Flatburn.


## Learn
- [/coding exercise](Coding%20Exercise): The folder contains [jupyter](https://jupyter.org/) notebooks for a coding activity written in Python to introduce non-experts to air quality and environmental sensing and analysis methods; including exploring time series analysis, geospatial analysis, and pollution hotspot analysis. The conding activity can be used with the [datasets](Explore/Datasets) captured by previous City Scanner deployment as well as data captured by Flatburn.
- [/facilitator handbook](https://github.com/MIT-Senseable-City-Lab/OSCS/tree/main/Learn/Facilitator%20Handbook): The folder contains a facilitator guide and educational slides to implement the coding exercise as part of workshops or classroom activities.

