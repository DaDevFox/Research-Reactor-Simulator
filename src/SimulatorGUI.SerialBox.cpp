#include "../include/SimulatorGUI.h"

void SimulatorGUI::initializeSerial()
	{
		try
		{
			comPorts = getCOMports();
			lastCOMports = comPorts;
			lastBoxCheck = get_seconds_since_epoch();
			for (size_t i = 0; i < comPorts.size(); i++)
			{
				if (!boxConnected)
					tryConnectingTo(comPorts[i]);
			}
		}
		catch (exception e)
		{
			std::cout << "Exception while reading COM ports!" << std::endl;
		}
	}

void SimulatorGUI::updateCOMports()
	{
		if (boxConnected)
			return;

		double now = get_seconds_since_epoch();
		if (now > lastBoxCheck + 5.)
		{
			try
			{
				lastCOMports = comPorts;
				comPorts = getCOMports();
				if (comPorts.size() > lastCOMports.size())
				{
					string port;
					for (size_t i = 0; i < comPorts.size(); i++)
					{ // handle new ports
						port = comPorts[i];
						for (size_t j = 0; j < lastCOMports.size(); j++)
						{
							if (port == lastCOMports[j])
							{
								port = "";
								break;
							}
						}
						if (port.length() && !boxConnected)
							tryConnectingTo(port);
					}
				}
			}
			catch (exception e)
			{
				std::cout << "Exception while reading COM ports!" << std::endl;
			}
			lastBoxCheck = now;
		}
	}

void SimulatorGUI::tryConnectingTo(string port)
	{
		cout << "Connecting to " << port << "..." << endl;
		try
		{
			theBox = new Serial(port.c_str());
			;
		}
		catch (exception e)
		{
			std::cout << "Exception while astablishing connection with " << port << "!" << std::endl;
		}
		bool flag = false;
		if (theBox->IsConnected())
		{
			if (theBox->availableBytes() >= 11)
			{
				char buffer[11];
				theBox->ReadData(buffer, 11);
				flag = true;
				for (int i = 0; i < 11; i++)
				{
					if (buffer[i] != box_auth[i])
					{
						flag = false;
						break;
					}
				}
			}
		}
		if (flag)
		{
			// Successfull authentication
			std::cout << "Box found on " << theBox->GetName() << "!" << std::endl;
		}
		else
		{
			theBox->~Serial();
		}
		boxConnected = flag;
	}

