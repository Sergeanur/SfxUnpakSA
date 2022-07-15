#include "Common.h"
#include "AudioContainers.h"

// Read lookups

std::vector<sAudioEntry> ReadBankLookup()
{
	std::vector<sAudioEntry> Banks;

	FILE* f = nullptr;
	fopen_s(&f, "CONFIG\\BankLkup.dat", "rb");
	if (f)
	{
		Banks.resize(fsize(f) / sizeof(sAudioEntry));
		fread(Banks.data(), sizeof(sAudioEntry), Banks.size(), f);
		fclose(f);
	}
	else printf("Couldn't read BankLkup.dat\n");
	return Banks;
}

std::vector<sAudioEntry> ReadTrackLookup()
{
	std::vector<sAudioEntry> Tracks;
	FILE* f = nullptr;
	fopen_s(&f, "CONFIG\\TrakLkup.dat", "rb");
	if (f)
	{
		Tracks.resize(fsize(f) / sizeof(sAudioEntry));
		fread(Tracks.data(), sizeof(sAudioEntry), Tracks.size(), f);
		fclose(f);
	}
	else printf("Couldn't read TrakLkup.dat\n");
	return Tracks;
}

// Read names from txt files

std::map<uint32_t, std::string> ReadAudioEvents()
{
	std::map<uint32_t, std::string> AudioEvents;
	FILE* f = nullptr;
	fopen_s(&f, "..\\data\\AudioEvents.txt", "r");
	if (f)
	{
		while (!feof(f))
		{
			char eventname[256];
			uint32_t eventId = 0;
			if (fscanf_s(f, "%s %i\n\n", eventname, 256, &eventId) == 2)
			{
				AudioEvents.emplace(eventId, eventname);
			}

		}
		fclose(f);
	}
	else printf("Couldn't read AudioEvents.txt\n");
	return AudioEvents;
}

std::vector<std::string> ReadBankNames(const char* filename)
{
	std::vector<std::string> bankNames;
	FILE* f = nullptr;
	fopen_s(&f, filename, "r");
	if (f)
	{
		char bankname[256];
		while (fscanf_s(f, "%s\n", bankname, 256) == 1)
			bankNames.emplace_back(bankname);
		fclose(f);
	}
	else printf("Couldn't read %s\n", filename);
	return bankNames;
}

std::map<uint32_t, std::string> ReadTrackNames(const char* filename)
{
	std::map<uint32_t, std::string> trackNames;
	FILE* f = nullptr;
	fopen_s(&f, filename, "r");
	if (f)
	{
		char trackname[256];
		int id;
		while (fscanf_s(f, "%s %i\n", trackname, 256, &id) == 2)
			trackNames.emplace(id, trackname);
		fclose(f);
	}
	else printf("Couldn't read %s\n", filename);
	return trackNames;
}

// Open paks

std::vector<std::pair<FILE*, std::string>> OpenSfxPakFiles(bool extension)
{
	std::vector<std::pair<FILE*, std::string>> pakFiles;

	FILE* f = nullptr;
	fopen_s(&f, "CONFIG\\PakFiles.dat", "rb");
	if (f)
	{
		char filename[52];
		while (fread(filename, 52, 1, f))
		{
			std::string s = "SFX\\";
			s += filename;
			if (extension)
				s += "01.PAK";
			FILE* f2 = nullptr;
			fopen_s(&f2, s.c_str(), "rb");
			pakFiles.emplace_back(f2, filename);
			if (!f2)
				printf("Couldn't read %s\n", s.c_str());
		}
		fclose(f);
	}
	else printf("Couldn't read PakFiles.dat\n");
	return pakFiles;
}

std::vector<std::pair<FILE*, std::string>> OpenStreamedPakFiles(bool extension)
{
	std::vector<std::pair<FILE*, std::string>> streamedPakFiles;
	FILE* f = nullptr;
	fopen_s(&f, "CONFIG\\StrmPaks.dat", "rb");
	if (f)
	{
		char filename[16];
		while (fread(filename, 16, 1, f))
		{

			std::string strFilename = filename;

			std::string s = "STREAMS\\";
			s += strFilename;
			if (extension)
				s += ".PAK";

			FILE* f2 = nullptr;
			fopen_s(&f2, s.c_str(), "rb");
			streamedPakFiles.emplace_back(f2, std::move(strFilename));
			if (!f2)
				printf("Couldn't read %s\n", s.c_str());
		}
		fclose(f);
	}
	else printf("Couldn't read PakFiles.dat\n");
	return streamedPakFiles;
}

void ClosePakFiles(std::vector<std::pair<FILE*, std::string>>& pakFiles)
{
	for (auto& [file, name] : pakFiles)
	{
		if (file)
			fclose(file);
	}
	pakFiles.clear();
}

// Read contents from paks

sBank ReadBank(const std::vector<std::pair<FILE*, std::string>>& pakFiles, const sAudioEntry& bankInfo)
{
	sBank bank;
	FILE* pakFile = pakFiles[bankInfo.pakId].first;

	bank.samples.resize(bankInfo.size);

	fseek(pakFile, bankInfo.offset, SEEK_SET);
	fread(&bank.header, sizeof(bank.header), 1, pakFile);
	fread(bank.samples.data(), 1, bank.samples.size(), pakFile);

	return bank;
}

uint8_t key[16];

void Decode(void* _data, size_t size, size_t offset)
{
	uint8_t* data = (uint8_t*)_data;
	for (size_t i = 0; i < size; i++)
		data[i] ^= key[(offset + i) & 0xf];
}

sTrack ReadTrack(const std::vector<std::pair<FILE*, std::string>>& streamedPakFiles, const sAudioEntry& trackInfo, bool decode)
{
	sTrack track;
	FILE* pakFile = streamedPakFiles[trackInfo.pakId].first;

	track.trackData.resize(trackInfo.size);

	fseek(pakFile, trackInfo.offset, SEEK_SET);
	fread(&track.header, sizeof(track.header), 1, pakFile);
	if (decode)
		Decode(&track.header, sizeof(track.header), trackInfo.offset);

	fread(track.trackData.data(), 1, track.trackData.size(), pakFile);
	if (decode)
		Decode(track.trackData.data(), track.trackData.size(), trackInfo.offset + sizeof(track.header));

	return track;
}
