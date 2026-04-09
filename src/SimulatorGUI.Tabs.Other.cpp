#include "../include/SimulatorGUI.h"

void SimulatorGUI::createOtherTab()
{
		CustomWidget *other_tab = tabControl->createTab("Other");
		other_tab->setId("other tab");
		RelativeGridLayout *rel = new RelativeGridLayout();
		rel->appendCol(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 0 left border
		rel->appendCol(RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed)); // 1 save button
		rel->appendCol(RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed));  // 2 border
		rel->appendCol(RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed)); // 3 load button
		rel->appendCol(RelativeGridLayout::Size(10.f, RelativeGridLayout::SizeType::Fixed));  // 2 border
		rel->appendCol(RelativeGridLayout::Size(120.f, RelativeGridLayout::SizeType::Fixed)); // 5 division thing
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 0 top border
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 1 Load and save settings
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 2 Seperating space
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 3 Load and save log
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 4 Seperating space
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 5 Rod Curves
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 6 Seperating space
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 7 Reset simulator
		rel->appendRow(RelativeGridLayout::Size(15.f, RelativeGridLayout::SizeType::Fixed));  // 8 Seperating space
		rel->appendRow(RelativeGridLayout::Size(30.f, RelativeGridLayout::SizeType::Fixed));  // 9 Load script

		other_tab->setLayout(rel);

		Button *saveBtn = other_tab->add<Button>("Save settings");
		rel->setAnchor(saveBtn, RelativeGridLayout::makeAnchor(1, 1));
		saveBtn->setCallback([this]()
							 {
				toggleBaseWindow(false);
				std::string saveFileName = file_dialog({ { "rrs", "Simulator settings file" } }, true);
				if (saveFileName.substr((size_t)std::max(0, (int)saveFileName.length() - 4), std::min((size_t)4, saveFileName.length())) != ".rrs")
				{
					saveFileName = saveFileName.append(".rrs");
				}
				//saveSettings(saveFileName);
				saveArchive(saveFileName); });

		Button *loadBtn = other_tab->add<Button>("Load settings");
		rel->setAnchor(loadBtn, RelativeGridLayout::makeAnchor(3, 1));
		loadBtn->setCallback([this]()
							 {
				toggleBaseWindow(false);
				std::string loadFileName = file_dialog({ { "rrs", "Simulator settings file" } }, false);
				// loadSettingsFromFile(loadFileName);
				loadArchive(loadFileName);
				std::cout << "Graph size after GUI: " << properties->graphSize << std::endl;
				updateSettings(); });

		Button *saveLogBtn = other_tab->add<Button>("Save data");
		rel->setAnchor(saveLogBtn, RelativeGridLayout::makeAnchor(1, 3));
		saveLogBtn->setCallback([this]()
								{
				std::string logFileName = file_dialog(
					{ { "dat", "Data file" },{ "txt", "Text file" } }, true);
				reactor->dataToFile(logFileName); });

		CustomLabel *divisionLabel = other_tab->add<CustomLabel>("Save each steps:");
		rel->setAnchor(divisionLabel, RelativeGridLayout::makeAnchor(3, 3));

		IntBox<int> *divisionBox = other_tab->add<IntBox<int>>(reactor->data_division);
		rel->setAnchor(divisionBox, RelativeGridLayout::makeAnchor(5, 3));
		// divisionBox->setUnits("%");
		divisionBox->setDefaultValue(to_string(reactor->data_division));
		divisionBox->setFontSize(16);
		divisionBox->setFormat("[0-9]+");
		divisionBox->setSpinnable(true);
		divisionBox->setMinValue(1);
		divisionBox->setMaxValue(100);
		divisionBox->setValueIncrement(1);
		divisionBox->setCallback([this](int a)
								 { reactor->data_division = a; });

		Button *saveRodCurves = other_tab->add<Button>("Rod curves");
		rel->setAnchor(saveRodCurves, RelativeGridLayout::makeAnchor(1, 5));
		saveRodCurves->setCallback([this]()
								   {
				std::string logFileName = file_dialog(
					{ { "dat", "Data file" },{ "txt", "Text file" } }, true);
				reactor->rodsToFile(logFileName); });

		Button *loadScriptBtn = other_tab->add<Button>("Load script");
		rel->setAnchor(loadScriptBtn, RelativeGridLayout::makeAnchor(1, 9));
		loadScriptBtn->setCallback([this]()
								   {
				toggleBaseWindow(false);
				std::string loadFileName = file_dialog({ { "rrs", "Simulator Script file" } }, false);
				loadScriptFromFile(loadFileName); });

		Button *resetBtn = other_tab->add<Button>("Reset simulator");
		rel->setAnchor(resetBtn, RelativeGridLayout::makeAnchor(1, 7));
		resetBtn->setCallback([this]()
							  { this->resetSimToStart(); });
	}

