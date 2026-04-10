#pragma once

#include "UiConfigProvider.h"

class CustomLabel;
class Simulator;
class Settings;

namespace SimulatorGuiUiHelpers {
void resetScramIndicators(const UiStyleConfig &style, CustomLabel *userScram,
                          CustomLabel *powerScram, CustomLabel *periodScram,
                          CustomLabel *waterTemperatureScram,
                          CustomLabel *waterLevelScram,
                          CustomLabel *fuelTemperatureScram);

void updateScramWarningIndicators(const UiStyleConfig &style,
                                  Simulator *reactor, Settings *properties,
                                  float currentTemperature,
                                  CustomLabel *periodScram,
                                  CustomLabel *fuelTemperatureScram,
                                  CustomLabel *waterTemperatureScram,
                                  CustomLabel *powerScram);
}  // namespace SimulatorGuiUiHelpers
