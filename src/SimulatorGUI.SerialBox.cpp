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

