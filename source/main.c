#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include <flite/flite.h>

cst_voice *register_cmu_us_kal();

ndspWaveBuf waveBuf;
cst_wave *fliteWave;
cst_voice *voice;
u16 *samples = NULL;
char mybuf[1024];

char *tl(SwkbdState swkbd ,char *texgen)
{        
            SwkbdButton button = SWKBD_BUTTON_NONE;
            swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
            swkbdSetHintText(&swkbd, texgen);
            swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	        
            button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
			return mybuf;	
}

void processText(const char *text)
{
	static int channel = 0;
	int dataSize;
	
	printf("\n%s ", text);
	
	fliteWave = flite_text_to_wave(text, voice);
	dataSize = fliteWave->num_samples * fliteWave->num_channels * 2;
	
	linearFree(samples);
	samples = linearAlloc(dataSize);
	memcpy(samples, fliteWave->samples, dataSize);
	
	memset(&waveBuf, 0, sizeof(ndspWaveBuf));
	waveBuf.data_vaddr = samples;
	waveBuf.nsamples = fliteWave->num_samples / fliteWave->num_channels;
	waveBuf.looping = false;
	waveBuf.status = NDSP_WBUF_FREE;
	
	ndspChnReset(channel);
	ndspChnSetInterp(channel, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(channel, fliteWave->sample_rate);
	ndspChnSetFormat(channel, NDSP_FORMAT_MONO_PCM16);
	
	DSP_FlushDataCache((u8*)samples, dataSize);
	ndspChnWaveBufAdd(channel, &waveBuf);
}

int main()
{
	gfxInitDefault();
	ndspInit();
	consoleInit(GFX_TOP, NULL);
	
    SwkbdState swkbd;
    SwkbdButton button = SWKBD_BUTTON_NONE;
	flite_init();
	voice = register_cmu_us_kal(NULL);
	
	waveBuf.status = NDSP_WBUF_DONE;
	svcSleepThread(2000000000);
    puts("Press A to begin");
	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break;
        
		if (waveBuf.status == NDSP_WBUF_DONE)
		{
			if(kDown & KEY_A)
			{
				
			char*dg=tl(swkbd,"Enter the sentence");
			processText(dg);
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}
}
	linearFree(samples);

	ndspExit();
	gfxExit();
	return 0;
}
