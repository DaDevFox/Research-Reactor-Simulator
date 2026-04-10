#include "../include/SimulatorGUI.h"

void SimulatorGUI::createOperationalLimitsTab() {
  CustomWidget *limits_tab = tabControl->createTab("Operational limits");
  limits_tab->setId("op. limits tab");
  RelativeGridLayout *rel = new RelativeGridLayout();
  rel->appendCol(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // left border
  rel->appendCol(RelativeGridLayout::Size(
      200.f, RelativeGridLayout::SizeType::Fixed));  // text
  rel->appendCol(RelativeGridLayout::Size(
      100.f, RelativeGridLayout::SizeType::Fixed));  // data
  rel->appendCol(1.f);                               // check boxes
  rel->appendRow(RelativeGridLayout::Size(
      15.f, RelativeGridLayout::SizeType::Fixed));  // top border
  limits_tab->setLayout(rel);

  // Skips water level
  for (int i = 0; i < 4; i++) {
    rel->appendRow(
        RelativeGridLayout::Size(27.5f, RelativeGridLayout::SizeType::Fixed));
    Label *temp = limits_tab->add<Label>(labels[i] + " SCRAM :", "sans-bold");
    rel->setAnchor(
        temp, RelativeGridLayout::makeAnchor(1, 1 + i, 1, 1, Alignment::Minimum,
                                             Alignment::Middle));
    scramEnabledBoxes[i] = limits_tab->add<SliderCheckBox>();
    RelativeGridLayout::Anchor a = RelativeGridLayout::makeAnchor(
        3, 1 + i, 1, 1, Alignment::Minimum, Alignment::Middle);
    a.padding[0] = 8;
    rel->setAnchor(scramEnabledBoxes[i], a);
    scramEnabledBoxes[i]->setChecked(reactor->getScramEnabled(reasons[i]));
    scramEnabledBoxes[i]->setCallback([this, i](bool checked) {
      reactor->setScramEnabled(reasons[i], checked);
      switch (i) {
        case 0:
          properties->periodScram = checked;
          break;
        case 1:
          properties->powerScram = checked;
          break;
        case 2:
          properties->tempScram = checked;
          break;
        case 3:
          properties->waterTempScram = checked;
          break;
        case 4:
          properties->waterLevelScram = checked;
          break;
      }
    });
  }

  // Create the period limit
  periodLimBox = limits_tab->add<IntBox<float>>((float)properties->periodLimit);
  rel->setAnchor(periodLimBox,
                 RelativeGridLayout::makeAnchor(2, 1, 1, 1, Alignment::Fill,
                                                Alignment::Middle));
  periodLimBox->setFixedSize(Vector2i(100, 20));
  periodLimBox->setUnits("s");
  periodLimBox->setDefaultValue(
      formatDecimalsDouble((float)properties->periodLimit, 1));
  periodLimBox->setFontSize(16);
  periodLimBox->setFormat(SCI_NUMBER_FORMAT);
  periodLimBox->setMinMaxValues(0.f, 3600.f);
  periodLimBox->setValueIncrement(0.1f);
  periodLimBox->setCallback([this](float a) {
    properties->periodLimit = a;
    reactor->setPeriodLimit(a);
  });

  // Create the power limit
  powerLimBox =
      limits_tab->add<FloatBox<double>>(properties->powerLimit / 1000.);
  rel->setAnchor(powerLimBox,
                 RelativeGridLayout::makeAnchor(2, 2, 1, 1, Alignment::Fill,
                                                Alignment::Middle));
  powerLimBox->setFixedSize(Vector2i(100, 20));
  powerLimBox->setUnits("kW");
  powerLimBox->setDefaultValue(to_string(powerLimBox->value()));
  powerLimBox->setFontSize(16);
  powerLimBox->setFormat(SCI_NUMBER_FORMAT);
  powerLimBox->setMinMaxValues(0., 1e12);
  powerLimBox->setValueIncrement(1e2);
  powerLimBox->setCallback([this](double a) {
    properties->powerLimit = a * 1e3;
    reactor->setPowerLimit(a * 1e3);
  });

  // Create the fuel temperature limit
  fuel_tempLimBox = limits_tab->add<FloatBox<float>>(properties->tempLimit);
  rel->setAnchor(fuel_tempLimBox,
                 RelativeGridLayout::makeAnchor(2, 3, 1, 1, Alignment::Fill,
                                                Alignment::Middle));
  fuel_tempLimBox->setFixedSize(Vector2i(100, 20));
  fuel_tempLimBox->setUnits(degCelsiusUnit);
  fuel_tempLimBox->setDefaultValue(to_string(fuel_tempLimBox->value()));
  fuel_tempLimBox->setFontSize(16);
  fuel_tempLimBox->setFormat(SCI_NUMBER_FORMAT);
  fuel_tempLimBox->setMinValue((int)ENVIRONMENT_TEMPERATURE_DEFAULT);
  fuel_tempLimBox->setValueIncrement(10);
  fuel_tempLimBox->setCallback([this](float a) {
    properties->tempLimit = a;
    reactor->setFuelTemperatureLimit(a);
  });

  // Create the water temperature limit
  water_tempLimBox =
      limits_tab->add<FloatBox<float>>(properties->waterTempLimit);
  rel->setAnchor(water_tempLimBox,
                 RelativeGridLayout::makeAnchor(2, 4, 1, 1, Alignment::Fill,
                                                Alignment::Middle));
  water_tempLimBox->setFixedSize(Vector2i(100, 20));
  water_tempLimBox->setUnits(degCelsiusUnit);
  water_tempLimBox->setDefaultValue(to_string(water_tempLimBox->value()));
  water_tempLimBox->setFontSize(16);
  water_tempLimBox->setFormat(SCI_NUMBER_FORMAT);
  water_tempLimBox->setMinMaxValues(0, 100);
  water_tempLimBox->setValueIncrement(10);
  water_tempLimBox->setCallback([this](float a) {
    properties->waterTempLimit = a;
    reactor->setWaterTemperatureLimit(a);
  });

  // Create the water level limit
  /*water_levelLimBox =
  limits_tab->add<FloatBox<float>>(properties->waterLevelLimit);
  rel->setAnchor(water_levelLimBox, RelativeGridLayout::makeAnchor(2, 5, 1, 1,
  Alignment::Fill, Alignment::Middle));
  water_levelLimBox->setFixedSize(Vector2i(100, 20));
  water_levelLimBox->setUnits("m");
  water_levelLimBox->setDefaultValue(to_string(water_levelLimBox->value()));
  water_levelLimBox->setFontSize(16);
  water_levelLimBox->setFormat("[0-9]*\\.?[0-9]+");
  water_levelLimBox->setMinValue(0.f);
  water_levelLimBox->setValueIncrement(0.1f);
  water_levelLimBox->setCallback([this](float a) {
          properties->waterLevelLimit = a;
          reactor->setWaterLevelLimit(a);
  });
  water_levelLimBox->setEnabled(!DEMO_VERSION);
  water_levelLimBox->setEditable(!DEMO_VERSION);*/

  // Create a panel for rod reactivity plot visibility
  rel->appendRow(
      RelativeGridLayout::Size(27.5f, RelativeGridLayout::SizeType::Fixed));
  CustomWidget *checkBoxPanelRods = limits_tab->add<CustomWidget>();
  rel->setAnchor(checkBoxPanelRods, RelativeGridLayout::makeAnchor(1, 5));
  checkBoxPanelRods->setLayout(panelsLayout);
  checkBoxPanelRods->add<Label>("All control rods at once: ", "sans-bold");
  allRodsBox = checkBoxPanelRods->add<SliderCheckBox>();
  allRodsBox->setFontSize(16);
  allRodsBox->setChecked(properties->allRodsAtOnce);
  allRodsBox->setCallback(
      [this](bool value) { properties->allRodsAtOnce = value; });

  // Create a panel for rod reactivity plot visibility
  rel->appendRow(
      RelativeGridLayout::Size(27.5f, RelativeGridLayout::SizeType::Fixed));
  CustomWidget *checkBoxPanelAutoScram = limits_tab->add<CustomWidget>();
  rel->setAnchor(checkBoxPanelAutoScram, RelativeGridLayout::makeAnchor(1, 6));
  checkBoxPanelAutoScram->setLayout(panelsLayout);
  checkBoxPanelAutoScram->add<Label>("SCRAM after pulse: ", "sans-bold");
  autoScramBox = checkBoxPanelAutoScram->add<SliderCheckBox>();
  autoScramBox->setFontSize(16);
  autoScramBox->setChecked(properties->automaticPulseScram);
  autoScramBox->setCallback([this](bool value) {
    properties->automaticPulseScram = value;
    reactor->setAutoScram(value);
  });
}
