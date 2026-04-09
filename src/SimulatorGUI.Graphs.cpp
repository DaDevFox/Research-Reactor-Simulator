#include "../include/SimulatorGUI.h"

void SimulatorGUI::updatePulseTrack(bool updateData)
{
		if (!pulsePerformed)
			return;
		size_t startIdx, endIdx;
		startIdx = reactor->getIndexFromTime(reactor->time_[lastPulseData.pulseStartIndex] + pulseTimer->value(0) * 5);
		endIdx = reactor->getIndexFromTime(reactor->time_[lastPulseData.pulseStartIndex] + pulseTimer->value(1) * 5);

		double timeLimits[2] = {reactor->time_[startIdx], reactor->time_[endIdx]};

		for (int i = 0; i < 4; i++)
		{
			pulsePlots[i]->setPlotRange(startIdx, endIdx);
			if (i == 3)
			{
				pair<int, int> orders = recalculatePowerExtremes(timeLimits[0], timeLimits[1]);
				pulsePlots[i]->setLimits(timeLimits[0], timeLimits[1], 0., std::pow(10., orders.second));
				bool maxVisible = (lastPulseData.timeAtMax >= timeLimits[0]) || (lastPulseData.timeAtMax <= timeLimits[1]);
				if (maxVisible)
				{
					pulsePlots[i]->setHorizontalPointerPosition((float)((lastPulseData.timeAtMax - timeLimits[0]) / (timeLimits[1] - timeLimits[0])));
					pulsePlots[i]->setPointerPosition((float)(lastPulseData.peakPower / pulsePlots[i]->limits()[3]));
				}
				pulsePlots[i]->setHorizontalPointerShown(maxVisible);
				pulsePlots[i]->setPointerShown(maxVisible);
				pulsePlots[i]->setLimitOverride(0, pulseTimer->value(0) ? (to_string((int)(pulseTimer->value(0) * 5e3)) + "ms") : "pulse start");
				pulsePlots[i]->setLimitOverride(1, to_string((int)(pulseTimer->value(1) * 5e3)) + "ms");
			}
			else if (!(i % 2))
			{
				pulsePlots[i]->setLimits(timeLimits[0], timeLimits[1], std::floor(reactor->reactivity_[startIdx] / 200.) * 200, std::ceil(reactor->rodReactivity_[endIdx] / 200.) * 200);
			}
			else
			{
				pulsePlots[i]->setLimits(timeLimits[0], timeLimits[1], 0.f, updateData ? (std::ceil(lastPulseData.maxFuelTemp / 200.) * 200) : pulsePlots[i]->limits()[3]);
			}
		}

		const int per[2] = {(int)roundf(fmodf(pulseTimer->value(0) * 5, 1.f) * 100), (int)roundf(fmodf(pulseTimer->value(1) * 5, 1.f) * 100)};
		pulseDisplayLabels[0]->setCaption(std::to_string((int)(pulseTimer->value(0) * 5)));
		pulseDisplayLabels[1]->setCaption((per[0] < 10) ? "0" : "" + std::to_string(per[0]));
		pulseDisplayLabels[2]->setCaption(std::to_string((int)(pulseTimer->value(1) * 5)));
		pulseDisplayLabels[3]->setCaption((per[1] < 10) ? "0" : "" + std::to_string(per[1]));

		if (updateData)
		{
			pulseLabels[0]->setCaption(formatDecimalsDouble(lastPulseData.peakPower * 1e-6, 1) + " MW");
			pulseLabels[1]->setCaption(to_string((int)(lastPulseData.FWHM * 1e3)) + " ms");
			pulseLabels[2]->setCaption(formatDecimalsDouble(lastPulseData.maxFuelTemp, 1) + " " + degCelsiusUnit);
			pulseLabels[3]->setCaption(formatDecimalsDouble(lastPulseData.releasedEnergy * 1e-6, 1) + " MJ");
		}
	}

