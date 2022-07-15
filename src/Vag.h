#pragma once

struct VagHeader
{
	uint32_t magic = 'pGAV';
	uint32_t hsize = 0x20000000;
	uint32_t nul = 0;
	uint32_t size;
	uint32_t freq;
	uint32_t padd[3] = { 0 };
	char name[16] = { 0 };

	VagHeader(uint32_t _size, uint32_t _freq)
	{
		size = _byteswap_ulong(_size);
		freq = _byteswap_ulong(_freq);
	}

	VagHeader(uint32_t _size, uint32_t _freq, const char* _name)
	{
		size = _byteswap_ulong(_size);
		freq = _byteswap_ulong(_freq);
		for (size_t i = 0; i < sizeof(name); i++)
		{
			name[i] = _name[i];
			if (_name[i] == '\0')
				break;
		}
	}
};

#define VAG_LINE_SIZE (0x10)
#define VAG_SAMPLES_IN_LINE (28)

class CVagDecoder
{
	const int8_t factorTbl[5][2] = {
		{ 0, 0 },
		{ 60, 0 },
		{ 115, -52 },
		{ 98, -55 },
		{ 122, -60 }
	};

	int16_t prev[2] = { 0, 0 };

	inline int16_t DecodeSample(int16_t s, uint8_t t)
	{
		int ds = ((prev[0] * factorTbl[t][0] + prev[1] * factorTbl[t][1] + 32) >> 6) + s;
		prev[1] = prev[0];
		prev[0] = ds;
		return std::clamp(ds, (int)INT16_MIN, (int)INT16_MAX);
	}
public:
	void ResetState()
	{
		prev[0] = prev[1] = 0;
	}

	uint32_t Decode(void* _inbuf, int16_t* outbuf, size_t size, bool stereo = false)
	{
		uint8_t* inbuf = (uint8_t*)_inbuf;
		size /= VAG_LINE_SIZE;

		uint32_t decodedLines = 0;
		while (size > 0) {
			uint8_t shift = *inbuf & 0xf;
			uint8_t t = *(inbuf++) >> 4; // factor table id

			// flags & 1 - (loop) end
			// flags & 2 - in loop
			// flags & 4 - loop start
			uint8_t flags = *(inbuf++);
			if (flags & 7) // failsafe
				break;

			for (int i = 0; i < VAG_SAMPLES_IN_LINE; i += 2) {
				uint8_t d = *(inbuf++);

				int16_t s = int16_t(d & 0xf) << 12;
				*(outbuf++) = DecodeSample(s >> shift, t);
				if (stereo) outbuf++;

				s = int16_t(d & 0xf0) << 8;
				*(outbuf++) = DecodeSample(s >> shift, t);
				if (stereo) outbuf++;
			}
			decodedLines++;
			size--;

			if (flags & 1) // end
				break;
		}
		return decodedLines * VAG_SAMPLES_IN_LINE;
	}
};

static uint32_t GetSampleCountFromVagSize(uint32_t size) { return size / VAG_LINE_SIZE * VAG_SAMPLES_IN_LINE; }
