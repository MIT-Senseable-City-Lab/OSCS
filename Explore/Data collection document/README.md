### Open-Source City Scanner Manual

### **Background**

Mobile environment monitoring is a fast-developing method that is capable of providing high spatial resolution information on air quality, noise, heat, and meteorology. It has been attracting prominent attention from both the scientific community and the public for a wide range of applications, including tracing emission sources, evaluating ambient air pollution distribution, and estimating personal exposures. 


The [City Scanner project](https://senseable.mit.edu/flatburn/) initiated by the [Senseable City Lab](https://senseable.mit.edu/) at MIT is a pioneer in mobile environment monitoring. It aims at developing a sensing platform to enable large scale environmental sensing tasks using existing urban fleets as sensing nodes.
We envision the platform to be adopted as a novel type of city infrastructure to enable big data collection and evidence-based environmental/climate policymaking. 
In general, City Scanner adopts a low-cost, modular design with [internet-of-things (IoT)](https://en.wikipedia.org/wiki/Internet_of_things) capabilities. 


Environmental sensing can be expensive and high maintenance in the traditional approach. United States Environmental Protection Agency (US EPA) defines that any air sensor either for a single or multiple pollutants that costs less than $2500 is considered low-cost. 

Sensors used on the [City Scanner](https://ieeexplore.ieee.org/document/8361419) are generally less than $500 and are tested with good accuracy and precision. To lower unit cost even more, City Scanner implements a modular design for its on-board sensors. Users can easily keep redundant sensing modules to a minimum and adapt the platform in different urban environment deployments. 

City Scanner is also IoT-enabled, where individual sensing units are mounted on top of urban fleets for data collection and stream data to cloud for storage, manipulation, and analysis via a cellular network. Jointly, these designs have made City Scanner a leading mobile sensing platform to empower environmental scientific research, support evidence-based environmental and climate decision-making, and encourage citizen engagement and awareness in environmental justice topics.

### **Sensing modules on a City Scanner**
<img src="https://github.com/MIT-Senseable-City-Lab/OSCS/blob/main/flatburn-images/flatburn%20configuration.jpeg" width="800px">

We illustrate City Scanner version **Flatburn** 

Each City Scanner has two major modules, the core module and the sensor module. The core module houses the motherboard, the data communication and local storage system, and the battery and thermal performance management system. The sensor module, GPS and solar panel are connected to and managed by the core module. 

Details of the core module design are presented in the hardware and firmware [open-source manual](https://github.com/MIT-Senseable-City-Lab/OSCS/blob/main/Build/Handbook/Flatburn_%20assembly%20guide.pdf).
