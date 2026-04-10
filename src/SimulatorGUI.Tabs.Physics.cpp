#include "../include/SimulatorGUI.h"

void SimulatorGUI::createPhysicsSettingsTab() {
  Widget *physics_settings_base =
      tabControl->createTab("Physics settings", false);
  CustomTabWidget *modeTabs = physics_settings_base->add<CustomTabWidget>();
  modeTabs->header()->setStretch(true);
  modeTabs->header()->setButtonAlignment(NVGalign::NVG_ALIGN_MIDDLE |
                                         NVGalign::NVG_ALIGN_CENTER);
  RelativeGridLayout *grid = new RelativeGridLayout();
  grid->appendCol(1.f);
  grid->appendRow(1.f);
  physics_settings_base->setLayout(grid);
  grid->setAnchor(modeTabs, RelativeGridLayout::makeAnchor(0, 0));

  // Create titles, periods and amplitudes
  std::string titles[2] = {"Physics", "Neutron source"};
  // RelativeGridLayout* layouts[2];
  // Widget* tabs[2];

  Widget *physics_settings = modeTabs->createTab("Physics");
  physics_settings->setId("Physics tab");
  RelativeGridLayout *physicsLayout = new RelativeGridLayout();
  physicsLayout->appendRow(
      RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
  physicsLayout->appendRow(
      RelativeGridLayout::Size(4 * 46.f, RelativeGridLayout::SizeType::Fixed));
  physicsLayout->appendRow(
      RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
  physicsLayout->appendRow(1.f);
  physicsLayout->appendRow(
      RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
  physicsLayout->appendCol(
      RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed));
  physicsLayout->appendCol(1.f);
  physicsLayout->appendCol(1.f);
  physicsLayout->appendCol(RelativeGridLayout::Size(
      10.f + SCROLL_BAR_THICKNESS, RelativeGridLayout::SizeType::Fixed));
  physics_settings->setLayout(physicsLayout);
  CustomWidget *delayedPanel = physics_settings->add<CustomWidget>();
  physicsLayout->setAnchor(delayedPanel,
                           RelativeGridLayout::makeAnchor(1, 1, 2, 1));
  delayedPanel->setDrawBackground(true);
  delayedPanel->setBackgroundColor(Color(40, 40, 40, 255));
  RelativeGridLayout *relPhysics = new RelativeGridLayout();
  for (int i = 1; i < 10; i++)
    relPhysics->appendRow(RelativeGridLayout::Size(
        (i % 2) ? 1.f : 45.f, RelativeGridLayout::SizeType::Fixed));
  relPhysics->appendCol(
      RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));
  relPhysics->appendCol(
      RelativeGridLayout::Size(100.f, RelativeGridLayout::SizeType::Fixed));
  for (int i = 0; i < 13; i++)
    relPhysics->appendCol((i % 2)
                              ? 1.f
                              : RelativeGridLayout::Size(
                                    1.f, RelativeGridLayout::SizeType::Fixed));
  delayedPanel->setLayout(relPhysics);

  // Source settings tab
  sourceSettings = modeTabs->createTab("Neutron source");
  sourceSettings->setId("Neutron source tab");
  RelativeGridLayout *sourceLayout = new RelativeGridLayout();
  sourceLayout->appendRow(
      RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
  for (int j = 0; j < 11; j++) {
    sourceLayout->appendRow(RelativeGridLayout::Size(
        (j == 2) ? 1.f : 30.f, RelativeGridLayout::SizeType::Fixed));
  }
  sourceLayout->appendRow(1.f);
  sourceLayout->appendRow(
      RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
  sourceLayout->appendCol(1.f);
  sourceLayout->appendCol(
      RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));
  sourceLayout->appendCol(
      RelativeGridLayout::Size(350.f, RelativeGridLayout::SizeType::Fixed));
  sourceLayout->appendCol(RelativeGridLayout::Size(
      SCROLL_BAR_THICKNESS, RelativeGridLayout::SizeType::Fixed));
  sourceSettings->setLayout(sourceLayout);
  // source seperator
  CustomWidget *ns_border = sourceSettings->add<CustomWidget>();
  ns_border->setDrawBackground(true);
  ns_border->setBackgroundColor(coolBlue);
  sourceLayout->setAnchor(ns_border, RelativeGridLayout::makeAnchor(1, 3, 2));
  // Source base
  sourceActivityBox = makeSettingLabel<FloatBox<double>>(
      sourceSettings, "Base intensity: ", 100,
      properties->neutronSourceActivity);
  sourceLayout->setAnchor(
      sourceActivityBox->parent(),
      RelativeGridLayout::makeAnchor(2, 1, 1, 1, Alignment::Minimum,
                                     Alignment::Middle));
  sourceActivityBox->setAlignment(TextBox::Alignment::Left);
  sourceActivityBox->setFixedSize(Vector2i(125, 30));
  sourceActivityBox->setSpinnable(true);
  sourceActivityBox->setValueIncrement(5e4);
  sourceActivityBox->setDefaultValue(
      std::to_string(sourceActivityBox->value()));
  sourceActivityBox->setMinValue(0.);
  sourceActivityBox->setFormat(SCI_NUMBER_FORMAT);
  sourceActivityBox->setUnits("n/s");
  sourceActivityBox->setCallback([this](double change) {
    properties->neutronSourceActivity = change;
    reactor->setNeutronSourceActivity(change);
  });
  neutronSourceModeBox =
      makeSettingLabel<ComboBox>(sourceSettings, "Source type:", 100, ns_modes);
  sourceLayout->setAnchor(
      neutronSourceModeBox->parent(),
      RelativeGridLayout::makeAnchor(2, 2, 1, 1, Alignment::Minimum,
                                     Alignment::Middle));
  neutronSourceModeBox->setFixedWidth(125);
  neutronSourceModeBox->setCallback([this](int change) {
    properties->ns_mode = (char)change;
    reactor->setNeutronSourceMode((SimulationModes)change);
    updateNeutronSourceTab();
  });
  neutronSourceModeBox->setSelectedIndex(0);
  // Source graph
  sourceGraph =
      sourceSettings->add<CustomGraph>(2, "Neutron source simulation");
  sourceGraph->setDrawBackground(true);
  sourceGraph->setBackgroundColor(Color(32, 255));
  sourceLayout->setAnchor(
      sourceGraph, RelativeGridLayout::makeAnchor(0, 0, 1, 14, Alignment::Fill,
                                                  Alignment::Fill));
  sourceGraph->setPadding(90.f, 20.f, 10.f, 70.f);
  sourceGraph->setPlotBackgroundColor(Color(60, 255));
  sourceGraph->setPlotGridColor(Color(177, 255));
  sourceGraph->setPlotBorderColor(Color(200, 255));
  PeriodicalMode *per;
  // source general
  for (int i = 0; i < 3; i++) {
    per = reactor->getSourceModeClass((SimulationModes)(i + 1));
    neutronSourcePeriodBoxes[i] = makeSettingLabel<FloatBox<float>>(
        sourceSettings, "Period: ", 100, per->getPeriod());
    sourceLayout->setAnchor(
        neutronSourcePeriodBoxes[i]->parent(),
        RelativeGridLayout::makeAnchor(2, 4, 1, 1, Alignment::Minimum,
                                       Alignment::Middle));
    neutronSourcePeriodBoxes[i]->setAlignment(TextBox::Alignment::Left);
    neutronSourcePeriodBoxes[i]->setFixedWidth(100);
    neutronSourcePeriodBoxes[i]->setFormat(SCI_NUMBER_FORMAT);
    neutronSourcePeriodBoxes[i]->setUnits("s");
    neutronSourcePeriodBoxes[i]->setMinValue(0.1f);
    neutronSourcePeriodBoxes[i]->setValueIncrement(0.1f);
    neutronSourcePeriodBoxes[i]->setSpinnable(true);
    neutronSourcePeriodBoxes[i]->setDefaultValue(
        formatDecimalsDouble(per->getPeriod(), 1));
    neutronSourcePeriodBoxes[i]->setCallback([this, i, per](float change) {
      switch (i) {
        case 0:
          properties->squareWave.period = change;
          break;
        case 1:
          properties->sineMode.period = change;
          break;
        case 2:
          properties->sawToothMode.period = change;
          break;
      }
      per->setPeriod(change);
      updateNeutronSourceTab();
    });
    neutronSourceAmplitudeBoxes[i] = makeSettingLabel<FloatBox<float>>(
        sourceSettings, "Intensity: ", 100, per->getAmplitude());
    sourceLayout->setAnchor(neutronSourceAmplitudeBoxes[i]->parent(),
                            RelativeGridLayout::makeAnchor(2, 5));
    neutronSourceAmplitudeBoxes[i]->setAlignment(TextBox::Alignment::Left);
    neutronSourceAmplitudeBoxes[i]->setFixedWidth(100);
    neutronSourceAmplitudeBoxes[i]->setFormat(SCI_NUMBER_FORMAT);
    neutronSourceAmplitudeBoxes[i]->setUnits("n/s");
    neutronSourceAmplitudeBoxes[i]->setMinValue(0.f);
    neutronSourceAmplitudeBoxes[i]->setValueIncrement(1e4);
    neutronSourceAmplitudeBoxes[i]->setSpinnable(true);
    neutronSourceAmplitudeBoxes[i]->setDefaultValue(
        to_string(per->getAmplitude()));
    neutronSourceAmplitudeBoxes[i]->setCallback([per, i, this](float change) {
      switch (i) {
        case 0:
          properties->squareWave.amplitude = change;
          break;
        case 1:
          properties->sineMode.amplitude = change;
          break;
        case 2:
          properties->sawToothMode.amplitude = change;
          break;
      }
      per->setAmplitude(change);
      updateNeutronSourceTab();
    });
  }
  // source SQW
  float sqPeriod = properties->ns_squareWave.period;
  for (int sqw = 0; sqw < 4; sqw++) {
    neutronSourceSQWBoxes[sqw] = makeSimulationSetting(
        sourceSettings,
        (int)roundf(properties->ns_squareWave.xIndex[sqw] * 100),
        sqw_settingNames[sqw]);
    sourceLayout->setAnchor(neutronSourceSQWBoxes[sqw]->parent(),
                            RelativeGridLayout::makeAnchor(2, 6 + sqw));
    neutronSourceSQWBoxes[sqw]->setCallback([this, sqw, sqPeriod](int change) {
      float val = change / 100.f;
      reactor->source_sqw->xIndex[sqw] = val;
      properties->squareWave.xIndex[sqw] = val;
      if (sqw != 3) {
        neutronSourceSQWBoxes[sqw + 1]->setMinValue(change);
        if (neutronSourceSQWBoxes[sqw + 1]->value() < change) {
          squareWaveBoxes[sqw + 1]->setValue(change);
        } else {
          updateNeutronSourceTab();
        }
      } else {
        updateNeutronSourceTab();
      }
    });
  }
  // source sine
  neutronSourceSINEModeBox =
      makeSettingLabel<ComboBox>(sourceSettings, "Sine type: ", 100, sineModes);
  sourceLayout->setAnchor(neutronSourceSINEModeBox->parent(),
                          RelativeGridLayout::makeAnchor(2, 6));
  neutronSourceSINEModeBox->setFixedWidth(125);
  neutronSourceSINEModeBox->setCallback([this](int change) {
    properties->ns_sineMode.mode = (Settings::SineSettings::SineMode)change;
    reactor->source_sinMode->mode = (Sine::SineMode)change;
    updateNeutronSourceTab();
  });
  // source saw tooth
  float stPeriod = properties->ns_sawToothMode.period;
  for (int saw = 0; saw < 6; saw++) {
    neutronSourceSAWBoxes[saw] = makeSimulationSetting(
        sourceSettings, (int)roundf(properties->sawToothMode.xIndex[saw] * 100),
        saw_settingNames[saw]);
    sourceLayout->setAnchor(neutronSourceSAWBoxes[saw]->parent(),
                            RelativeGridLayout::makeAnchor(2, 6 + saw));
    neutronSourceSAWBoxes[saw]->setCallback([this, stPeriod, saw](int change) {
      float val = change / 100.f;
      properties->ns_sawToothMode.xIndex[saw] = val;
      reactor->source_saw->xIndex[saw] = val;
      if (saw < 5) {
        neutronSourceSAWBoxes[saw + 1]->setMinValue(change);
        if (neutronSourceSAWBoxes[saw + 1]->value() < change) {
          neutronSourceSAWBoxes[saw + 1]->setValue(change);
        } else {
          updateNeutronSourceTab();
        }
      } else {
        updateNeutronSourceTab();
      }
    });
  }
  updateNeutronSourceTab();

  /* PHYSICS */
  // Texts and borders
  for (int i = 0; i < 6; i++) {
    Label *textLabel = delayedPanel->add<Label>(
        "Group " + std::to_string(i + 1) +
            ((i == 5 || i == 0) ? (i == 5 ? " (fastest)" : " (slowest)") : ""),
        "sans-bold");
    relPhysics->setAnchor(textLabel, RelativeGridLayout::makeAnchor(
                                         (i + 1) * 2 + 1, 1, 1, 1,
                                         Alignment::Middle, Alignment::Middle));
  }
  CustomWidget *border;
  for (int i = 0; i < 8; i++) {
    border = delayedPanel->add<CustomWidget>();
    border->setDrawBackground(true);
    border->setBackgroundColor(coolBlue);
    relPhysics->setAnchor(border,
                          RelativeGridLayout::makeAnchor(i * 2, 0, 1, 9));
  }
  CustomLabel *textLabel;
  std::string rowText[3] = {"Beta(i):", "Lambda(i):", "Enabled:"};
  for (int i = 0; i < 3; i++) {
    textLabel = delayedPanel->add<CustomLabel>(rowText[i], "sans-bold");
    textLabel->setPadding(0, 7.f);
    relPhysics->setAnchor(
        textLabel,
        RelativeGridLayout::makeAnchor(0, 2 * (i + 1) + 1, 1, 1,
                                       Alignment::Minimum, Alignment::Middle));
  }

  // Data inputs
  for (int row = 0; row < 4; row++) {
    border = delayedPanel->add<CustomWidget>();
    border->setDrawBackground(true);
    border->setBackgroundColor(coolBlue);
    relPhysics->setAnchor(border,
                          RelativeGridLayout::makeAnchor(0, 2 * row, 15));
    switch (row) {
      case 2: {
        for (int i = 0; i < 6; i++) {
          delayedGroupsEnabledBoxes[i] = delayedPanel->add<SliderCheckBox>();
          delayedGroupsEnabledBoxes[i]->setChecked(
              properties->groupsEnabled[i]);
          relPhysics->setAnchor(delayedGroupsEnabledBoxes[i],
                                RelativeGridLayout::makeAnchor(
                                    2 * (i + 1) + 1, 7, 1, 1, Alignment::Middle,
                                    Alignment::Middle));
          delayedGroupsEnabledBoxes[i]->setCallback([this, i](bool change) {
            properties->groupsEnabled[i] = change;
            reactor->setDelayedGroupEnabled(i, change);
          });
        }
        break;
      }
      case 3: {
        border = delayedPanel->add<CustomWidget>();
        border->setDrawBackground(true);
        border->setBackgroundColor(coolBlue);
        relPhysics->setAnchor(
            border, RelativeGridLayout::makeAnchor(0, 2 * (row + 1), 15));
        continue;
        break;
      }
      default: {
        for (int i = 0; i < 6; i++) {
          size_t index = 6 * row + i;
          delayedGroupBoxes[index] = delayedPanel->add<FloatBox<double>>(
              row ? properties->lambdas[i] : properties->betas[i]);
          relPhysics->setAnchor(delayedGroupBoxes[index],
                                RelativeGridLayout::makeAnchor(
                                    2 * (i + 1) + 1, 2 * (row + 1) + 1, 1, 1,
                                    Alignment::Middle, Alignment::Middle));
          delayedGroupBoxes[index]->setFixedSize(Vector2i(130, 30));
          delayedGroupBoxes[index]->setSpinnable(true);
          delayedGroupBoxes[index]->setValueIncrement(0.00001f);
          delayedGroupBoxes[index]->setDefaultValue(
              std::to_string(delayedGroupBoxes[index]->value()));
          delayedGroupBoxes[index]->setMinValue(0.0000001f);
          delayedGroupBoxes[index]->setFormat(SCI_NUMBER_FORMAT);
          delayedGroupBoxes[index]->setCallback([this, row, i](double change) {
            if (row) {
              properties->lambdas[i] = change;
              reactor->setDelayedGroupDecay(i, change);
            } else {
              properties->betas[i] = change;
              reactor->setDelayedGroupFraction(i, change);
            }
          });
        }
      }
    }
  }

  // Create panel for lower left settings
  Widget *settingsVert = physics_settings->add<Widget>();
  physicsLayout->setAnchor(settingsVert,
                           RelativeGridLayout::makeAnchor(1, 3, 1, 1));
  settingsVert->setLayout(
      new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 15));

  // Core volume
  Widget *corePanel = settingsVert->add<Widget>();

  corePanel->setLayout(panelsLayout);
  corePanel->add<Label>("Core volume: ", "sans-bold");
  coreVolumeBox =
      corePanel->add<FloatBox<double>>(properties->coreVolume * 1e3);
  coreVolumeBox->setAlignment(TextBox::Alignment::Left);
  coreVolumeBox->setFixedSize(Vector2i(180, 30));
  coreVolumeBox->setSpinnable(true);
  coreVolumeBox->setValueIncrement(1.);
  coreVolumeBox->setDefaultValue(std::to_string(coreVolumeBox->value()));
  coreVolumeBox->setMinValue(1.);
  coreVolumeBox->setFormat(SCI_NUMBER_FORMAT);
  coreVolumeBox->setUnits("L");
  coreVolumeBox->setCallback([this](double change) {
    reactor->setReactorCoreVolume(change * 1e-03);
    properties->coreVolume = change * 1e-03;
  });

  Widget *waterPanel = settingsVert->add<Widget>();
  waterPanel->setLayout(panelsLayout);
  waterPanel->add<Label>("Water volume: ", "sans-bold");
  waterVolumeInput = waterPanel->add<FloatBox<double>>(properties->waterVolume);
  waterVolumeInput->setFixedSize(Vector2i(100, 30));
  waterVolumeInput->setAlignment(TextBox::Alignment::Left);
  waterVolumeInput->setSpinnable(true);
  waterVolumeInput->setValueIncrement(1.);
  waterVolumeInput->setDefaultValue(std::to_string(waterVolumeInput->value()));
  waterVolumeInput->setMinValue(1.);
  waterVolumeInput->setFormat(SCI_NUMBER_FORMAT);
  waterVolumeInput->setUnits("m" + string(utf8(0xB3).data()));
  waterVolumeInput->setCallback([this](double change) {
    reactor->setWaterVolume(change);
    properties->waterVolume = change;
  });

  // Create a panel for temperature effects
  Widget *checkBoxPanelTemperatureEffects = settingsVert->add<Widget>();
  checkBoxPanelTemperatureEffects->setLayout(panelsLayout);
  // Create a panel for Fission poisoning
  Widget *checkBoxPanelFissionPoisoning = settingsVert->add<Widget>();
  checkBoxPanelFissionPoisoning->setLayout(panelsLayout);
  // Create a panel for excess reactivity
  Widget *excessPanel = settingsVert->add<Widget>();
  excessPanel->setLayout(panelsLayout);
  // Create a panel for cooling power
  Widget *waterCoolingPowerPanel = settingsVert->add<Widget>();
  waterCoolingPowerPanel->setLayout(panelsLayout);
  // Create a panel for prompt neutron lifetime
  Widget *promptPanel = settingsVert->add<Widget>();
  promptPanel->setLayout(panelsLayout);

  {
    checkBoxPanelTemperatureEffects->add<Label>("Temperature effects: ",
                                                "sans-bold");
    tempEffectsBox = checkBoxPanelTemperatureEffects->add<SliderCheckBox>();
    tempEffectsBox->setFontSize(16);
    tempEffectsBox->setChecked(properties->temperatureEffects);
    tempEffectsBox->setCallback([this](bool value) {
      reactor->setTemperatureEffectsEnabled(value);
      properties->temperatureEffects = value;
    });
  }
  {
    checkBoxPanelFissionPoisoning->add<Label>("Xe poisoning: ", "sans-bold");
    fissionProductsBox = checkBoxPanelFissionPoisoning->add<SliderCheckBox>();
    fissionProductsBox->setFontSize(16);
    fissionProductsBox->setChecked(properties->fissionPoisons);
    fissionProductsBox->setCallback([this](bool value) {
      reactor->setFissionPoisoningEffectsEnabled(value);
      properties->fissionPoisons = value;
    });
  }

  excessPanel->add<Label>("Excess reactivity: ", "sans-bold");
  excessReactivityBox =
      excessPanel->add<FloatBox<float>>(properties->excessReactivity);
  excessReactivityBox->setFixedSize(Vector2i(125, 30));
  excessReactivityBox->setUnits("pcm");
  excessReactivityBox->setMinValue(0.f);
  excessReactivityBox->setFormat(SCI_NUMBER_FORMAT);
  excessReactivityBox->setSpinnable(true);
  excessReactivityBox->setAlignment(TextBox::Alignment::Left);
  excessReactivityBox->setDefaultValue(
      std::to_string(excessReactivityBox->value()));
  excessReactivityBox->setValueIncrement(10.);
  excessReactivityBox->setCallback([this](float change) {
    properties->excessReactivity = change;
    reactor->setExcessReactivity(change);
  });

  // Water cooling power
  waterCoolingPowerPanel->add<Label>("Water cooling power: ", "sans-bold");
  coolingPowerBox = waterCoolingPowerPanel->add<FloatBox<double>>(
      properties->waterCoolingPower);
  coolingPowerBox->setAlignment(TextBox::Alignment::Left);
  coolingPowerBox->setFixedSize(Vector2i(150, 30));
  coolingPowerBox->setSpinnable(true);
  coolingPowerBox->setValueIncrement(1.);
  coolingPowerBox->setDefaultValue(std::to_string(coolingPowerBox->value()));
  coolingPowerBox->setMinValue(1.);
  coolingPowerBox->setFormat(SCI_NUMBER_FORMAT);
  coolingPowerBox->setUnits("W");
  coolingPowerBox->setCallback([this](double change) {
    properties->waterCoolingPower = change;
    reactor->setCoolingPower(change);
  });

  // Prompt neutron lifetime
  promptPanel->add<Label>("Prompt neutron lifetime: ", "sans-bold");
  promptNeutronLifetimeBox =
      promptPanel->add<FloatBox<double>>(properties->promptNeutronLifetime);
  promptNeutronLifetimeBox->setAlignment(TextBox::Alignment::Left);
  promptNeutronLifetimeBox->setFixedSize(Vector2i(150, 30));
  promptNeutronLifetimeBox->setSpinnable(true);
  promptNeutronLifetimeBox->setValueIncrement(1e-6);
  promptNeutronLifetimeBox->setDefaultValue(
      std::to_string(promptNeutronLifetimeBox->value()));
  promptNeutronLifetimeBox->setMinMaxValues(1e-7, 1.);
  promptNeutronLifetimeBox->setFormat(SCI_NUMBER_FORMAT);
  promptNeutronLifetimeBox->setUnits("s");
  promptNeutronLifetimeBox->setCallback([this](double change) {
    properties->promptNeutronLifetime = change;
    reactor->setPromptNeutronLifetime(change);
  });

  // Alpha panel
  Widget *alphaPanel = physics_settings->add<Widget>();
  physicsLayout->setAnchor(alphaPanel,
                           RelativeGridLayout::makeAnchor(2, 3, 1, 1));
  alphaPanel->setLayout(
      new BoxLayout(Orientation::Vertical, Alignment::Fill, 0, 10));
  // Temperature reactivity coef.
  CustomGraph *alphaGraph =
      alphaPanel->add<CustomGraph>(1, "Temp. reactivity coef.");
  alphaGraph->setBackgroundColor(Color(60, 255));
  alphaGraph->setDrawBackground(true);
  alphaGraph->setFixedHeight(250);
  alphaGraph->setPadding(80.f, 25.f, 30.f, 65.f);
  alphaGraph->setTextColor(Color(0, 255));

  alphaPlot = alphaGraph->addPlot(sigmaPoints, false);
  alphaPlot->setName("-" + alpha);
  alphaPlot->setHorizontalName("Temperature");
  alphaPlot->setTextColor(Color(250, 255));
  alphaPlot->setTextOffset(30.f);
  alphaPlot->setColor(Color(0, 120, 255, 255));
  alphaPlot->setPointerColor(Color(0, 120, 255, 255));
  alphaPlot->setPointerOverride(true);
  alphaPlot->setFill(true);
  alphaPlot->setFillColor(Color(0, 120, 255, 150));
  alphaPlot->setAxisShown(true);
  alphaPlot->setMainLineShown(true);
  alphaPlot->setAxisColor(Color(250, 255));
  alphaPlot->setMajorTickNumber(2);
  alphaPlot->setMinorTickNumber(4);
  alphaPlot->setTextShown(true);
  alphaPlot->setMainTickFontSize(22.f);
  alphaPlot->setMajorTickFontSize(20.f);
  alphaPlot->setUnits("pcm/" + degCelsiusUnit);
  alphaPlot->setHorizontalPointerColor(Color(120, 0, 255, 255));
  alphaPlot->setHorizontalAxisShown(true);
  alphaPlot->setHorizontalMainLineShown(true);
  alphaPlot->setHorizontalPointerShown(true);
  alphaPlot->setHorizontalMajorTickNumber(3);
  alphaPlot->setHorizontalMinorTickNumber(4);
  alphaPlot->setHorizontalUnits(degCelsiusUnit);
  alphaPlot->setDrawMode(DrawMode::Default);
  alphaPlot->setPlotRange(0, 2);

  alphaPlot->setLimits(0., 1000., 0., 15.);
  updateAlphaGraph();
  alphaPlot->setXdata(alphaX);
  alphaPlot->setYdata(alphaY);

  // Alpha settings
  string alphaText[4] = {"Starting " + alpha + ":", "Peak temp.:",
                         alpha + " at peak temp.:", "Slope after peak temp.:"};
  float vals[4] = {properties->alpha0, properties->alphaT1,
                   properties->alphaAtT1, (float)properties->alphaK};
  float increments[4] = {.1f, 5.f, .1f, .001f};
  float maxVals[3] = {20.f, 1000.f, 120.f};
  string units[4] = {"pcm/" + degCelsiusUnit, degCelsiusUnit,
                     "pcm/" + degCelsiusUnit,
                     "pcm/" + degCelsiusUnit + string(utf8(0xB2).data())};
  for (int i = 0; i < 4; i++) {
    // Create a panel for prompt neutron lifetime
    Widget *alphaSettingPanel = alphaPanel->add<Widget>();
    alphaSettingPanel->setLayout(panelsLayout);
    alphaSettingPanel->add<Label>(alphaText[i] + " ", "sans-bold");
    FloatBox<float> *temp = alphaSettingPanel->add<FloatBox<float>>(vals[i]);
    temp->setAlignment(TextBox::Alignment::Left);
    temp->setFixedSize(Vector2i(150, 30));
    temp->setSpinnable(true);
    temp->setValueIncrement(increments[i]);
    temp->setDefaultValue(std::to_string(temp->value()));
    if (i < 3) {
      temp->setMinMaxValues((i == 1) ? 0.f : -maxVals[i], maxVals[i]);
    } else {
      temp->setMinMaxValues(-4.f, 4.f);
    }
    temp->setFormat((i == 1) ? SCI_NUMBER_FORMAT : SCI_NUMBER_FORMAT_NEG);
    temp->setUnits(units[i]);
    switch (i) {
      case 0:
        alpha0Box = temp;
        break;
      case 1:
        tempPeakBox = temp;
        break;
      case 2:
        alphaPeakBox = temp;
        break;
      case 3:
        alphaSlopeBox = temp;
        break;
    }
  }

  alpha0Box->setCallback([this](float change) {
    properties->alpha0 = change;
    reactor->setAlpha0(change);
    updateAlphaGraph();
  });
  tempPeakBox->setCallback([this](float change) {
    properties->alphaT1 = change;
    reactor->setAlphaTempPeak(change);
    updateAlphaGraph();
  });
  alphaPeakBox->setCallback([this](float change) {
    properties->alphaAtT1 = change;
    reactor->setAlphaPeak(change);
    updateAlphaGraph();
  });
  alphaSlopeBox->setCallback([this](float change) {
    properties->alphaK = (double)change;
    reactor->setAlphaSlope((double)change);
    updateAlphaGraph();
  });

  modeTabs->setActiveTab(0);
}
