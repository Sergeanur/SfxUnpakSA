#include "Common.h"
#include "AudioContainers.h"
#include "Wav.h"

//#define DUMP_INFO

const std::filesystem::path pathAppend = L"unpacked";

int main()
{
	printf("SfxUnpak v0.2.1\n");
	{
		auto streamedPakFiles = OpenStreamedPakFiles(false);
		auto Tracks = ReadTrackLookup();
		auto trackNames = ReadTrackNames("tracklist.txt");

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
		auto bankNames = ReadBankNames("banklist.txt");

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
					DATAHeader datah(bank.GetSfxSize(sfxId));
					FMTHeader fmt(FMT_PCM, 1, bank.GetSfxFrequency(sfxId));
					RIFFHeader riffh(datah.ChunkSize + sizeof(fmt) + sizeof(datah) + 4);

					fwrite(&riffh, sizeof(riffh), 1, f);
					fwrite(&fmt, sizeof(fmt), 1, f);
					fwrite(&datah, sizeof(datah), 1, f);
					fwrite(bank.GetSfxPointer(sfxId), 1, datah.ChunkSize, f);
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