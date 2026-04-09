#include "../include/SimulatorGUI.h"

void SimulatorGUI::createOperationModesTab()
{
		Widget *modes_base = tabControl->createTab("Operation modes", false);
		CustomTabWidget *modeTabs = modes_base->add<CustomTabWidget>();
		modeTabs->header()->setStretch(true);
		modeTabs->header()->setButtonAlignment(NVGalign::NVG_ALIGN_MIDDLE | NVGalign::NVG_ALIGN_CENTER);
		RelativeGridLayout *grid = new RelativeGridLayout();
		grid->appendCol(1.f);
		grid->appendRow(1.f);
		modes_base->setLayout(grid);
		grid->setAnchor(modeTabs, RelativeGridLayout::makeAnchor(0, 0));

		// Create titles, periods and amplitudes
		std::string titles[3] = {"Square wave", "Sine wave", "Saw tooth"};
		RelativeGridLayout *layouts[3];
		CustomWidget *tabs[3];
		for (int i = 0; i < 3; i++)
		{
			tabs[i] = modeTabs->createTab(titles[i]);
			tabs[i]->setId(titles[i] + " tab");
			tabs[i]->setDrawBackground(true);
			tabs[i]->setBackgroundColor(Color(100, 255));

			layouts[i] = new RelativeGridLayout();
			layouts[i]->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));	 // left border
			layouts[i]->appendCol(RelativeGridLayout::Size(250.f, RelativeGridLayout::SizeType::Fixed)); // settings
			layouts[i]->appendCol(1.f);																	 // graph
			layouts[i]->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));	 // right border
			layouts[i]->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));	 // top border
			for (int j = 0; j < simModeFields[i]; j++)
			{
				layouts[i]->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));
			}
			layouts[i]->appendRow(1.f);																	// empty space
			layouts[i]->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed)); // bottom border
			tabs[i]->setLayout(layouts[i]);

			PeriodicalMode *use = reactor->regulatingRod()->getMode((SimulationModes)(i + 1));
			size_t arraySize = use->num_points();

			// create graph first just for event binding
			CustomGraph *graph = tabs[i]->add<CustomGraph>(2, titles[i]);
			graph->setDrawBackground(true);
			graph->setBackgroundColor(Color(32, 255));
			layouts[i]->setAnchor(graph, RelativeGridLayout::makeAnchor(2, 0, 2, simModeFields[i] + 3, Alignment::Fill, Alignment::Fill));
			graph->setPadding(90.f, 20.f, 10.f, 70.f);
			graph->setPlotBackgroundColor(Color(60, 255));
			graph->setPlotGridColor(Color(177, 255));
			graph->setPlotBorderColor(Color(200, 255));
			operationModes[i] = graph->addPlot(arraySize);
			operationModes[i]->setPlotRange(0, arraySize - 1);
			operationModes[i]->setLimits(0., use->getPeriod(), -1.5 * use->getAmplitude(), 1.5 * use->getAmplitude());
			operationModesTrackers[i] = graph->addPlot(2);
			operationModesTrackers[i]->setPlotRange(0, 1);
			operationModesTrackers[i]->setDrawMode(DrawMode::Default);
			operationModesTrackers[i]->setAxisShown(false);
			operationModesTrackers[i]->setColor(Color(1.f, 0.f, 0.f, 1.f));
			operationModesTrackers[i]->setHorizontalAxisShown(false);
			operationModesTrackers[i]->setTextShown(false);
			operationModesTrackers[i]->setPointerShown(false);
			operationModesTrackers[i]->setXdata(use->getTrackerArray());
			operationModesTrackers[i]->setYdata(trackerY);
			operationModesTrackers[i]->setLimits(0., use->getPeriod(), 0., 1.);

			// Make arrays
			for (size_t p = 0; p < 2; p++)
				operationModesPlots[p][i] = new double[arraySize];
			// Fill arrays
			use->fillXYaxis(operationModesPlots[0][i], operationModesPlots[1][i]);

			// period
			periodBoxes[i] = makeSettingLabel<FloatBox<float>>(tabs[i], "Period: ", 100, use->getPeriod());
			layouts[i]->setAnchor(periodBoxes[i]->parent(), RelativeGridLayout::makeAnchor(1, 1, 1, 1, Alignment::Minimum, Alignment::Middle));
			periodBoxes[i]->setFixedWidth(100);
			periodBoxes[i]->setAlignment(TextBox::Alignment::Left);
			periodBoxes[i]->setFormat(SCI_NUMBER_FORMAT);
			periodBoxes[i]->setUnits("s");
			periodBoxes[i]->setMinValue(0.1f);
			periodBoxes[i]->setValueIncrement(0.1f);
			periodBoxes[i]->setSpinnable(true);
			periodBoxes[i]->setDefaultValue(formatDecimalsDouble(use->getPeriod(), 1));
			periodBoxes[i]->setCallback([use, i, this](float change)
										{
					switch (i)
					{
					case 0: properties->squareWave.period = change; break;
					case 1: properties->sineMode.period = change; break;
					case 2: properties->sawToothMode.period = change; break;
					}
					use->setPeriod(change);
					use->fillXYaxis(operationModesPlots[0][i], operationModesPlots[1][i]);
					operationModes[i]->setLimits(0., change, -1.5 * use->getAmplitude(), 1.5 * use->getAmplitude());
					operationModesTrackers[i]->setLimits(0., use->getPeriod(), 0., 1.); });

			// amplitude
			amplitudeBoxes[i] = makeSettingLabel<FloatBox<float>>(tabs[i], "Amplitude: ", 100, use->getAmplitude());
			layouts[i]->setAnchor(amplitudeBoxes[i]->parent(), RelativeGridLayout::makeAnchor(1, 2, 1, 1, Alignment::Minimum, Alignment::Middle));
			amplitudeBoxes[i]->setFixedWidth(100);
			amplitudeBoxes[i]->setAlignment(TextBox::Alignment::Left);
			amplitudeBoxes[i]->setFormat(SCI_NUMBER_FORMAT);
			amplitudeBoxes[i]->setUnits("steps");
			amplitudeBoxes[i]->setMinValue(0.f);
			amplitudeBoxes[i]->setValueIncrement(1);
			amplitudeBoxes[i]->setSpinnable(true);
			amplitudeBoxes[i]->setDefaultValue(to_string(use->getAmplitude()));
			amplitudeBoxes[i]->setCallback([use, i, this](float change)
										   {
					switch (i)
					{
					case 0: properties->squareWave.amplitude = change; break;
					case 1: properties->sineMode.amplitude = change; break;
					case 2: properties->sawToothMode.amplitude = change; break;
					}
					use->setAmplitude(change);
					use->fillXYaxis(operationModesPlots[0][i], operationModesPlots[1][i]);
					operationModes[i]->setLimits(0., use->getPeriod(), -1.5 * change, 1.5 * change); });

			operationModes[i]->setMainTickFontSize(18.f);
			operationModes[i]->setMajorTickFontSize(16.f);
			operationModes[i]->setNameFontSize(24.f);
			operationModes[i]->setPointerShown(false);
			operationModes[i]->setXdata(operationModesPlots[0][i]);
			operationModes[i]->setYdata(operationModesPlots[1][i]);
			operationModes[i]->setColor(coolBlue);
			operationModes[i]->setLimits(0., use->getPeriod(), -1.5 * use->getAmplitude(), 1.5 * use->getAmplitude());
			operationModes[i]->setDrawMode(DrawMode::Default);
			operationModes[i]->setTextColor(Color(250, 255));
			operationModes[i]->setAxisColor(Color(250, 255));
			operationModes[i]->setTextShown(true);
			operationModes[i]->setAxisShown(true);
			operationModes[i]->setUnits("steps");
			operationModes[i]->setName("Delta rod position");
			operationModes[i]->setTextOffset(40.f);
			operationModes[i]->setMainLineShown(true);
			operationModes[i]->setMajorTickNumber(3);
			operationModes[i]->setMinorTickNumber(1);
			operationModes[i]->setHorizontalAxisShown(true);
			operationModes[i]->setHorizontalUnits("s");
			operationModes[i]->setHorizontalName("Time");
			operationModes[i]->setHorizontalMainLineShown(true);
			operationModes[i]->setHorizontalMajorTickNumber(3);
			operationModes[i]->setHorizontalMinorTickNumber(1);
		}

		/* SQUARE WAVE */
		for (int sqw = 0; sqw < 4; sqw++)
		{
			squareWaveBoxes[sqw] = makeSimulationSetting(tabs[0], (int)roundf(properties->squareWave.xIndex[sqw] * 100), sqw_settingNames[sqw]);
			layouts[0]->setAnchor(squareWaveBoxes[sqw]->parent(), RelativeGridLayout::makeAnchor(1, 3 + sqw, 1, 1, Alignment::Minimum, Alignment::Middle));
			squareWaveBoxes[sqw]->setCallback([this, sqw](int change)
											  {
					float val = change / 100.f;
					reactor->regulatingRod()->squareWave()->xIndex[sqw] = val;
					properties->squareWave.xIndex[sqw] = val;
					if (sqw != 3)
					{
						squareWaveBoxes[sqw + 1]->setMinValue(change);
						if (squareWaveBoxes[sqw + 1]->value() < change) squareWaveBoxes[sqw + 1]->setValue(change);
					}
					reactor->regulatingRod()->squareWave()->fillXYaxis(operationModesPlots[0][0], operationModesPlots[1][0]); });
		}
		squareWaveSpeedBox = makeSettingLabel<SliderCheckBox>(tabs[0], "Use finite rod speed: ");
		layouts[0]->setAnchor(squareWaveSpeedBox->parent(), RelativeGridLayout::makeAnchor(1, 7, 1, 1, Alignment::Minimum, Alignment::Middle));
		squareWaveSpeedBox->setCallback([this](bool change)
										{
				properties->squareWaveUsesRodSpeed = change;
				if (change)
				{
					reactor->regulatingRod()->squareWave()->rodSpeed = reactor->regulatingRod()->getRodSpeed();
				}
				else
				{
					reactor->regulatingRod()->squareWave()->rodSpeed = 0.f;
				}
				reactor->regulatingRod()->squareWave()->fillXYaxis(operationModesPlots[0][0], operationModesPlots[1][0]); });

		/* SINE MODE */
		sineModeBox = makeSettingLabel<ComboBox>(tabs[1], "Sine type: ", 0, sineModes);
		layouts[1]->setAnchor(sineModeBox->parent(), RelativeGridLayout::makeAnchor(1, 3, 1, 1, Alignment::Minimum, Alignment::Middle));
		sineModeBox->setFixedWidth(150);
		sineModeBox->setCallback([this](int change)
								 {
				properties->sineMode.mode = (Settings::SineSettings::SineMode)change;
				reactor->regulatingRod()->sine()->mode = (Sine::SineMode)change;
				reactor->regulatingRod()->sine()->fillXYaxis(operationModesPlots[0][1], operationModesPlots[1][1]); });

		/* SAW TOOTH */
		for (int saw = 0; saw < 6; saw++)
		{
			sawToothBoxes[saw] = makeSimulationSetting(tabs[2], (int)roundf(properties->sawToothMode.xIndex[saw] * 100), saw_settingNames[saw]);
			layouts[2]->setAnchor(sawToothBoxes[saw]->parent(), RelativeGridLayout::makeAnchor(1, 3 + saw, 1, 1, Alignment::Minimum, Alignment::Middle));
			sawToothBoxes[saw]->setCallback([this, saw](int change)
											{
					float val = change / 100.f;
					reactor->regulatingRod()->sawTooth()->xIndex[saw] = val;
					properties->sawToothMode.xIndex[saw] = val;
					if (saw < 5)
					{
						sawToothBoxes[saw + 1]->setMinValue(change);
						if (sawToothBoxes[saw + 1]->value() < change) sawToothBoxes[saw + 1]->setValue(change);
					}
					reactor->regulatingRod()->sawTooth()->fillXYaxis(operationModesPlots[0][2], operationModesPlots[1][2]); });
		}

		// AUTOMATIC
		CustomWidget *autoTab = modeTabs->createTab("Automatic");
		RelativeGridLayout *autoLayout = new RelativeGridLayout();
		autoLayout->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed)); // left border
		autoLayout->appendCol(1.f);																	// everything else
		autoLayout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed)); // top border
		for (int i = 0; i < 4; i++)
			autoLayout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed)); // 4 rows for 4 settings
		autoTab->setLayout(autoLayout);

		keepCurrentPowerBox = makeSettingLabel<SliderCheckBox>(autoTab, "Current power stability: ", 0);
		keepCurrentPowerBox->setChecked(properties->steadyCurrentPower);
		autoLayout->setAnchor(keepCurrentPowerBox->parent(), RelativeGridLayout::makeAnchor(1, 1, 1, 1, Alignment::Minimum, Alignment::Middle));

		steadyPowerBox = makeSettingLabel<FloatBox<double>>(autoTab, "Steady power: ", 100, properties->steadyGoalPower);
		steadyPowerBox->setEnabled(!properties->steadyCurrentPower);
		steadyPowerBox->setUnits("W");
		steadyPowerBox->setFormat(SCI_NUMBER_FORMAT);
		steadyPowerBox->setMinValue(0.);
		steadyPowerBox->setAlignment(TextBox::Alignment::Right);
		steadyPowerBox->setValueIncrement(1e3);
		steadyPowerBox->setSpinnable(true);
		autoLayout->setAnchor(steadyPowerBox->parent(), RelativeGridLayout::makeAnchor(1, 2, 1, 1, Alignment::Minimum, Alignment::Middle));

		keepCurrentPowerBox->setCallback([this](bool checked)
										 {
				steadyPowerBox->setEnabled(!checked);
				properties->steadyCurrentPower = checked;
				reactor->setKeepCurrentPower(checked); });

		steadyPowerBox->setCallback([this](double newValue)
									{
				if (newValue > 0.)
				{
					properties->steadyGoalPower = newValue;
					reactor->setAutomaticSteadyPower(newValue);
				} });

		avoidPeriodScramBox = makeSettingLabel<SliderCheckBox>(autoTab, "Avoid period SCRAM: ", 0);
		avoidPeriodScramBox->setChecked(reactor->getKeepCurrentPower());
		autoLayout->setAnchor(avoidPeriodScramBox->parent(), RelativeGridLayout::makeAnchor(1, 3, 1, 1, Alignment::Minimum, Alignment::Middle));

		avoidPeriodScramBox->setCallback([this](bool checked)
										 {
				properties->avoidPeriodScram = checked;
				reactor->setAutomaticAvoidPeriodScram(checked); });

		automaticMarginBox = makeSettingLabel<FloatBox<float>>(autoTab, "Maximum power deviation: ", 0, properties->steadyMargin * 100);
		automaticMarginBox->setUnits("%");
		automaticMarginBox->setFormat(SCI_NUMBER_FORMAT);
		automaticMarginBox->setMinMaxValues(0.1f, 10.f);
		automaticMarginBox->setAlignment(TextBox::Alignment::Right);
		automaticMarginBox->setValueIncrement(0.5f);
		automaticMarginBox->setSpinnable(true);
		autoLayout->setAnchor(automaticMarginBox->parent(), RelativeGridLayout::makeAnchor(1, 4, 1, 1, Alignment::Minimum, Alignment::Middle));

		automaticMarginBox->setCallback([this](float change)
										{
				properties->steadyMargin = change / 100;
				reactor->setAutomaticDeviation(change / 100); });

		modeTabs->setActiveTab(0);
	}