void SimulatorGUI::createDataDisplays(Widget *parent, RelativeGridLayout *rLayout)
{
		// Create the displays
		displayPanel1 = parent->add<CustomWidget>();
		displayPanel1->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill, 10, 5));
		rLayout->setAnchor(displayPanel1, RelativeGridLayout::makeAnchor(0, 0, 1, 1, Alignment::Fill, Alignment::Fill, RelativeGridLayout::FillMode::Always, RelativeGridLayout::FillMode::IfLess));
		displayPanel1->setBackgroundColor(Color(50, 255));
		displayPanel1->setDrawBackground(true);
		displayPanel2 = parent->add<CustomWidget>();
		displayPanel2->setLayout(new BoxLayout(Orientation::Vertical, Alignment::Fill, 10, 5));
		rLayout->setAnchor(displayPanel2, RelativeGridLayout::makeAnchor(1, 0, 1, 1, Alignment::Fill, Alignment::Fill, RelativeGridLayout::FillMode::Always, RelativeGridLayout::FillMode::IfLess));
		displayPanel2->setBackgroundColor(Color(50, 255));
		displayPanel2->setDrawBackground(true);
		powerShow = displayPanel1->add<DataDisplay<double>>("Power:");
		powerShow->setTextColor(Color(200, 255));
		powerShow->setData(reactor->getCurrentPower());
		powerShow->setFixedHeight(85);
		powerShow->setDisplayMode(DisplayMode::Scientific);
		powerShow->setUnit("W");
		reactivityShow = displayPanel2->add<DataDisplay<float>>("Reactivity (actual):");
		reactivityShow->setVisible(!properties->reactivityHardcore);
		reactivityShow->setTextColor(Color(200, 255));
		reactivityShow->setPointerColor(Color(0, 0, 255, 255));
		reactivityShow->setData(reactor->getCurrentReactivity());
		reactivityShow->setFixedHeight(85);
		reactivityShow->setDisplayMode(DisplayMode::FixedDecimalPlaces1);
		reactivityShow->setUnit("pcm");
		rodReactivityShow = displayPanel2->add<DataDisplay<float>>("Reactivity (inserted):");
		rodReactivityShow->setVisible(!properties->reactivityHardcore);
		rodReactivityShow->setTextColor(Color(200, 255));
		rodReactivityShow->setPointerColor(Color(180, 255));
		rodReactivityShow->setData(reactor->getCurrentRodReactivity());
		rodReactivityShow->setFixedHeight(85);
		rodReactivityShow->setDisplayMode(DisplayMode::FixedDecimalPlaces1);
		rodReactivityShow->setUnit("pcm");
		periodShow = displayPanel1->add<DataDisplay<double>>("Period:");
		periodShow->setTextColor(Color(200, 255));
		periodShow->setPointerColor(Color(0, 0, 0, 0));
		periodShow->setData(*reactor->getReactorPeriod());
		periodShow->setAbsoluteLimit(100.);
		periodShow->setFixedHeight(85);
		periodShow->setDisplayMode(DisplayMode::FixedDecimalPlaces1);
		periodShow->setUnit("s");
		periodShow->setBackgroundColor(Color(255, 255));
		temperatureShow = displayPanel1->add<DataDisplay<float>>("Fuel temperature:");
		temperatureShow->setTextColor(Color(200, 255));
		temperatureShow->setPointerColor(Color(0, 255, 0, 255));
		temperatureShow->setData(reactor->getCurrentTemperature());
		temperatureShow->setFixedHeight(85);
		temperatureShow->setDisplayMode(DisplayMode::FixedDecimalPlaces2);
		temperatureShow->setUnit(degCelsiusUnit);
		temperatureShow->setBackgroundColor(Color(255, 255));

		waterTemperatureShow = displayPanel2->add<DataDisplay<double>>("Water temperature:");
		waterTemperatureShow->setTextColor(Color(200, 255));
		waterTemperatureShow->setPointerColor(Color(0, 0, 0, 0));
		waterTemperatureShow->setData(*reactor->getWaterTemperature());
		waterTemperatureShow->setFixedHeight(85);
		waterTemperatureShow->setDisplayMode(DisplayMode::FixedDecimalPlaces2);
		waterTemperatureShow->setUnit(degCelsiusUnit);
		waterTemperatureShow->setBackgroundColor(Color(255, 255));

		waterLevelShow = displayPanel1->add<DataDisplay<double>>("Water level change:");
		waterLevelShow->setTextColor(Color(200, 255));
		waterLevelShow->setPointerColor(Color(0, 0, 0, 0));
		waterLevelShow->setData(*reactor->getWaterLevel() * 100.);
		waterLevelShow->setFixedHeight(85);
		waterLevelShow->setDisplayMode(DisplayMode::FixedDecimalPlaces2);
		waterLevelShow->setUnit("cm");
		waterLevelShow->setBackgroundColor(Color(255, 255));

		waterLevelShow->setVisible(false);
	}