void SimulatorGUI::handleBox()
{
		LEDstatus = (uint16_t)0;
		// Write LED status
		int scramS = reactor->getScramStatus();
		if (Simulator::ScramSignals::Period & scramS)
			LEDstatus |= SCRAM_PER;
		if (Simulator::ScramSignals::FuelTemperature & scramS)
			LEDstatus |= SCRAM_FT;
		if (Simulator::ScramSignals::WaterTemperature & scramS)
			LEDstatus |= SCRAM_WT;
		if (Simulator::ScramSignals::Power & scramS)
			LEDstatus |= SCRAM_POW;
		if (Simulator::ScramSignals::User & scramS)
			LEDstatus |= SCRAM_MAN;

		if (reactor->safetyRod()->isEnabled())
		{
			LEDstatus |= ROD_SAFETY_ENBL;
		}
		if (reactor->regulatingRod()->isEnabled())
		{
			LEDstatus |= ROD_REG_ENBL;
		}
		if (reactor->shimRod()->isEnabled())
		{
			LEDstatus |= ROD_SHIM_ENBL;
		}

		if (reactor->safetyRod()->getCommandType() == ControlRod::CommandType::Top || *reactor->safetyRod()->getExactPosition() == (float)*reactor->safetyRod()->getRodSteps())
		{
			LEDstatus |= ROD_SAFETY_UP;
		}
		if (reactor->regulatingRod()->getCommandType() == ControlRod::CommandType::Top || *reactor->regulatingRod()->getExactPosition() == (float)*reactor->regulatingRod()->getRodSteps())
		{
			LEDstatus |= ROD_REG_UP;
		}
		if (reactor->shimRod()->getCommandType() == ControlRod::CommandType::Top || *reactor->shimRod()->getExactPosition() == (float)*reactor->shimRod()->getRodSteps())
		{
			LEDstatus |= ROD_SHIM_UP;
		}
		if (reactor->safetyRod()->getCommandType() == ControlRod::CommandType::Bottom || *reactor->safetyRod()->getExactPosition() == 0.f)
		{
			LEDstatus |= ROD_SAFETY_DOWN;
		}
		if (reactor->regulatingRod()->getCommandType() == ControlRod::CommandType::Bottom || *reactor->regulatingRod()->getExactPosition() == 0.f)
		{
			LEDstatus |= ROD_REG_DOWN;
		}
		if (reactor->shimRod()->getCommandType() == ControlRod::CommandType::Bottom || *reactor->shimRod()->getExactPosition() == 0.f)
		{
			LEDstatus |= ROD_SHIM_DOWN;
		}
		if (reactor->regulatingRod()->getOperationMode() == ControlRod::OperationModes::Pulse && reactor->getScramStatus() == 0)
		{
			LEDstatus |= FIRE_LED_B;
		}

		// Convert LED status to two bytes
		char sendByte[3];
		sendByte[0] = 77;
		sendByte[1] = LEDstatus >> 8;
		sendByte[2] = LEDstatus & 0x00ff;

		// Write LED data

		theBox->WriteData(sendByte, 3);

		// Reset sounds
		// LEDstatus &= (1 << 13) - 1;

		// Read data
		uint16_t box_data = 0;
		bool gotData = false;
		char buffer[2];
		double time_now = get_seconds_since_epoch();
		while (theBox->availableBytes() >= 2)
		{
			theBox->ReadData(buffer, 2);
			box_data = (unsigned char)buffer[1];
			box_data += ((uint16_t)buffer[0]) << 8;
			gotData = true;
			handleBoxData(box_data, time_now);
		}
		if (!gotData)
		{ // Disconnect box if no data is recieved in 1 second
			if (lastData == 0.)
			{
				lastData = time_now;
			}
			else if (time_now > lastData + 1.)
			{
				boxConnected = false;
				theBox->~Serial();
				std::cout << "Box disconnected! (timeout)" << std::endl;
				lastData = 0.;
			}
			return;
		}
	}

