#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include <flite/flite.h>
cst_voice *register_cmu_us_slt();
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
	
	printf("\x1b[37;1m\n%s\x1b[0m", text);
	
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
	PrintConsole top;
	PrintConsole bottom;
	consoleInit(GFX_TOP, &top);
    consoleInit(GFX_BOTTOM, &bottom);
	consoleSelect(&bottom);
	
    SwkbdState swkbd;
    SwkbdButton button = SWKBD_BUTTON_NONE;
	flite_init();
	voice = register_cmu_us_kal(NULL);
	
	waveBuf.status = NDSP_WBUF_DONE;
	svcSleepThread(2000000000);
	printf("\x1b[31;1m");
	printf("\x1b[12;8HWelcome to flite-3ds!");
	printf("\x1b[32;1m");
    printf("\x1b[13;8HPress A to begin");
	printf("\x1b[33;1m");
	printf("\x1b[14;8HPress R for female voice");
	printf("\x1b[34;1m");
	printf("\x1b[15;8HPress L for male voice\x1b[0m");
	consoleSelect(&top);
	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();
        if(kDown & KEY_R)
		{  
			voice=register_cmu_us_slt(NULL);
			consoleSelect(&bottom);
			printf("\x1b[32;1m");
			printf("\x1b[16;8HVoice changed to female\x1b[0m");
			consoleSelect(&top);
		}
		if(kDown & KEY_L)
		{
			voice=register_cmu_us_kal(NULL);
			consoleSelect(&bottom);
			printf("\x1b[32;1m");
			printf("\x1b[16;8HVoice changed to   male\x1b[0m");
			consoleSelect(&top);
		}
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
