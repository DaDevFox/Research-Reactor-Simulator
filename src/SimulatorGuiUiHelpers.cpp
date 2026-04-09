#include "../include/SimulatorGuiUiHelpers.h"

#include "../include/CustomLabel.h"
#include "../include/Settings.h"
#include "../include/Simulator.h"

namespace SimulatorGuiUiHelpers {
void resetScramIndicators(const UiStyleConfig &style, CustomLabel *userScram,
                          CustomLabel *powerScram, CustomLabel *periodScram,
                          CustomLabel *waterTemperatureScram,
                          CustomLabel *waterLevelScram,
                          CustomLabel *fuelTemperatureScram) {
  userScram->setGlow(false);
  userScram->setBackgroundColor(style.scramNormalColor);
  powerScram->setGlow(false);
  powerScram->setBackgroundColor(style.scramNormalColor);
  periodScram->setGlow(false);
  periodScram->setBackgroundColor(style.scramNormalColor);
  waterTemperatureScram->setGlow(false);
  waterTemperatureScram->setBackgroundColor(style.scramNormalColor);
  waterLevelScram->setGlow(false);
  waterLevelScram->setBackgroundColor(style.scramNormalColor);
  fuelTemperatureScram->setGlow(false);
  fuelTemperatureScram->setBackgroundColor(style.scramNormalColor);
}

void updateScramWarningIndicators(const UiStyleConfig &style,
                                  Simulator *reactor, Settings *properties,
                                  float currentTemperature,
                                  CustomLabel *periodScram,
                                  CustomLabel *fuelTemperatureScram,
                                  CustomLabel *waterTemperatureScram,
                                  CustomLabel *powerScram) {
  if ((*reactor->getReactorPeriod() < 1.1 * properties->periodLimit) &&
      (*reactor->getReactorPeriod() > 0.))
    periodScram->setBackgroundColor(style.scramWarningColor);
  else
    periodScram->setBackgroundColor(style.scramNormalColor);

  if (currentTemperature > 0.9 * properties->tempLimit)
    fuelTemperatureScram->setBackgroundColor(style.scramWarningColor);
  else
    fuelTemperatureScram->setBackgroundColor(style.scramNormalColor);

  if (*reactor->getWaterTemperature() > 0.9 * properties->waterTempLimit)
    waterTemperatureScram->setBackgroundColor(style.scramWarningColor);
  else
    waterTemperatureScram->setBackgroundColor(style.scramNormalColor);

  if (reactor->getCurrentPower() > 0.9 * properties->powerLimit)
    powerScram->setBackgroundColor(style.scramWarningColor);
  else
    powerScram->setBackgroundColor(style.scramNormalColor);
}
}  // namespace SimulatorGuiUiHelpers
