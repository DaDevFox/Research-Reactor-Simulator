#include "../include/SimulatorGUI.h"

void SimulatorGUI::saveArchive(std::string path)
{
		properties->saveArchive(path);
		toggleBaseWindow(true);
	}

void SimulatorGUI::loadArchive(std::string path)
{
		properties->restoreArchive(path);
		toggleBaseWindow(true);
	}

void SimulatorGUI::loadScriptFromFile(std::string path)
{
		double time0 = reactor->getCurrentTime();
		std::ifstream ifs;
		if (path.length())
		{
			ifs.open(path);

			if (!ifs)
			{
				std::cerr << "Error opening input file: " << path << std::endl;
				return;
			}
			std::istream &is = static_cast<std::istream &>(ifs);

			Command cmd;

			while (is >> cmd)
			{
				cmd.timed += time0;
				cout << cmd;
				reactor->scriptCommands.push_back(cmd);
			}

			MessageDialog *msg = new MessageDialog(this, MessageDialog::Type::Warning, "Load script", "Loaded.");
			msg->setPosition(Vector2i((this->size().x() - msg->size().x()) / 2, (this->size().y() - msg->size().y()) / 2));
			msg->setCallback([this](int /*choice*/)
							 { toggleBaseWindow(true); });
		}
		else
		{
			MessageDialog *msg = new MessageDialog(this, MessageDialog::Type::Warning, "Load script", "Bad file name.");
			msg->setPosition(Vector2i((this->size().x() - msg->size().x()) / 2, (this->size().y() - msg->size().y()) / 2));
			msg->setCallback([this](int /*choice*/)
							 { toggleBaseWindow(true); });
		}
	}

