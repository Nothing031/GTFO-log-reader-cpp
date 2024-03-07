#include <iostream>
#include <ShlObj.h>
#include <Windows.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

int main(void)
{
	// get gtfo log folder path
	wchar_t* localAppData = nullptr;
	SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &localAppData);
	// wchar_t to string
	std::wstring ws(localAppData);
	std::string path(ws.begin(), ws.end());
	CoTaskMemFree(localAppData);
	path+= "Low\\10 Chambers Collective\\GTFO\\";

	std::cout << "Press enter to stop refresh" << std::endl;
	std::cout << "Start in 3 seconds" << std::endl;
	Sleep(3000);

label:
	while (1) {

		// get netstatus file
		std::filesystem::path fileList[100];
		size_t pos;
		std::string logPath = path;

		if (fs::exists(path) && fs::is_directory(path)) {
			int i = 0;
			for (const auto& entry : fs::directory_iterator(path)) {
				pos = entry.path().filename().string().find("_NICKNAME_NETSTATUS.");
				if (pos != std::string::npos) {
					fileList[i] = entry.path().filename();
					i++;
				}
			}
			for (int i = sizeof(fileList) / sizeof(fileList[0]) - 1; i > -1; i--) {
				if (fileList[i] != "") {
					logPath += fileList[i].string();
					break;
				}
				else if (i == 0) {
					std::cout << "Error : could not find game log" << std::endl;
					Sleep(1000);
					goto label;
				}
			}
		}

		// read data from log file
		std::ifstream inputFile(logPath);
		std::string fline;
		std::vector<std::string> data;

		while (std::getline(inputFile, fline)) {
			data.push_back(fline);
		}
		inputFile.close();

		// get last drop index
		int lastDropIndex = 0;
		for (int i = 0; i < data.size(); i++) {
			pos = data[i].find("SelectActiveExpedition :");
			if (pos != std::string::npos) {
				lastDropIndex = i;
			}
		}
		if (lastDropIndex == 0) {
			std::cout << "Error : could not find drop point" << std::endl;
			Sleep(1000);
			goto label;
		}

		// search every key
		std::string everyKeyName[32];
		std::string everyKeyRi[32];
		std::string everyKeyZone[32];

		int k = 0;
		for (int i = lastDropIndex; i < data.size(); i++) {
			pos = data[i].find("CreateKeyItemDistribution");
			if (pos != std::string::npos) {
				data[i].erase(0, data[i].find("PublicName: ") + 12)
					.erase(data[i].find(" SpawnedItem:"), data[i].length());
				everyKeyName[k] = data[i];
				for (int j = i; j < data.size(); j++) {
					pos = data[j].find("ri: ");
					if (pos != std::string::npos) {
						std::string riString = data[j]
							.substr(data[j].length() - 53, data[j].length());
						std::string zoneString = data[j]
							.substr(83, data[j].length() - 137);

						everyKeyRi[k] = riString
							.erase(0, riString.find("ri: ") + 4)
							.erase(riString.find(" had"), riString.length());

						everyKeyZone[k] = zoneString
							.erase(0, zoneString.find("zone: ZONE") + 10)
							.erase(zoneString.find(" function:"), zoneString.length());
						k++;
						break;
					}
				}
			}
		}

		//categorize key
		std::string BulkheadKeyName[16];
		std::string BulkheadKeyRi[16];
		std::string BulkheadKeyZone[16];

		std::string ColorKeyName[16];
		std::string ColorKeyRi[16];
		std::string ColorKeyZone[16];

		int bulkheadNum = 0;
		int colorNum = 0;
		for (int i = 0; i < sizeof(everyKeyRi) / sizeof(everyKeyRi[0]); i++) {
			pos = everyKeyName[i].find("BULKHEAD");
			if (pos != std::string::npos && !everyKeyName[i].empty()) {
				BulkheadKeyName[bulkheadNum] = everyKeyName[i];
				BulkheadKeyRi[bulkheadNum] = everyKeyRi[i];
				BulkheadKeyZone[bulkheadNum] = everyKeyZone[i];
				bulkheadNum++;
			}
			else if (!everyKeyName[i].empty()) {
				ColorKeyName[colorNum] = everyKeyName[i];
				ColorKeyRi[colorNum] = everyKeyRi[i];
				ColorKeyZone[colorNum] = everyKeyZone[i];
				colorNum++;
			}
		}

		// print

		std::cout << "\nBULKHEAD KEY" << std::endl;
		if (!BulkheadKeyName[0].empty()) {
			for (int i = 0; i < sizeof(BulkheadKeyRi) / sizeof(BulkheadKeyRi[0]); i++) {
				if (BulkheadKeyName[i].empty())
					break;
				std::cout << "    Name : " << BulkheadKeyName[i] << "\t    Ri : " << BulkheadKeyRi[i] << "\t    Zone : " << BulkheadKeyZone[i] << std::endl;
			}
		}

		std::cout << "\nCOLOR KEY" << std::endl;
		if (!ColorKeyName[0].empty()) {
			for (int i = 0; i < sizeof(ColorKeyRi) / sizeof(ColorKeyRi[0]); i++) {
				if (ColorKeyName[i].empty())
					break;
				std::cout << "    Name : " << ColorKeyName[i] << "\t    Ri : " << ColorKeyRi[i] << "\t    Zone : " << ColorKeyZone[i] << std::endl;
			}
		}
		std::cout << "\n#######################################################\n";
		Sleep(1000);
	}
}
