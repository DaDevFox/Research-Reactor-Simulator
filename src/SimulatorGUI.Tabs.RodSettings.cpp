#include "../include/SimulatorGUI.h"

void SimulatorGUI::createRodSettingsTab()
{
		Widget *rod_settings = tabControl->createTab("Control rods");
		rod_settings->setId("Rod settings tab");
		RelativeGridLayout *rod_settings_layout = new RelativeGridLayout(); /* COLUMNS */
		for (int i = 0; i < 9; i++)
		{
			if (i == 3 || i == 6)
				rod_settings_layout->appendCol(RelativeGridLayout::Size(2.f, RelativeGridLayout::SizeType::Fixed));
			rod_settings_layout->appendCol(1.f);
		}
		/* ROWS */
		rod_settings_layout->appendRow(RelativeGridLayout::Size(35.f, RelativeGridLayout::SizeType::Fixed));  // 0 title
		rod_settings_layout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 1 empty space
		rod_settings_layout->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));  // 2 setting name
		rod_settings_layout->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));  // 3 setting value
		rod_settings_layout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 4 top slider
		rod_settings_layout->appendRow(RelativeGridLayout::Size(320.f, RelativeGridLayout::SizeType::Fixed)); // 5 graph
		rod_settings_layout->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 6 bottom slider
		rod_settings_layout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 7 empty space
		rod_settings_layout->appendRow(RelativeGridLayout::Size(1.f, RelativeGridLayout::SizeType::Fixed));	  // 8 border
		rod_settings_layout->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 9 empty space
		rod_settings_layout->appendRow(RelativeGridLayout::Size(20.f, RelativeGridLayout::SizeType::Fixed));  // 10 label
		rod_settings_layout->appendRow(RelativeGridLayout::Size(320.f, RelativeGridLayout::SizeType::Fixed)); // 11 derivative graph
		rod_settings->setLayout(rod_settings_layout);

		// Create borders
		for (int i = 0; i < 2; i++)
		{
			CustomWidget *border = rod_settings->add<CustomWidget>();
			rod_settings_layout->setAnchor(border, RelativeGridLayout::makeAnchor(i ? 3 : 7, 0, 1, 13));
			border->setBackgroundColor(coolBlue);
			border->setDrawBackground(true);
		}
		CustomWidget *border = rod_settings->add<CustomWidget>();
		rod_settings_layout->setAnchor(border, RelativeGridLayout::makeAnchor(0, 8, 11, 1));
		border->setBackgroundColor(coolBlue);
		border->setDrawBackground(true);

		Label *tempLabel;
		ControlRod *useRod;
		// for each rod
		for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
		{
			useRod = reactor->rods[i];

			// Titles
			tempLabel = rod_settings->add<Label>(useRod->getRodName() + " rod", "sans-bold");
			tempLabel->setFontSize(30);
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4, 0, 3, 1, Alignment::Middle, Alignment::Maximum));

			// Rod steps setting
			tempLabel = rod_settings->add<Label>("Rod steps:", "sans-bold");
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4, 2, 1, 1, Alignment::Middle, Alignment::Middle));
			rodStepsBox[i] = rod_settings->add<IntBox<int>>((int)properties->rodSettings[i].rodSteps);
			rod_settings_layout->setAnchor(rodStepsBox[i], RelativeGridLayout::makeAnchor(i * 4, 3, 1, 1, Alignment::Middle));
			rodStepsBox[i]->setFixedSize(Vector2i(100, 20));
			rodStepsBox[i]->setAlignment(TextBox::Alignment::Left);
			rodStepsBox[i]->setUnits("steps");
			rodStepsBox[i]->setDefaultValue(to_string(properties->rodSettings[i].rodSteps));
			rodStepsBox[i]->setMinMaxValues(1, 100000);
			rodStepsBox[i]->setValueIncrement(10);
			rodStepsBox[i]->setFormat("[0-9]+");
			rodStepsBox[i]->setCallback([useRod, i, this](int change)
										{
					properties->rodSettings[i].rodSteps = change;
					useRod->setRodSteps((size_t)change);
					rodCurves[i]->setLimitHorizontalMultiplier((double)*useRod->getRodSteps()); });

			// Rod worth setting
			tempLabel = rod_settings->add<Label>("Rod worth:", "sans-bold");
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4 + 1, 2, 1, 1, Alignment::Middle, Alignment::Middle));
			rodWorthBox[i] = rod_settings->add<FloatBox<float>>(properties->rodSettings[i].rodWorth);
			rod_settings_layout->setAnchor(rodWorthBox[i], RelativeGridLayout::makeAnchor(i * 4 + 1, 3, 1, 1, Alignment::Middle));
			rodWorthBox[i]->setFixedSize(Vector2i(100, 20));
			rodWorthBox[i]->setAlignment(TextBox::Alignment::Left);
			rodWorthBox[i]->setUnits("pcm");
			rodWorthBox[i]->setDefaultValue(to_string(properties->rodSettings[i].rodWorth));
			rodWorthBox[i]->setMinValue(1.f);
			rodWorthBox[i]->setValueIncrement(10.f);
			rodWorthBox[i]->setFormat("[0-9]*\\.?[0-9]+");
			rodWorthBox[i]->setCallback([useRod, i, this](float change)
										{
					properties->rodSettings[i].rodWorth = change;
					useRod->setRodWorth(change); });

			// Rod speed setting
			tempLabel = rod_settings->add<Label>("Rod speed:", "sans-bold");
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4 + 2, 2, 1, 1, Alignment::Middle, Alignment::Middle));
			rodSpeedBox[i] = rod_settings->add<FloatBox<float>>(properties->rodSettings[i].rodSpeed);
			rod_settings_layout->setAnchor(rodSpeedBox[i], RelativeGridLayout::makeAnchor(i * 4 + 2, 3, 1, 1, Alignment::Middle));
			rodSpeedBox[i]->setFixedSize(Vector2i(100, 20));
			rodSpeedBox[i]->setAlignment(TextBox::Alignment::Left);
			rodSpeedBox[i]->setUnits("steps/s");
			rodSpeedBox[i]->setDefaultValue(to_string(properties->rodSettings[i].rodSpeed));
			rodSpeedBox[i]->setMinValue(0.f);
			rodSpeedBox[i]->setValueIncrement(1.f);
			rodSpeedBox[i]->setFormat("[0-9]*\\.?[0-9]+");
			rodSpeedBox[i]->setCallback([useRod, i, this](float change)
										{
											properties->rodSettings[i].rodSpeed = change;
											useRod->setRodSpeed(change);
											if (i == 1)
												reactor->regulatingRod()->sine()->fillXYaxis(operationModesPlots[0][1], operationModesPlots[1][1]); // Update SQW graph
										});

			// Display tool
			CustomGraph *rodControl = rod_settings->add<CustomGraph>(1);
			rod_settings_layout->setAnchor(rodControl, RelativeGridLayout::makeAnchor(i * 4, 5, 3, 1, Alignment::Middle));
			rodControl->setFixedWidth(320);
			rodControl->setBackgroundColor(Color(60, 255));
			rodControl->setDrawBackground(true);
			rodControl->setPadding(80.f, 25.f, 30.f, 70.f);
			rodControl->setTextColor(Color(250, 255));
			rodCurves[i] = rodControl->addBezierCurve();
			rodCurves[i]->setName("Reactivity");
			rodCurves[i]->setHorizontalName("Position");
			rodCurves[i]->setTextColor(Color(250, 255));
			rodCurves[i]->setTextOffset(30.f);
			rodCurves[i]->setColor(Color(0, 120, 255, 255));
			rodCurves[i]->setPointerColor(Color(0, 120, 255, 255));
			rodCurves[i]->setPointerOverride(true);
			rodCurves[i]->setFill(true);
			rodCurves[i]->setFillColor(Color(0, 120, 255, 150));
			rodCurves[i]->setAxisShown(true);
			rodCurves[i]->setMainLineShown(true);
			rodCurves[i]->setAxisColor(Color(250, 255));
			rodCurves[i]->setMajorTickNumber(3);
			rodCurves[i]->setMinorTickNumber(1);
			rodCurves[i]->setTextShown(true);
			rodCurves[i]->setMainTickFontSize(22.f);
			rodCurves[i]->setMajorTickFontSize(20.f);
			rodCurves[i]->setLimitMultiplier(100.);
			rodCurves[i]->setUnits("%");
			rodCurves[i]->setHorizontalPointerColor(Color(120, 0, 255, 255));
			rodCurves[i]->setHorizontalAxisShown(true);
			rodCurves[i]->setHorizontalMainLineShown(true);
			rodCurves[i]->setHorizontalPointerShown(true);
			rodCurves[i]->setHorizontalMajorTickNumber(1);
			rodCurves[i]->setHorizontalMinorTickNumber(1);
			rodCurves[i]->setLimitHorizontalMultiplier((double)*useRod->getRodSteps());
			rodCurves[i]->setHorizontalUnits("steps");

			// Sliders
			for (int j = 0; j < 2; j++)
			{
				rodCurves[i]->setParameter(j * 2, properties->rodSettings[i].rodCurve[j]);
				size_t sliderIndex = i * 2 + j;
				rodCurveSliders[sliderIndex] = rod_settings->add<Slider>();
				rod_settings_layout->setAnchor(rodCurveSliders[sliderIndex], RelativeGridLayout::makeAnchor(i * 4, 4 + j * 2, 3, 1, Alignment::Middle, Alignment::Middle));
				rodCurveSliders[sliderIndex]->setFixedSize(Vector2i(340, 15));
				rodCurveSliders[sliderIndex]->setValue(properties->rodSettings[i].rodCurve[j]);
				rodCurveSliders[sliderIndex]->setCallback([this, i, j](float change)
														  { rodCurves[i]->setParameter(j * 2, change); });
				rodCurveSliders[sliderIndex]->setFinalCallback([useRod, j, i, this](float stop)
															   {
						rodCurves[i]->setParameter(j * 2, stop);
						properties->rodSettings[i].rodCurve[j] = stop;
						useRod->setParameter(j, stop);
						handleDerivativeChange(); });
			}

			// Derivative graphs
			tempLabel = rod_settings->add<Label>("Differential\nrod worth curve", "sans-bold", 25);
			rod_settings_layout->setAnchor(tempLabel, RelativeGridLayout::makeAnchor(i * 4, 10, 3, 1, Alignment::Middle, Alignment::Maximum));

			CustomGraph *rodDerDisplay = rod_settings->add<CustomGraph>(1);
			rod_settings_layout->setAnchor(rodDerDisplay, RelativeGridLayout::makeAnchor(i * 4, 11, 3, 1, Alignment::Middle, Alignment::Minimum));
			rodDerDisplay->setFixedSize(Vector2i(320, 320));
			rodDerDisplay->setBackgroundColor(Color(60, 255));
			rodDerDisplay->setDrawBackground(true);
			rodDerDisplay->setPadding(80.f, 25.f, 30.f, 70.f);
			rodDerDisplay->setTextColor(Color(250, 255));
			rodDerivatives[i] = rodDerDisplay->addPlot(ControlRod::dataPoints, false);
			rodDerivatives[i]->setName("Reactivity change");
			rodDerivatives[i]->setHorizontalName("Position");
			rodDerivatives[i]->setTextColor(Color(250, 255));
			rodDerivatives[i]->setTextOffset(30.f);
			rodDerivatives[i]->setColor(Color(0, 120, 255, 255));
			rodDerivatives[i]->setPointerColor(Color(0, 120, 255, 255));
			rodDerivatives[i]->setPointerOverride(true);
			rodDerivatives[i]->setPointerShown(true);
			rodDerivatives[i]->setHorizontalPointerShown(true);
			rodDerivatives[i]->setFill(true);
			rodDerivatives[i]->setFillColor(Color(0, 120, 255, 150));
			rodDerivatives[i]->setAxisShown(true);
			rodDerivatives[i]->setMainLineShown(true);
			rodDerivatives[i]->setAxisColor(Color(250, 255));
			rodDerivatives[i]->setMajorTickNumber(3);
			rodDerivatives[i]->setMinorTickNumber(1);
			rodDerivatives[i]->setTextShown(true);
			rodDerivatives[i]->setMainTickFontSize(22.f);
			rodDerivatives[i]->setMajorTickFontSize(20.f);
			rodDerivatives[i]->setUnits("pcm/step");
			rodDerivatives[i]->setHorizontalPointerColor(Color(120, 0, 255, 255));
			rodDerivatives[i]->setHorizontalAxisShown(true);
			rodDerivatives[i]->setHorizontalMainLineShown(true);
			rodDerivatives[i]->setHorizontalPointerShown(true);
			rodDerivatives[i]->setHorizontalMajorTickNumber(1);
			rodDerivatives[i]->setHorizontalMinorTickNumber(1);
			rodDerivatives[i]->setLimitHorizontalMultiplier((double)*useRod->getRodSteps());
			rodDerivatives[i]->setHorizontalUnits("steps");
			rodDerivatives[i]->setXdataLin(0L);
			rodDerivatives[i]->setPlotRange(0, *useRod->getRodSteps());
			rodDerivatives[i]->setValueComputing([this, i](double *v, size_t /*in*/)
												 { *v *= reactor->rods[i]->getRodWorth(); });
		}
		handleDerivativeChange();
	}