void SimulatorGUI::updateSettings(bool updateReactor)
{
		curveFillBox->setChecked(properties->curveFill);
		curveFillBox->callback()(properties->curveFill);
		avoidPeriodScramBox->setChecked(properties->avoidPeriodScram);
		avoidPeriodScramBox->callback()(properties->avoidPeriodScram);
		for (int i = 0; i < 6; i++)
		{
			delayedGroupBoxes[i]->setValue(properties->betas[i]);
			delayedGroupBoxes[6 + i]->setValue(properties->lambdas[i]);
			delayedGroupsEnabledBoxes[i]->setChecked(properties->groupsEnabled[i]);
			delayedGroupsEnabledBoxes[i]->callback()(properties->groupsEnabled[i]);
		}
		coreVolumeBox->setValue(properties->coreVolume * 1e3); // Convert from m3 to L
		alpha0Box->setValue(properties->alpha0);
		alphaPeakBox->setValue(properties->alphaAtT1);
		alphaSlopeBox->setValue((float)properties->alphaK);
		tempPeakBox->setValue(properties->alphaT1);
		displayBox->setValue(properties->displayTime);
		excessReactivityBox->setValue(properties->excessReactivity);
		fissionProductsBox->setChecked(properties->fissionPoisons);
		fissionProductsBox->callback()(properties->fissionPoisons);
		graphSizeBox->setValue((int)(properties->graphSize * 100));
		sourceActivityBox->setValue(properties->neutronSourceActivity);
		neutronSourceCB->setChecked(properties->neutronSourceInserted);
		neutronSourceCB->callback()(properties->neutronSourceInserted);
		neutronSourceModeBox->setSelectedIndex(properties->ns_mode);
		neutronSourcePeriodBoxes[0]->setValue(properties->ns_squareWave.period);
		neutronSourcePeriodBoxes[1]->setValue(properties->ns_sineMode.period);
		neutronSourcePeriodBoxes[2]->setValue(properties->ns_sawToothMode.period);
		neutronSourceAmplitudeBoxes[0]->setValue(properties->ns_squareWave.amplitude);
		neutronSourceAmplitudeBoxes[1]->setValue(properties->ns_sineMode.amplitude);
		neutronSourceAmplitudeBoxes[2]->setValue(properties->ns_sawToothMode.amplitude);
		for (int i = 0; i < 4; i++)
			neutronSourceSQWBoxes[i]->setValue((int)(properties->ns_squareWave.xIndex[i] * 100));
		neutronSourceSINEModeBox->setSelectedIndex((int)properties->ns_sineMode.mode);
		for (int i = 0; i < 6; i++)
			neutronSourceSAWBoxes[i]->setValue((int)(properties->ns_sawToothMode.xIndex[i] * 100));
		periodLimBox->setValue((float)properties->periodLimit);
		powerLimBox->setValue(properties->powerLimit * 1e-03);
		fuel_tempLimBox->setValue(properties->tempLimit);
		water_tempLimBox->setValue(properties->waterTempLimit);
		// water_levelLimBox->setValue(properties->waterLevelLimit);
		for (int i = 0; i < 4; i++)
		{
			bool value = false;
			switch (i)
			{
			case 0:
				value = properties->periodScram;
				break;
			case 1:
				value = properties->powerScram;
				break;
			case 2:
				value = properties->tempScram;
				break;
			case 3:
				value = properties->waterTempScram;
				break;
			case 4:
				value = properties->waterLevelScram;
				break;
			}
			scramEnabledBoxes[i]->setChecked(value);
			scramEnabledBoxes[i]->callback()(value);
		}

		promptNeutronLifetimeBox->setValue(properties->promptNeutronLifetime);
		for (int i = 0; i < 2; i++)
		{
			reactivityLimitBox[i]->setValue(properties->reactivityGraphLimits[i]);
			temperatureLimitBox[i]->setValue(properties->temperatureGraphLimits[i]);
		}
		rodReactivityBox->setChecked(properties->rodReactivityPlot);
		rodReactivityBox->callback()(properties->rodReactivityPlot);
		for (int i = 0; i < 3; i++)
		{
			rodStepsBox[i]->setValue((int)properties->rodSettings[i].rodSteps);
			rodWorthBox[i]->setValue(properties->rodSettings[i].rodWorth);
			rodSpeedBox[i]->setValue(properties->rodSettings[i].rodSpeed);
			for (int j = 0; j < 2; j++)
			{
				rodCurveSliders[i * 2 + j]->setValue(properties->rodSettings[i].rodCurve[j]);
				rodCurveSliders[i * 2 + j]->finalCallback()(properties->rodSettings[i].rodCurve[j]);
				rodCurves[i]->setParameter(j * 2, properties->rodSettings[i].rodCurve[j]);
			}
		}
		periodBoxes[0]->setValue(properties->squareWave.period);
		amplitudeBoxes[0]->setValue(properties->squareWave.amplitude);
		for (int i = 0; i < 4; i++)
			squareWaveBoxes[i]->setValue((int)(properties->squareWave.xIndex[i] * 100));
		periodBoxes[1]->setValue(properties->sineMode.period);
		amplitudeBoxes[1]->setValue(properties->sineMode.amplitude);
		sineModeBox->setSelectedIndex(properties->sineMode.mode);
		periodBoxes[2]->setValue(properties->sawToothMode.period);
		amplitudeBoxes[2]->setValue(properties->sawToothMode.amplitude);
		for (int i = 0; i < 6; i++)
			sawToothBoxes[i]->setValue((int)(properties->sawToothMode.xIndex[i] * 100));
		keepCurrentPowerBox->setChecked(properties->steadyCurrentPower);
		keepCurrentPowerBox->callback()(properties->steadyCurrentPower);
		steadyPowerBox->setValue(properties->steadyGoalPower);
		automaticMarginBox->setValue(properties->steadyMargin * 100);
		tempEffectsBox->setChecked(properties->temperatureEffects);
		tempEffectsBox->callback()(properties->temperatureEffects);
		cooling->setChecked(properties->waterCooling);
		cooling->callback()(properties->waterCooling);
		coolingPowerBox->setValue(properties->waterCoolingPower);
		waterVolumeInput->setValue(properties->waterVolume);
		allRodsBox->setChecked(properties->allRodsAtOnce);
		allRodsBox->callback()(properties->allRodsAtOnce);
		logScaleBox->setChecked(properties->yAxisLog);
		logScaleBox->callback()(properties->yAxisLog);
		autoScramBox->setChecked(properties->automaticPulseScram);
		autoScramBox->callback()(properties->automaticPulseScram);
		hardcoreBox->setChecked(properties->reactivityHardcore);
		hardcoreBox->callback()(properties->reactivityHardcore);
		squareWaveSpeedBox->setChecked(properties->squareWaveUsesRodSpeed);
		squareWaveSpeedBox->callback()(properties->squareWaveUsesRodSpeed);

		if (updateReactor)
			reactor->setProperties(properties);
	}

