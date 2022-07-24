#include "Common.h"
#include "AudioContainers.h"
#include "Wav.h"
#include "Vag.h"
#include <fstream>

//#define DUMP_INFO

const std::filesystem::path pathAppend = L"unpacked";
const size_t BLOCK_SIZE = 0x21000;

std::string GetTrackName(const std::map<uint32_t, std::string>& trackNames, uint32_t i)
{
	std::string strName;
	auto name = trackNames.find(i);
	if (name != trackNames.end())
		strName = name->second;
	else
		strName = "Track_" + std::to_string(i);
	return strName;
}

size_t GetNumberOfVagLines(uint8_t* data, size_t size)
{
	size /= VAG_LINE_SIZE;
	if (size == 0) return 0;

	size_t result = 0;

	while (result < size)
	{
		if (data[1] & 1)
			break;
		result++;
		data += VAG_LINE_SIZE;
	}

	return result + 1;
}

int main()
{
	bool dumpVAGS = false;
	bool convertToWav = false;

	printf("SfxUnpak_PS2 v0.2.2 by Serge (aka Sergeanur)\n");

	while (true)
	{
		printf("Choose desired mode:\n0 - convert to wav\n1 - save vag files\n2 - both\n");

		std::string line;
		std::getline(std::cin, line);

		if (line == "0")
		{
			convertToWav = true;
			break;
		}
		else if (line == "1")
		{
			dumpVAGS = true;
			break;
		}
		else if (line == "2")
		{
			convertToWav = true;
			dumpVAGS = true;
			break;
		}

		printf("Unknown value. Try Again.\n");
	}

#if 1
	{
		auto streamedPakFiles = OpenStreamedPakFiles();
		auto Tracks = ReadTrackLookup();
		auto trackNames = ReadTrackNames("tracklist_ps2.txt");

		for (size_t i = 0; i < Tracks.size(); i++)
		{
			sTrack track = ReadTrack(streamedPakFiles, Tracks[i]);
			std::filesystem::path path = pathAppend;
			path /= L"STREAMS";
			path /= streamedPakFiles[Tracks[i].pakId].second;
			std::filesystem::create_directories(path);

			std::vector<FILE*> files;
			std::vector<int> size_left(track.header.nChannels);
			std::vector<CVagDecoder> decoders(track.header.nChannels);

			for (uint16_t n = 0; n < track.header.nChannels; n++)
				size_left[n] = track.header.channels[n].size;

			FILE *fLow = nullptr, *fHigh = nullptr;
			std::vector<int16_t> pcmLowBuf;
			std::vector<int16_t> pcmHighBuf;
			std::vector<int16_t*> pcmChannelBuffer(track.header.nChannels);

			size_t highChannel = 0;

			std::string strName = GetTrackName(trackNames, i);

			{
				std::filesystem::path path2 = path / strName;
				printf("%S channels: %i\n", path2.c_str(), track.header.nChannels);
			}

			// ==== LAMBDA FUNCTIONS START ===

			auto CreateWavFile = [](const std::filesystem::path &path, uint32_t freq, uint32_t dataSize)
			{
				FILE* f = nullptr;

				_wfopen_s(&f, path.c_str(), L"wb");
				if (f)
				{
					DATAHeader datah(dataSize);
					FMTHeader fmt(FMT_PCM, 2, freq);
					RIFFHeader riffh(datah.ChunkSize + sizeof(fmt) + sizeof(datah) + 4);
					fwrite(&riffh, sizeof(riffh), 1, f);
					fwrite(&fmt, sizeof(fmt), 1, f);
					fwrite(&datah, sizeof(datah), 1, f);
				}

				return f;
			};

			auto CreateVagFile = [](const std::filesystem::path& path, uint32_t freq, uint32_t dataSize, const std::string& vagName = "")
			{
				FILE* f = nullptr;

				_wfopen_s(&f, path.c_str(), L"wb");
				if (f)
				{
					VagHeader vagheader(dataSize + 16, freq, vagName.c_str());
					char nil[16] = { 0 };
					fwrite(&vagheader, sizeof(vagheader), 1, f);
					fwrite(nil, 1, 16, f);
				}
				return f;
			};

			auto GetBlockSize = [](uint32_t freq) { return freq * 4096 / 1500; };
		
			// ==== LAMBDA FUNCTIONS END ===

			switch (track.header.nChannels)
			{
			case 4:
			{
				std::string strNameLow = strName + "_LOW";
				if (convertToWav)
				{
					pcmLowBuf.resize(GetSampleCountFromVagSize(GetBlockSize(track.header.channels[0].freq)) * 2);
					pcmChannelBuffer[0] = pcmLowBuf.data();
					pcmChannelBuffer[1] = pcmLowBuf.data() + 1;
					fLow = CreateWavFile(path / (strNameLow + ".wav"), track.header.channels[0].freq, GetSampleCountFromVagSize(track.header.channels[0].size) * 4);
				}

				if (dumpVAGS)
				{
					std::string vagName = strNameLow + "_LEFT";
					files.push_back(CreateVagFile(path / (vagName + ".VAG"), track.header.channels[0].freq, track.header.channels[0].size, vagName));

					vagName = strNameLow + "_RIGHT";
					files.push_back(CreateVagFile(path / (vagName + ".VAG"), track.header.channels[1].freq, track.header.channels[1].size, vagName));
				}
				highChannel = 2;

			}
			[[__fallthrough]]
			case 2:
			{
				if (convertToWav)
				{
					pcmHighBuf.resize(GetSampleCountFromVagSize(GetBlockSize(track.header.channels[highChannel].freq)) * 2);
					pcmChannelBuffer[highChannel] = pcmHighBuf.data();
					pcmChannelBuffer[highChannel + 1] = pcmHighBuf.data() + 1;
					fHigh = CreateWavFile(path / (strName + ".wav"), track.header.channels[highChannel].freq, GetSampleCountFromVagSize(track.header.channels[highChannel].size) * sizeof(int16_t) * 2);
				}

				if (dumpVAGS)
				{
					std::string vagName = strName + "_LEFT";
					files.push_back(CreateVagFile(path / (vagName + ".VAG"), track.header.channels[highChannel].freq, track.header.channels[highChannel].size, vagName));

					vagName = strName + "_RIGHT";
					files.push_back(CreateVagFile(path / (vagName + ".VAG"), track.header.channels[highChannel + 1].freq, track.header.channels[highChannel + 1].size, vagName));
				}

				break;
			}
			default:
				throw 0;
				break;
			}

			uint32_t offset = 0;
			bool notZero = false;
			do
			{
				for (uint16_t n = 0; n < track.header.nChannels; n++)
				{
					if (size_left[n] <= 0) continue;

					uint32_t blockSize = GetBlockSize(track.header.channels[n].freq);

					if (dumpVAGS)
						fwrite(track.trackData.data() + offset, 1, blockSize, files[n]);

					if (convertToWav)
						decoders[n].Decode(track.trackData.data() + offset, pcmChannelBuffer[n], blockSize, true);

					offset += blockSize;
					size_left[n] -= blockSize;
				}

				if (convertToWav)
				{
					if (fLow)
						fwrite(pcmLowBuf.data(), sizeof(int16_t), pcmLowBuf.size(), fLow);

					fwrite(pcmHighBuf.data(), sizeof(int16_t), pcmHighBuf.size(), fHigh);
				}

				// block is always the same size despite the number of channels
				if ((offset % BLOCK_SIZE) != 0)
					offset += BLOCK_SIZE - (offset % BLOCK_SIZE);

				notZero = false;

				for (uint32_t n = 0; n < track.header.nChannels; n++)
				{
					if (size_left[n] > 0)
					{
						notZero = true;
						break;
					}
				}

			} while (notZero);

			if (convertToWav)
			{
				if (fLow)
				{
					fclose(fLow);
					fLow = nullptr;
				}
				if (fHigh)
				{
					fclose(fHigh);
					fHigh = nullptr;
				}
			}

			if (dumpVAGS)
			{
				for (FILE* file : files)
				{
					if (file)
						fclose(file);
				}
				files.clear();
			}
		}


		ClosePakFiles(streamedPakFiles);
	}
#endif

	{
		auto pakFiles = OpenSfxPakFiles();
		auto Banks = ReadBankLookup();
		auto AudioEvents = ReadAudioEvents();
		auto bankNames = ReadBankNames("banklist_ps2.txt");

#ifdef DUMP_INFO
		FILE* infofile = nullptr;
		fopen_s(&infofile, "script_info.txt", "w");
#endif

		for (uint32_t i = 0; i < Banks.size(); i++)
		{
			// TEMP!!!!!!!!!
			//if (Banks[i].pakId != 3) continue;

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

					/*
						uint32_t offset = 0;
						int32_t loopStartOffset = -1;
						uint16_t frequency = 0;
						int16_t unk = -1;
					*/
#ifdef DUMP_INFO
					fprintf_s(infofile, "%i lines %i frequency %i loopStartOffset %i unk %i \n", (i - first) * 200 + 2000 + sfxId, GetNumberOfVagLines(bank.GetSfxPointer(sfxId) + VAG_LINE_SIZE, bank.GetSfxSize(sfxId) - VAG_LINE_SIZE), bank.header.SfxEntries[sfxId].frequency
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

				std::filesystem::path sfxpath = path / strSfxName;
				uint32_t VagSize = bank.GetSfxSize(sfxId);
				if (convertToWav)
				{
					sfxpath.replace_extension(".wav");

					//printf("%s\n", sfxpath.string().c_str());
					FILE* f = nullptr;
					_wfopen_s(&f, sfxpath.c_str(), L"wb");

					if (f)
					{
						// skip first line in VAG (it's just zeroes)
						std::vector<int16_t> pcmbuf(GetSampleCountFromVagSize(VagSize - VAG_LINE_SIZE));

						CVagDecoder decoder;

						uint32_t decodedSamples = decoder.Decode(bank.GetSfxPointer(sfxId) + VAG_LINE_SIZE, pcmbuf.data(), VagSize - VAG_LINE_SIZE);

						DATAHeader datah(decodedSamples * sizeof(int16_t));
						FMTHeader fmt(FMT_PCM, 1, bank.GetSfxFrequency(sfxId));
						RIFFHeader riffh(datah.ChunkSize + sizeof(fmt) + sizeof(datah) + 4);

						fwrite(&riffh, sizeof(riffh), 1, f);
						fwrite(&fmt, sizeof(fmt), 1, f);
						fwrite(&datah, sizeof(datah), 1, f);
						fwrite(pcmbuf.data(), sizeof(int16_t), decodedSamples, f);
						fclose(f);
					}

				}

				if (dumpVAGS)
				{
					sfxpath.replace_extension(".VAG");

					//printf("%s\n", sfxpath.string().c_str());
					FILE* f = nullptr;
					_wfopen_s(&f, sfxpath.c_str(), L"wb");
					if (f)
					{
						VagHeader vagheader(VagSize, bank.GetSfxFrequency(sfxId), strSfxName.c_str());

						fwrite(&vagheader, sizeof(vagheader), 1, f);
						fwrite(bank.GetSfxPointer(sfxId), 1, VagSize, f);
						fclose(f);
					}
				}

			}
		}
#ifdef DUMP_INFO
		fclose(infofile);
#endif
		ClosePakFiles(pakFiles);//*/
	}
	return 0;
}