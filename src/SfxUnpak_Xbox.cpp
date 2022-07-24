#include "Common.h"
#include "AudioContainers.h"
#include "Wav.h"
#include "ImaADPCM.h"

#ifndef XBOX
	#error XBOX must be defined
#endif // !XBOX

//#define DUMP_INFO

enum
{
	ENCODED_SAMPLES_IN_ADPCM_BLOCK = 64,
	SAMPLES_IN_ADPCM_BLOCK, // plus one

	ADPCM_INTERLEAVE_BYTES = 4,
	ADPCM_INTERLEAVE_SAMPLES = ADPCM_INTERLEAVE_BYTES * 2,

	ADPCM_BLOCK_BYTES = ENCODED_SAMPLES_IN_ADPCM_BLOCK / 2 + sizeof(CImaADPCM),
};

// convert to PCM
size_t decode(const uint8_t* adpcm, int16_t* pcm, size_t num_samples)
{
	CImaADPCM Converter;
	size_t i = 0;

	for (i = 0; i < num_samples; i += SAMPLES_IN_ADPCM_BLOCK)
	{
		Converter = *(CImaADPCM*)adpcm;
		adpcm += sizeof(CImaADPCM);
		*(pcm++) = Converter.GetSample();

		for (size_t j = 0; j < ENCODED_SAMPLES_IN_ADPCM_BLOCK / ADPCM_INTERLEAVE_SAMPLES; j++)
		{
			for (size_t k = 0; k < ADPCM_INTERLEAVE_SAMPLES; k++)
				pcm[k] = Converter.DecodeSample((k & 1) ? (adpcm[k / 2] >> 4) : (adpcm[k / 2] & 0xF));
			adpcm += ADPCM_INTERLEAVE_BYTES;
			pcm += ADPCM_INTERLEAVE_SAMPLES;
		}
	}
	return i;
}

const std::filesystem::path pathAppend = L"unpacked";

