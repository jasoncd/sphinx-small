#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

short myabs(short v)
{
  if(v<0) return -1*v;
  else
  return v;
}

void write_wave_header(FILE* outf, int len)
{
    int bytes = 2*len;
    int file_size = 2*len+36;
    int data_header_length = 16;
    short format_type = 1;
    short number_of_channels = 1;
    int sample_rate = 16000;
    int bytes_per_second = 32000;
    short bytes_per_frame = 2;
    short bits_per_sample = 16;

    fwrite("RIFF", 4, sizeof(char), outf);
    fwrite(&file_size, 4, sizeof(char), outf);
    fwrite("WAVE", 4, sizeof(char), outf);
    fwrite("fmt ", 4, sizeof(char), outf);
    fwrite(&data_header_length, 4, sizeof(char), outf);
    fwrite(&format_type, 2, sizeof(char), outf);
    fwrite(&number_of_channels, 2, sizeof(char), outf);
    fwrite(&sample_rate, 4, sizeof(char), outf);
    fwrite(&bytes_per_second, 4, sizeof(char), outf);
    fwrite(&bytes_per_frame, 2, sizeof(char), outf);
    fwrite(&bits_per_sample, 2, sizeof(char), outf);
    fwrite("data", 4, sizeof(char), outf);
    fwrite(&bytes, 4, sizeof(char), outf);

}


void save_wave(char* dir, short* utt, int uttlen, int cnt, char *uttid)
{
   FILE* outf;
   char filename[128];

   if(cnt<10)
   {
     sprintf(filename, "%s/recording-000%i.wav", dir, cnt);
     sprintf(uttid, "recording-000%i", cnt);
   }
   else
   if(cnt<100)
   {
     sprintf(filename, "%s/recording-00%i.wav", dir, cnt);
     sprintf(uttid, "recording-00%i", cnt);
   }
   else
   if(cnt<1000)
   {
     sprintf(filename, "%s/recording-0%i.wav", dir, cnt);
     sprintf(uttid, "recording-0%i", cnt);
   }
   else
   {
     sprintf(filename, "%s/recording-%i.wav", dir, cnt);
     sprintf(uttid, "recording-%i", cnt);
   }

   outf = fopen(filename, "wb");

   write_wave_header(outf, uttlen);

   fwrite(utt, uttlen, sizeof(short), outf);

   fclose(outf);

}