void SimulatorGUI::initializeGraph()
{
		// Create a graph object
		canvas = baseWindow->add<CustomGraph>(4, "Main graph");
		relativeLayout->setAnchor(canvas, RelativeGridLayout::makeAnchor(0, 0));
		canvas->setBackgroundColor(Color(250, 255));
		canvas->setDrawBackground(true);
		canvas->setPadding(90.f, 25.f, properties->reactivityHardcore ? 120.f : 220.f, 50.f);

		// Create and save the plots
		rodReactivityPlot = canvas->addPlot(reactor->getDataLength(), true);
		temperaturePlot = canvas->addPlot(reactor->getDataLength(), true);
		reactivityPlot = canvas->addPlot(reactor->getDataLength(), true);
		powerPlot = canvas->addPlot(reactor->getDataLength(), true);

		// Styling
		canvas->setTextColor(Color(0, 255));
		reactivityPlot->setEnabled(!properties->reactivityHardcore);
		reactivityPlot->setName("Reactivity");
		reactivityPlot->setUnits("pcm");
		reactivityPlot->setColor(Color(0, 0, 255, 255));
		reactivityPlot->setFillColor(Color(0, 0, 255, 50));
		reactivityPlot->setPointerColor(Color(0, 0, 255, 255));
		reactivityPlot->setAxisShown(true);
		reactivityPlot->setMainLineShown(true);
		reactivityPlot->setTextShown(true);
		reactivityPlot->setNumberFormatMode(GraphElement::FormattingMode::Normal);
		reactivityPlot->setRoundFloating(true);
		reactivityPlot->setMajorTickNumber(7);
		reactivityPlot->setMinorTickNumber(2);
		reactivityPlot->setAxisPosition(GraphElement::AxisLocation::Right);
		reactivityPlot->setAxisOffset(110.f);
		reactivityPlot->setTextOffset(60.f);
		reactivityPlot->setFill(properties->curveFill);
		rodReactivityPlot->setEnabled(properties->rodReactivityPlot && !properties->reactivityHardcore);
		rodReactivityPlot->setName("Rod position");
		rodReactivityPlot->setColor(Color(200, 255));
		rodReactivityPlot->setFillColor(Color(0, 0, 255, 10));
		rodReactivityPlot->setPointerColor(Color(180, 255));
		rodReactivityPlot->setAxisPosition(GraphElement::AxisLocation::Right);
		rodReactivityPlot->setAxisOffset(110.f);
		rodReactivityPlot->setFill(properties->curveFill);
		powerPlot->setName("Power");
		powerPlot->setUnits("W");
		powerPlot->setColor(Color(255, 0, 0, 255));
		powerPlot->setFillColor(Color(255, 0, 0, 50));
		powerPlot->setAxisShown(true);
		powerPlot->setYlog(properties->yAxisLog);
		powerPlot->setAxisPosition(GraphElement::AxisLocation::Right);
		powerPlot->setTextOffset(60.f);
		powerPlot->setMajorTickNumber(4);
		powerPlot->setMinorTickNumber(4);
		powerPlot->setTextShown(true);
		powerPlot->setNumberFormatMode(GraphElement::FormattingMode::Exponential);
		powerPlot->setDrawMode(DrawMode::Smart);
		powerPlot->setHorizontalAxisShown(true);
		powerPlot->setHorizontalMinorTickNumber(4);
		powerPlot->setHorizontalName("Time");
		powerPlot->setHorizontalTextOffset(20.f);
		powerPlot->setLimitOverride(0, "30 seconds ago");
		powerPlot->setLimitOverride(1, "now");
		powerPlot->setFill(properties->curveFill);
		temperaturePlot->setName("Temperature");
		temperaturePlot->setUnits(degCelsiusUnit);
		temperaturePlot->setColor(Color(0, 255, 0, 255));
		temperaturePlot->setFillColor(Color(0, 255, 0, 50));
		temperaturePlot->setPointerColor(Color(0, 255, 0, 255));
		temperaturePlot->setAxisShown(true);
		temperaturePlot->setMainLineShown(true);
		temperaturePlot->setTextShown(true);
		temperaturePlot->setNumberFormatMode(GraphElement::FormattingMode::Normal);
		temperaturePlot->setMajorTickNumber(3);
		temperaturePlot->setMinorTickNumber(4);
		temperaturePlot->setFill(properties->curveFill);
		// Link plots to data
		reactivityPlot->setXdata(reactor->time_);
		reactivityPlot->setYdata(reactor->reactivity_);
		rodReactivityPlot->setXdata(reactor->time_);
		rodReactivityPlot->setYdata(reactor->rodReactivity_);
		powerPlot->setXdata(reactor->time_);
		powerPlot->setYdata(reactor->state_vector_[0]);
		powerPlot->setValueComputing([this](double *val, const size_t /*index*/)
									 { *val = reactor->powerFromNeutrons(*val); });
		temperaturePlot->setXdata(reactor->time_);
		temperaturePlot->setYdata(reactor->temperature_);
		// Link plots to display interval
		// for (size_t i = 0; i < canvas->graphNumber(); i++) {
		//	canvas->getPlot(i)->setPlotRange(displayInterval[0], displayInterval[1]);
		//}
	}

