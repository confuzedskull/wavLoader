#include <cstdlib>
#include <iostream>
#include <AL/al.h>
#include <AL/alc.h>

//by defining these types we can avoid using windows.h
typedef unsigned int DWORD;
typedef char BYTE;

int main(int argc, char *argv[])
{
    //open wave file
    FILE *sound_file = NULL;
    sound_file=fopen("./sound.wav","rb");//read the file in binary mode
    if(!sound_file)
        std::cerr<<"error opening sound file\n";

    //file info
    char type[4];
    DWORD size,chunk_size;
    short format_type,channels;
    DWORD sample_rate,average_byte_sample;
    short byte_sample,bit_sample;
    DWORD data_size;

    //check the sound file
    fread(type,sizeof(char),4,sound_file);
    if(type[0]!='R' || type[1]!='I' || type[2]!='F' || type[3]!='F')
        std::cerr<<"not riff\n";
    fread(&size, sizeof(DWORD),1,sound_file);
    fread(type, sizeof(char),4,sound_file);
    if (type[0]!='W' || type[1]!='A' || type[2]!='V' || type[3]!='E')
        std::cerr<<"not wave file\n";
    fread(type,sizeof(char),4,sound_file);
    if (type[0]!='f' || type[1]!='m' || type[2]!='t' || type[3]!=' ')
        std::cerr<<"not fmt\n";

    //read info about the wave file
    fread(&chunk_size,sizeof(DWORD),1,sound_file);
    fread(&format_type,sizeof(short),1,sound_file);
    fread(&channels,sizeof(short),1,sound_file);
    fread(&sample_rate,sizeof(DWORD),1,sound_file);
    fread(&average_byte_sample,sizeof(DWORD),1,sound_file);
    fread(&byte_sample,sizeof(short),1,sound_file);
    fread(&bit_sample,sizeof(short),1,sound_file);
    fread(type,sizeof(char),4,sound_file);
    if (type[0]!='d' || type[1]!='a' || type[2]!='t' || type[3]!='a')
        std::cerr<<"missing data\n";
    fread(&data_size,sizeof(DWORD),1,sound_file);
    unsigned char* buf= new unsigned char[data_size];
    fread(buf,sizeof(BYTE),data_size,sound_file);

    //display the info about the wave file
    std::cout<<"Chunk Size: "<<chunk_size<<std::endl;
    std::cout<<"Format Type: "<<format_type<<std::endl;
    std::cout<<"Channels: "<<channels<<std::endl;
    std::cout<<"Sample Rate: "<<sample_rate<<std::endl;
    std::cout<<"Average Bytes Per Second: "<<average_byte_sample<<std::endl;
    std::cout<<"Bytes Per Sample: "<< byte_sample<<std::endl;
    std::cout<<"Bits Per Sample: "<<bit_sample<<std::endl;
    std::cout<<"Data Size: "<<data_size<<std::endl;

    //initialize OpenAL
    ALCdevice *device;
    ALCcontext *context;
    device = alcOpenDevice(NULL);
    if(!device)
        std::cerr<<"no sound device\n";
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    if(!context)
        std::cerr<<"no sound context\n";
    ALuint source;
    ALuint buffer;
    ALuint frequency=sample_rate;
    ALenum format=0;
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);
    if(alGetError() != AL_NO_ERROR)
        std::cerr<<"error generating source\n";

    //identify wave format
    if(bit_sample == 8)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO8;
        else if(channels == 2)
            format = AL_FORMAT_STEREO8;
    }
    else if(bit_sample == 16)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO16;
        else if(channels == 2)
            format = AL_FORMAT_STEREO16;
    }
    if(!format)
        std::cerr<<"wrong bit sample\n";

    alBufferData(buffer, format, buf, data_size, frequency);
    if(alGetError() != AL_NO_ERROR)
        std::cerr<<"error loading buffer\n";

    //sound settings
    ALfloat source_position[] = {0.0, 0.0, 0.0};
    ALfloat source_velocity[] = {0.0, 0.0, 0.0};
    ALfloat listener_position[] = {0.0, 0.0, 0.0};
    ALfloat listener_velocity[] = {0.0, 0.0, 0.0};
    ALfloat listener_orientation[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

    //listener
    alListenerfv(AL_POSITION, listener_position);
    alListenerfv(AL_VELOCITY, listener_velocity);
    alListenerfv(AL_ORIENTATION, listener_orientation);

    //source
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, 1.0f);
    alSourcefv(source, AL_POSITION, source_position);
    alSourcefv(source, AL_VELOCITY, source_velocity);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    //play the sound
    alSourcePlay(source);
    if(alGetError() != AL_NO_ERROR)
        std::cerr<<"error playing sound\n";
    else
        std::cout<<"playing sound...\n";
    std::cout<<"press enter to quit.\n";
    std::cin.get();

    //clean everything up before closing
    fclose(sound_file);
    delete[] buf;
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
    return 0;
}