int main()
{
	printf("SfxUnpak_Xbox v0.2.3 by Serge (aka Sergeanur)\n");

	bool convertToPcm = false;

	while (true)
	{
		printf("Choose desired mode:\n0 - don't convert SFX to PCM\n1 - convert SFX to PCM\n");

		std::string line;
		std::getline(std::cin, line);

		if (line == "0")
		{
			convertToPcm = false;
			break;
		}
		else if (line == "1")
		{
			convertToPcm = true;
			break;
		}

		printf("Unknown value. Try Again.\n");
	}

	{
		auto streamedPakFiles = OpenStreamedPakFiles(false);
		auto Tracks = ReadTrackLookup();
		auto trackNames = ReadTrackNames("tracklist_xbox.txt");

		fread(&key, 1, sizeof(key), streamedPakFiles[0].first);
		for (size_t i = 0; i < sizeof(key); i++)
			if ((i & 4) == 0)
				key[i] ^= 0xff;

		for (size_t i = 0; i < Tracks.size(); i++)
		{
			sTrack track = ReadTrack(streamedPakFiles, Tracks[i], true);
			std::filesystem::path path = pathAppend;
			path /= L"STREAMS";
			path /= streamedPakFiles[Tracks[i].pakId].second;
			std::filesystem::create_directories(path);
			auto name = trackNames.find(i);
			if (name != trackNames.end())
				path /= name->second + ".ogg";
			else
				path /= L"Track_" + std::to_wstring(i) + L".ogg";
			printf("%S\n", path.c_str());
			FILE* f = nullptr;
			_wfopen_s(&f, path.c_str(), L"wb");
			if (f)
			{
				fwrite(track.trackData.data(), 1, track.trackData.size(), f);
				fclose(f);
			}
		}

		ClosePakFiles(streamedPakFiles);
	}

	{
		auto pakFiles = OpenSfxPakFiles(false);
		auto Banks = ReadBankLookup();
		auto AudioEvents = ReadAudioEvents();
		auto bankNames = ReadBankNames("banklist_xbox.txt");

#ifdef DUMP_INFO
		FILE* infofile = nullptr;
		fopen_s(&infofile, "script_info.txt", "w");
#endif

		for (uint32_t i = 0; i < Banks.size(); i++)
		{
			sBank bank = ReadBank(pakFiles, Banks[i]);
			std::filesystem::path path = pathAppend;
			path /= L"SFX";

			std::string bankName;
			if (i < bankNames.size())
				bankName = bankNames[i];
			else
				bankName = "Bank_" + std::to_string(i);

			path /= bankName;
			std::filesystem::create_directories(path);

			printf("%S\t\tSfxCount: %i\n", path.c_str(), bank.header.SfxCount);
			for (uint32_t sfxId = 0; sfxId < bank.header.SfxCount; sfxId++)
			{
				std::string strSfxName;
				if (Banks[i].pakId == 3)
				{
					static uint32_t first = i;
					auto it = AudioEvents.find((i - first) * 200 + 2000 + sfxId);
					if (it != AudioEvents.end())
						strSfxName = it->second.substr(6);
#ifdef DUMP_INFO
					fprintf_s(infofile, "%i lines %i frequency %i loopStartOffset %i unk %i \n", (i - first) * 200 + 2000 + sfxId, (bank.GetSfxSize(sfxId) / 2 + 27) / 28, bank.header.SfxEntries[sfxId].frequency
						, bank.header.SfxEntries[sfxId].loopStartOffset
						, bank.header.SfxEntries[sfxId].unk);
#endif
				}

				if (strSfxName.empty())
				{
					strSfxName = bankName;
					strSfxName += '_';
					char sfxnum[4];
					sprintf_s(sfxnum, "%03i", sfxId);
					strSfxName += sfxnum;
				}
				std::filesystem::path sfxpath = path / (strSfxName + ".wav");

				//printf("%s\n", sfxpath.string().c_str());
				FILE* f = nullptr;
				_wfopen_s(&f, sfxpath.c_str(), L"wb");
				if (f)
				{
					if (!convertToPcm)
					{
						DATAHeader datah(bank.GetSfxSize(sfxId));
						FMTHeader fmt(FMT_XBOX_ADPCM, 1, bank.GetSfxFrequency(sfxId));
						ExtraADPCM extra(2, ENCODED_SAMPLES_IN_ADPCM_BLOCK);
						RIFFHeader riffh(datah.ChunkSize + sizeof(fmt) + sizeof(extra) + sizeof(datah) + 4);

						fwrite(&riffh, sizeof(riffh), 1, f);
						fwrite(&fmt, sizeof(fmt), 1, f);
						fwrite(&extra, sizeof(extra), 1, f);
						fwrite(&datah, sizeof(datah), 1, f);
						fwrite(bank.GetSfxPointer(sfxId), 1, datah.ChunkSize, f);
					}
					else
					{
						static std::vector<int16_t> pcm;
						size_t SamplesCount = SAMPLES_IN_ADPCM_BLOCK * (bank.GetSfxSize(sfxId) / ADPCM_BLOCK_BYTES);
						if (pcm.size() < SamplesCount)
							pcm.resize(SamplesCount);
						decode(bank.GetSfxPointer(sfxId), pcm.data(), SamplesCount);

						DATAHeader datah(SamplesCount * sizeof(int16_t));
						FMTHeader fmt(FMT_PCM, 1, bank.GetSfxFrequency(sfxId));
						RIFFHeader riffh(datah.ChunkSize + sizeof(fmt) + sizeof(datah) + 4);

						fwrite(&riffh, sizeof(riffh), 1, f);
						fwrite(&fmt, sizeof(fmt), 1, f);
						fwrite(&datah, sizeof(datah), 1, f);
						fwrite(pcm.data(), sizeof(int16_t), SamplesCount, f);
					}
					fclose(f);
				}
			}
		}
#ifdef DUMP_INFO
		fclose(infofile);
#endif

		ClosePakFiles(pakFiles);
	}
	return 0;
}