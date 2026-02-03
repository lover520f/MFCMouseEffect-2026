#pragma once

#include <string>

struct TrailHistoryProfileModel {
    int durationMs = 300;
    int maxPoints = 32;
};

struct TrailProfilesModel {
    TrailHistoryProfileModel line{300, 32};
    TrailHistoryProfileModel streamer{420, 46};
    TrailHistoryProfileModel electric{280, 24};
    TrailHistoryProfileModel meteor{520, 60};
    TrailHistoryProfileModel tubes{350, 40};
};

struct TrailParamsModel {
    float streamerGlowWidthScale = 1.8f;
    float streamerCoreWidthScale = 0.55f;
    float streamerHeadPower = 1.6f;

    float electricAmplitudeScale = 1.0f;
    float electricForkChance = 0.10f;

    float meteorSparkRateScale = 1.0f;
    float meteorSparkSpeedScale = 1.0f;
};

struct TrailTuningModel {
    std::string style = "default"; // default | snappy | long | cinematic | custom
    TrailProfilesModel profiles;
    TrailParamsModel params;
};