int vad(short *utt)
{
    int err;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int sampleRate = 16000;
    int dir;
    snd_pcm_uframes_t frames = 320;
    const char *device = "plughw:1,0"; // USB microphone
    //const char *device = "default"; // Integrated system microphone
    //char *buffer;
    short buffer[320];

    // vad vars
    float HiLoRate = 10.0;  // rate for voicing

    // end silence length (default 40 frames, 1 frame = 160 samples)
    int esl = 40;

    // average, lo, floor and hi values of each 320 samples of up to 40 sec.
    short avg;
    float floor;  // global sil floor
    short lo;     // local lo of every second
    short hi;

    // hi to floor ratio
    float r;

    // end of sentence cnt
    int ecnt = 0;

    // beginning of sentence cnt
    //int bcnt = 0;

    // voice cnt
    int vcnt = 0;
    // high energy voice counter
    int hvcnt = 0;

    // voice state
    int vs = 0;

    // frame (320 samples) counter
    int fcnt = 0;

    // utt index
    int idx = 0;

    union abc
    {
      char c[2];
      short s;
    } u;

    int i,j,k,sum;

    /* Open PCM device for recording (capture). */
    err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err)
    {
        fprintf(stderr, "Unable to open PCM device: %s\n", snd_strerror(err));
        return err;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* ### Set the desired hardware parameters. ### */

    /* Interleaved mode */
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err)
    {
        fprintf(stderr, "Error setting interleaved mode: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    /* Signed 16-bit little-endian format */
    err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    if (err)
    {
        fprintf(stderr, "Error setting format: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    /* mono channel */
    err = snd_pcm_hw_params_set_channels(handle, params, 1);
    if (err)
    {
        fprintf(stderr, "Error setting channels: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    /* 16000 bits/second sampling rate */
    err = snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, &dir);
    if (err)
    {
        fprintf(stderr, "Error setting sampling rate (%d): %s\n", sampleRate, snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }
 
    /* Set period size*/
    err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    if (err)
    {
        fprintf(stderr, "Error setting period size: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0)
    {
        fprintf(stderr, "Unable to set HW parameters: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    /* Use a buffer large enough to hold one period */
    err = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    if (err)
    {
        fprintf(stderr, "Error retrieving period size: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return err;
    }

    //size = frames * 2 * 1; /* 2 bytes/sample, 1 channel */

    err = snd_pcm_hw_params_get_period_time(params, &sampleRate, &dir);
    if (err)
    {
        fprintf(stderr, "Error retrieving period time: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        //free(buffer);
        return err;
    }

    // stabalize mic
    static int run_cnt = 0;
    if(run_cnt == 0)
    {
      for(i=0; i<30; i++)
        snd_pcm_readi(handle, buffer, frames);
      run_cnt = 1;
    }

    // init floor by 5 buffers
    sum = 0;
    int byteread;

    for(i=0; i<5; i++)
    {
      byteread = snd_pcm_readi(handle, buffer, frames);

      //printf("byteread: %i\n", byteread);

      for(j=0; j<320; j++)
      {
        sum += myabs(buffer[j]);

        //printf("%i ",buffer[j]);

      }

      //printf("\n\n");

      memcpy(&(utt[idx]),buffer,320*sizeof(short));
      idx+=320;
    }
    floor = (sum/(5*320.0));

    printf("noise floor: %.3f\n", floor);


    // the loop to capture one utt
    lo = 32000;
    hi = 0;
    for(;;)
    {
      sum = 0;
      hi = 0;
      snd_pcm_readi(handle, buffer, frames);
      for(i=0; i<320; i++)
      {
        if(buffer[i] > hi) hi = buffer[i];
        sum += myabs(buffer[i]);
      }
      avg = (short) (sum/320.0);
      if(avg < lo) lo = avg; // capture local lo's
      if((fcnt % 40) == 0)  // update floor each 0.5 second
      {
        floor = (0.6*floor) + (0.4*lo);
        lo = 32000;
      }

      fcnt++;

      r = hi / floor;

      //printf("hi: %i, floor: %.3f, rate: %.3f\n", hi, floor, r);

      // utt begin
      if(r > 10 && vs == 0) 
      {
        vs = 1;
        vcnt ++;
        memcpy(&(utt[idx]),buffer,320*sizeof(short));
        idx+=320;
      }
      else if(r > 10 && vs == 1)
      {
        ecnt = 0;
        vcnt ++;
        memcpy(&(utt[idx]),buffer,320*sizeof(short));
        idx+=320;
      }
      else if(vs == 1 && r <= 10 && ecnt < 20)
      {
        ecnt ++;
        memcpy(&(utt[idx]),buffer,320*sizeof(short));
        idx+=320;
      }
      else if(vs == 1 && ecnt == 20)
      {
        break;
      }

      if(r > 30) hvcnt ++;
      if(idx>160000) break;
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    
    //if(hvcnt > 15)
    if(hvcnt > 0)
    return idx;
    else
    return -1;
}



int main(int argc, char** argv)
{
  int utt_len;
  int uttcnt = 0;
  char uttid[100];
  FILE* namefile;
  short utt[1000000];

  while(1)
  { 
      utt_len = vad(utt);
    

      if(utt_len > 9600)  // 0.3 sec
      {
        uttcnt++;
        printf("Voice captured %.2f sec.\n", utt_len/16000.0 );
        save_wave("audio/", utt, utt_len, uttcnt, uttid);
	namefile = fopen("tt","w");
        printf("uttid: %s\n", uttid);
        fprintf(namefile,"%s\n", uttid);
	fclose(namefile);
        system("mv tt namefile.txt");
      }

  }

  return 0;
}

