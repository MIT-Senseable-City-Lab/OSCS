#include "HistogramData.h"

float HistogramData::getTempC()
{
    return -45 + 175 * (temperature / (pow(2, 16) - 1));
}
float HistogramData::getTempF()
{
    return -49 + 347 * (temperature / (pow(2, 16) - 1));
}
float HistogramData::getHumidity()
{
    return 100 * (humidity / (pow(2, 16) - 1));
}

String HistogramData::toString() {

    String info = "-----Histogram Data-----\n";
    info += "Validity: ";
    info += valid;
    info += "\n";
    info += "Bin Counts\n";
    for (int i = 0; i < 24; i++) {
        info += binCounts[i];
        info += " ";
    }
    info += "\n";
    info += "-------------------------------------------------\n";
    info += "Time To Cross\n";
    info += bin1TimeToCross;
    info += " ";
    info += bin3TimeToCross;
    info += " ";
    info += bin5TimeToCross;
    info += " ";
    info += bin7TimeToCross;
    info += "\n";

    info += "-------------------------------------------------\n";
    info += "Sampling Period\n";
    info += samplingPeriod;
    info += "\n";
    info += "-------------------------------------------------\n";
    info += "Sample Flow Rate\n";
    info += sampleFlowRate;
    info += "\n";
    info += "-------------------------------------------------\n";
    info += "Temperature\n";
    info += temperature;
    info += "\n";
    info += "-------------------------------------------------\n";
    info += "Humidity\n";
    info += humidity;
    info += "\n";
    info += "-------------------------------------------------\n";
    info += "pm1\n";
    info += (String)pm1;
    info += "\n";
    info += "-------------------------------------------------\n";
    info += "pm2.5\n";
    info += (String)pm2_5;
    info += "\n";
    info += "-------------------------------------------------\n";
    info += "pm10\n";
    info += (String)pm10;
    info += "\n";
    info += "-------------------------------------------------\n";

    info += "MSLNS\n";

    info += rejectCountGlitch;
    info += " ";
    info += rejectCountLongTOF;
    info += " ";
    info += rejectCountRatio;
    info += " ";
    info += rejectCountOutOfRange;
    info += " ";
    info += fanRevCount;
    info += " ";
    info += laserStatus;
    info += " ";
    info += checkSum;
    info += "\n";

    return info;
}