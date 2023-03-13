#include "ConfigurationVariables.h"

String ConfigurationVariables::toString() {
    String info = "-----Configuration Variables-----\n";
    info += "Validity: ";
    info += valid;
    info += "\n";
    info += "Bin Boundries ADC";
    info += "\n";
    for (int i = 0; i < 25; i++) {
        info += binBoundriesADC[i];
        info += " ";
    }
    info += "\n";
    info += "-------------------------------------------------";
    info += "\n";
    info += "Bin Boundries Diametors";
    info += "\n";
    for (int i = 0; i < 25; i++) {
        info += binBoundriesDiametor[i];
        info += " ";
    }
    info += "\n";
    info += "-------------------------------------------------";
    info += "\n";
    info += "Bin Weights";
    info += "\n";
    for (int i = 0; i < 24; i++) {
        info += binWeightings[i];
        info += " ";
    }
    info += "\n";
    info += "-------------------------------------------------";
    info += "\n";
    info += "PM Diametors";
    info += "\n";
    info += pmDiametorA;
    info += " ";
    info += pmDiametorB;
    info += " ";
    info += pmDiametorC;
    info += "\n";
    info += "-------------------------------------------------";
    info += "\n";
    info += "PM MSLNS";
    info += "\n";
    info += maximumTimeOfFlight;
    info += " ";
    info += AMSamplingIntervalCount;
    info += " ";
    info += AMIdleIntervalCount;
    info += " ";
    info += AMMaxDataArraysInFile;
    info += " ";
    info += AMOnlySavePMData;
    info += " ";
    info += AMFanOnInIdle;
    info += " ";

    return info;
}