void SimulatorGUI::handleBoxData(uint16_t box_data, double now)
{
		lastData = now;
		bool rodsMoving[NUMBER_OF_CONTROL_RODS];
		for (int i = 0; i < NUMBER_OF_CONTROL_RODS; i++)
			rodsMoving[i] = (reactor->rods[i]->getCommandType() == ControlRod::CommandType::None);
		if (box_data & SCRAM_BTN)
		{
			if (!btns[0])
				reactor->scram(Simulator::ScramSignals::User);
		}
		if (box_data & FIRE_BTN)
		{
			if (!btns[1])
			{
				if (reactor->getScramStatus() == 0)
					reactor->beginPulse();
			}
		}
		if (box_data & ENABLE_SAFETY_BTN)
		{
			if (!btns[2])
			{
				if (reactor->getScramStatus() == 0)
					reactor->safetyRod()->setEnabled(!reactor->safetyRod()->isEnabled());
			}
		}
		if (box_data & UP_SAFETY_BTN)
		{
			if (!btns[3] && ((rodsMoving[1] && rodsMoving[2]) || properties->allRodsAtOnce))
				reactor->safetyRod()->commandToTop();
		}
		else
		{
			if (btns[3])
				reactor->safetyRod()->clearCommands(ControlRod::CommandType::Top);
		}
		if (box_data & DOWN_SAFETY_BTN)
		{
			if (!btns[4] && ((rodsMoving[1] && rodsMoving[2]) || properties->allRodsAtOnce))
				reactor->safetyRod()->commandToBottom();
		}
		else
		{
			if (btns[4])
				reactor->safetyRod()->clearCommands(ControlRod::CommandType::Bottom);
		}
		if (box_data & ENABLE_REG_BTN)
		{
			if (!btns[5])
			{
				if (reactor->getScramStatus() == 0)
					reactor->regulatingRod()->setEnabled(!reactor->regulatingRod()->isEnabled());
			}
		}
		if (box_data & UP_REG_BTN)
		{
			if (!btns[6] && ((rodsMoving[0] && rodsMoving[2]) || properties->allRodsAtOnce))
				reactor->regulatingRod()->commandToTop();
		}
		else
		{
			if (btns[6])
				reactor->regulatingRod()->clearCommands(ControlRod::CommandType::Top);
		}
		if (box_data & DOWN_REG_BTN)
		{
			if (!btns[7] && ((rodsMoving[0] && rodsMoving[2]) || properties->allRodsAtOnce))
				reactor->regulatingRod()->commandToBottom();
		}
		else
		{
			if (btns[7])
				reactor->regulatingRod()->clearCommands(ControlRod::CommandType::Bottom);
		}
		if (box_data & ENABLE_SHIM_BTN)
		{
			if (!btns[8])
			{
				if (reactor->getScramStatus() == 0)
					reactor->shimRod()->setEnabled(!reactor->shimRod()->isEnabled());
			}
		}
		if (box_data & UP_SHIM_BTN)
		{
			if (!btns[9] && ((rodsMoving[0] && rodsMoving[1]) || properties->allRodsAtOnce))
				reactor->shimRod()->commandToTop();
		}
		else
		{
			if (btns[9])
				reactor->shimRod()->clearCommands(ControlRod::CommandType::Top);
		}
		if (box_data & DOWN_SHIM_BTN)
		{
			if (!btns[10] && ((rodsMoving[0] && rodsMoving[1]) || properties->allRodsAtOnce))
				reactor->shimRod()->commandToBottom();
		}
		else
		{
			if (btns[10])
				reactor->shimRod()->clearCommands(ControlRod::CommandType::Bottom);
		}
		btns[0] = (box_data & SCRAM_BTN) != 0;
		btns[1] = (box_data & FIRE_BTN) != 0;
		btns[2] = (box_data & ENABLE_SAFETY_BTN) != 0;
		btns[3] = (box_data & UP_SAFETY_BTN) != 0;
		btns[4] = (box_data & DOWN_SAFETY_BTN) != 0;
		btns[5] = (box_data & ENABLE_REG_BTN) != 0;
		btns[6] = (box_data & UP_REG_BTN) != 0;
		btns[7] = (box_data & DOWN_REG_BTN) != 0;
		btns[8] = (box_data & ENABLE_SHIM_BTN) != 0;
		btns[9] = (box_data & UP_SHIM_BTN) != 0;
		btns[10] = (box_data & DOWN_SHIM_BTN) != 0;

		int mode = box_data & 7;
		if (mode != lastModeState)
		{
			rodMode->setSelectedIndex(mode);
			lastModeState = mode;
		}
	}

std::vector<string> SimulatorGUI::getCOMports()
	{
#ifdef _WIN32
		TCHAR *ptr = new TCHAR[65535];
		TCHAR *temp_ptr;
		unsigned long dwChars = QueryDosDevice(NULL, ptr, 65535);
		std::vector<string> comPorts_ = std::vector<string>();
		while (dwChars)
		{
			int port;
			if (sscanf(ptr, "COM%d", &port) == 1)
			{
				comPorts_.push_back("COM" + std::to_string(port));
			}
			temp_ptr = strchr(ptr, 0);
			dwChars -= (DWORD)((temp_ptr - ptr) / sizeof(TCHAR) + 1);
			ptr = temp_ptr + 1;
		}
		return comPorts_;
#else
		return std::vector<string>();
#endif
	}

