#include <Arduino.h>

struct HistogramData
{

    uint16_t binCounts[24];

    uint8_t bin1TimeToCross;
    uint8_t bin3TimeToCross;
    uint8_t bin5TimeToCross;
    uint8_t bin7TimeToCross;

    uint16_t samplingPeriod;
    uint16_t sampleFlowRate;
    uint16_t temperature;
    uint16_t humidity;

    float pm1;
    float pm2_5;
    float pm10;

    uint16_t rejectCountGlitch;
    uint16_t rejectCountLongTOF;
    uint16_t rejectCountRatio;
    uint16_t rejectCountOutOfRange;
    uint16_t fanRevCount;
    uint16_t laserStatus;
    uint16_t checkSum;
    bool valid;

    float getTempC();
    float getTempF();
    float getHumidity();

    String toString();
};