void SimulatorGUI::initializePulseGraph()
{
		pulseGraph->setBackgroundColor(Color(245, 255));
		pulseGraph->setTextColor(Color(16, 255));
		pulseGraph->setDrawBackground(true);
		pulseGraph->setPadding(90, 25, 220, 50);

		for (int i = 0; i < 4; i++)
		{
			pulsePlots[i] = pulseGraph->addPlot(reactor->getDataLength(), true);
			pulsePlots[i]->setXdata(reactor->time_);
			pulsePlots[i]->setNumberFormatMode((i < 3) ? GraphElement::FormattingMode::Normal : GraphElement::FormattingMode::Exponential);
			pulsePlots[i]->setDrawMode(DrawMode::Default);
			pulsePlots[i]->setAxisShown(i > 0);
			pulsePlots[i]->setTextShown(i > 0);
			pulsePlots[i]->setPointerShown(i > 0);
		}
		pulsePlots[3]->setName("Power");
		pulsePlots[3]->setUnits("W");
		pulsePlots[3]->setAxisPosition(GraphElement::AxisLocation::Right);
		pulsePlots[3]->setColor(Color(255, 0, 0, 255));
		pulsePlots[3]->setTextOffset(60.f);
		pulsePlots[3]->setMajorTickNumber(4);
		pulsePlots[3]->setMinorTickNumber(4);
		pulsePlots[3]->setPointerOverride(true);
		pulsePlots[3]->setHorizontalPointerColor(Color(64, 255));
		pulsePlots[3]->setHorizontalPointerShown(true);
		pulsePlots[3]->setHorizontalAxisShown(true);
		pulsePlots[3]->setHorizontalMinorTickNumber(4);
		pulsePlots[3]->setHorizontalName("Time");
		pulsePlots[3]->setHorizontalUnits("s");
		pulsePlots[3]->setHorizontalTextOffset(20.f);
		pulsePlots[3]->setYdata(reactor->state_vector_[0]);
		pulsePlots[3]->setValueComputing([this](double *val, const size_t /*index*/)
										 { *val = reactor->powerFromNeutrons(*val); }); // convert neutrons to watts

		pulsePlots[2]->setName("Reactivity");
		pulsePlots[2]->setUnits("pcm");
		pulsePlots[2]->setColor(Color(0, 0, 255, 255));
		pulsePlots[2]->setYdata(reactor->reactivity_);
		pulsePlots[2]->setAxisShown(true);
		pulsePlots[2]->setPointerColor(Color(0, 0, 255, 255));
		pulsePlots[2]->setMainLineShown(true);
		pulsePlots[2]->setRoundFloating(true);
		pulsePlots[2]->setMajorTickNumber(4);
		pulsePlots[2]->setMinorTickNumber(4);
		pulsePlots[2]->setAxisPosition(GraphElement::AxisLocation::Right);
		pulsePlots[2]->setAxisOffset(110.f);
		pulsePlots[2]->setTextOffset(60.f);

		pulsePlots[0]->setColor(Color(200, 255));
		pulsePlots[0]->setYdata(reactor->rodReactivity_);
		pulsePlots[0]->setAxisPosition(GraphElement::AxisLocation::Right);
		pulsePlots[0]->setAxisOffset(110.f);

		pulsePlots[1]->setColor(Color(0, 255, 0, 255));
		pulsePlots[1]->setYdata(reactor->temperature_);
		pulsePlots[1]->setName("Temperature");
		pulsePlots[1]->setUnits("C");
		pulsePlots[1]->setPointerColor(Color(0, 255, 0, 255));
		pulsePlots[1]->setAxisShown(true);
		pulsePlots[1]->setMainLineShown(true);
		pulsePlots[1]->setMajorTickNumber(4);
		pulsePlots[1]->setMinorTickNumber(4);
	}

