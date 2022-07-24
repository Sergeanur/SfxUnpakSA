#pragma once

struct sAudioEntry
{
	uint8_t pakId = 0;
	uint32_t offset = 0;
	uint32_t size = 0;
};

struct sBankHeaderEntry
{
	uint32_t offset = 0;
	int32_t loopStartOffset = -1;
	uint16_t frequency = 0;
	int16_t unk = -1;
};

struct sBankHeader
{
	uint32_t SfxCount = 0;
	sBankHeaderEntry SfxEntries[400];
};

struct sBank
{
	sBankHeader header;
	std::vector<uint8_t> samples;

	/*sBank() = default;
	sBank(const sBank&) = delete;
	sBank(sBank&&) = default;*/

	uint8_t* GetSfxPointer(uint32_t sfxId)
	{
		if (sfxId >= header.SfxCount)
			return nullptr;
		return samples.data() + header.SfxEntries[sfxId].offset;
	}

	uint32_t GetSfxSize(uint32_t sfxId)
	{
		if (sfxId >= header.SfxCount)
			return 0;
		if (sfxId == header.SfxCount - 1)
			return samples.size() - header.SfxEntries[sfxId].offset;
		return header.SfxEntries[sfxId + 1].offset - header.SfxEntries[sfxId].offset;
	}

	uint16_t GetSfxFrequency(uint32_t sfxId)
	{
		if (sfxId >= header.SfxCount)
			return 0;
		return header.SfxEntries[sfxId].frequency;
	}
};

struct sTrackBeat
{
	int time = -1;
	uint32_t button = 0;
};

struct sTrackChannel
{
	uint32_t size = 0;
	uint32_t freq = 0;
};

struct sTrackHeader
{
	sTrackBeat beats[1000];
#ifdef XBOX
	sTrackChannel channels[2];
#else
	sTrackChannel channels[8];
	uint16_t nChannels = 0;
#endif
};

struct sTrack
{
	sTrackHeader header;
	std::vector<uint8_t> trackData;
};

std::vector<sAudioEntry> ReadBankLookup();
std::vector<sAudioEntry> ReadTrackLookup();
std::map<uint32_t, std::string> ReadAudioEvents();
std::vector<std::string> ReadBankNames(const char* filename);
std::map<uint32_t, std::string> ReadTrackNames(const char* filename);

std::vector<std::pair<FILE*, std::string>> OpenSfxPakFiles(bool extension = true);
std::vector<std::pair<FILE*, std::string>> OpenStreamedPakFiles(bool extension = true);
void ClosePakFiles(std::vector<std::pair<FILE*, std::string>>& pakFiles);

sBank ReadBank(const std::vector<std::pair<FILE*, std::string>>& pakFiles, const sAudioEntry& bankInfo);
sTrack ReadTrack(const std::vector<std::pair<FILE*, std::string>>& streamedPakFiles, const sAudioEntry& trackInfo, bool decode = false);

extern uint8_t key[16];
