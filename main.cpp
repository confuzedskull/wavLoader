#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

//global variables
std::ifstream sound_file;
bool loaded=false;
char* data;
int16_t format_type, channels, byte_sample, bit_sample;
int32_t chunk_size, sample_rate, byte_rate, data_size;
ALCdevice *device;
ALCcontext *context;
ALuint source;
ALuint buffer;

//this function checks endianness
bool is_big_endian()
{
    int i=1;
    return !((char*)&i)[0];
}

//this function retrieves the appropriate value according to endianness
int16_t to_int16(char* buffer, int length)
{
    int16_t i=0;
    if(!is_big_endian())
        for(int j=0;j<length;j++)
            ((char*)&i)[j]=buffer[j];
    else
        for(int j=0;j<length;j++)
            ((char*)&i)[3-j]=buffer[j];
    return i;
}

//this function retrieves the appropriate value according to endianness
int32_t to_int32(char* buffer, int length)
{
    int32_t i=0;
    if(!is_big_endian())
        for(int j=0;j<length;j++)
            ((char*)&i)[j]=buffer[j];
    else
        for(int j=0;j<length;j++)
            ((char*)&i)[sizeof(int)-1-j]=buffer[j];
    return i;
}

void load(std::string filename)
{
    char file_info[4];
    //open wave file
    sound_file.open(filename.c_str(),std::ios::binary);
    if(sound_file.bad())
    {
        std::cerr<<"error opening sound file\n";
        return;
    }
    sound_file.read(file_info,4);
    //read file information
    sound_file.read(file_info,4);//"RIFF" label
    sound_file.read(file_info,4);//"WAVE" label
    sound_file.read(file_info,4);//"fmt" label
    sound_file.read(file_info,4);//chunk size
    chunk_size=to_int32(file_info,4);
    sound_file.read(file_info,2);//format type
    format_type=to_int16(file_info,2);
    sound_file.read(file_info,2);//channels
    channels=to_int16(file_info,2);
    sound_file.read(file_info,4);//sample rate
    sample_rate=to_int32(file_info,4);
    sound_file.read(file_info,4);//byte rate
    byte_rate=to_int32(file_info,4);
    sound_file.read(file_info,2);//byte sample
    byte_sample=to_int16(file_info,2);
    sound_file.read(file_info,2);//bit sample
    bit_sample=to_int16(file_info,2);
    sound_file.read(file_info,4);//"data" label
    sound_file.read(file_info,4);//data size
    data_size=to_int32(file_info,4);
    data= new char[data_size];//create a buffer to store sound data
    sound_file.read(data,data_size);//retrieve sound data

    ALuint format=0;
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);
    if(alGetError() != AL_NO_ERROR)
    {
        std::cerr<<"error generating source\n";
        return;
    }
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
    {
        std::cerr<<"wrong bit sample\n";
        return;
    }
    alBufferData(buffer, format, data, data_size, sample_rate);
    if(alGetError() != AL_NO_ERROR)
    {
        std::cerr<<"error loading buffer\n";
        return;
    }
    //source settings
    ALfloat source_position[] = {0.0, 0.0, 0.0};
    ALfloat source_velocity[] = {0.0, 0.0, 0.0};
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, 1.0f);
    alSourcefv(source, AL_POSITION, source_position);
    alSourcefv(source, AL_VELOCITY, source_velocity);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    loaded=true;
    sound_file.close();
}

int main(int argc, char *argv[])
{
    //initialize OpenAL
    device = alcOpenDevice(NULL);
    if(!device)
        std::cerr<<"no sound device\n";
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    if(!context)
        std::cerr<<"no sound context\n";
    //listener settings
    ALfloat listener_position[] = {0.0, 0.0, 0.0};
    ALfloat listener_velocity[] = {0.0, 0.0, 0.0};
    ALfloat listener_orientation[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
    alListenerfv(AL_POSITION, listener_position);
    alListenerfv(AL_VELOCITY, listener_velocity);
    alListenerfv(AL_ORIENTATION, listener_orientation);
    //process arguments
    if(argc==2)
    {
        load(argv[1]);
        alSourcePlay(source);
    }
    else
        std::cout<<"No wave file loaded. Type 'help' for a list of available commands.\n";
    //enter command loop
    std::string command,filename;
    while(true)
    {
        std::cin>>command;
        if(command=="load")
        {
            std::cin>>filename;
            load(filename);
        }
        else if(command=="info")
        {
            std::cout<<"Chunk Size: "<<chunk_size<<std::endl;
            std::cout<<"Format type: "<<format_type<<std::endl;
            std::cout<<"Channels: "<<channels<<std::endl;
            std::cout<<"Sample Rate: "<<sample_rate<<std::endl;
            std::cout<<"Byte Rate: "<<byte_rate<<std::endl;
            std::cout<<"Bytes Per Sample: "<<byte_sample<<std::endl;
            std::cout<<"Bits Per Sample: "<<bit_sample<<std::endl;
            std::cout<<"Data Size: "<<data_size<<std::endl;
        }
        else if(command=="play")
            alSourcePlay(source);
        else if(command=="pause")
            alSourcePause(source);
        else if(command=="stop")
            alSourceStop(source);
        else if(command=="rewind")
            alSourceRewind(source);
        else if(command=="quit")
            break;
        else
        {
            if(command!="help")
                std::cout<<"Command not recognized. Type 'help' for a list of available commands. \n";
            else
            {
                std::cout<<"'load <filename>' - loads the given wave file\n";
                std::cout<<"'info' - displays file information\n";
                std::cout<<"'play' - plays the current wave file\n";
                std::cout<<"'pause' - halts the sound momentarily\n";
                std::cout<<"'stop' - terminates the sound completely\n";
                std::cout<<"'rewind' - reverses the playback of the sound\n";
                std::cout<<"'quit' - closes the program\n";
            }
        }
    }
    //free memory
    delete[] data;
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
    return 0;
}
