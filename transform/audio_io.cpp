#include "audio.hpp"
#include "audio_io.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <sndfile.h>

namespace{
    struct ReadContext{
        sf_count_t size;
        sf_count_t position;
        const unsigned char* data;
    };
    sf_count_t read_get_filelen(void* data){
        return static_cast<ReadContext*>(data)->size;
    }
    sf_count_t read_seek(sf_count_t offset, int whence, void* user_data){
        auto* ctx = static_cast<ReadContext*>(user_data);
        sf_count_t new_pos = 0;
        switch(whence){
            case SEEK_SET:
                new_pos = offset;
                break;
            case SEEK_CUR:
                new_pos = ctx->position+offset;
                break;
            case SEEK_END:
                new_pos = ctx->size + offset;
                break;
            default:
                return -1;
        }
        ctx->position = std::clamp(new_pos, static_cast<sf_count_t>(0), ctx->size);
        return ctx->position;
    }
    sf_count_t read_read(void* ptr, sf_count_t count, void* user_data){
        auto* ctx = static_cast<ReadContext*>(user_data);
        sf_count_t available = ctx->size - ctx->position;
        sf_count_t to_read = std::min(available, count);
        if(to_read>0){
            std::memcpy(ptr, ctx->data + ctx->position, to_read);
            ctx->position+=to_read;
        }
        return to_read;
    }
    sf_count_t read_write(const void *ptr, sf_count_t count, void *user_data){
        return 0;
    }
    sf_count_t read_tell(void* data){
        return static_cast<ReadContext*>(data)->position;
    }
    struct WriteContext{
        sf_count_t position;
        std::vector<unsigned char>& buffer;
    };
    sf_count_t write_get_filelen(void* user_data){
        return static_cast<WriteContext*>(user_data)->buffer.size();
    }
    sf_count_t write_seek(sf_count_t offset, int whence, void* user_data){
        auto* ctx = static_cast<WriteContext*>(user_data);
        sf_count_t new_pos = 0;
        sf_count_t buffer_size = static_cast<sf_count_t>(ctx->buffer.size());
        switch(whence){
            case SEEK_SET:
                new_pos = offset;
                break;
            case SEEK_CUR:
                new_pos = ctx->position+offset;
                break;
            case SEEK_END:
                new_pos = buffer_size + offset;
                break;
            default:
                return -1;
        }
        if(new_pos>buffer_size){
            ctx->buffer.resize(new_pos, 0);
        }
        ctx->position = new_pos;
        return ctx->position;
    }
    sf_count_t write_read(void* ptr, sf_count_t count, void* user_data){
        return 0;
    }
    sf_count_t write_write(const void *ptr, sf_count_t count, void *user_data){
        auto* ctx = static_cast<WriteContext*>(user_data);
        const unsigned char* data_ptr = static_cast<const unsigned char*>(ptr);
        if(ctx->position+count > static_cast<sf_count_t>(ctx->buffer.size())){
            ctx->buffer.resize(ctx->position+count, 0);
        }
        std::memcpy(ctx->buffer.data()+ctx->position, data_ptr, count);
        ctx->position+=count;
        return count;
    }
    sf_count_t write_tell(void* user_data){
        return static_cast<WriteContext*>(user_data)->position;
    }

}

namespace AudioIO{
    Audio decode_from_memory(const std::vector<unsigned char>& raw_bytes){
        ReadContext ctx = {
            .size = static_cast<sf_count_t>(raw_bytes.size()),
            .position= static_cast<sf_count_t>(0),
            .data = raw_bytes.data() 
        };
        SF_VIRTUAL_IO vio = {read_get_filelen, read_seek, read_read, read_write, read_tell};
        SF_INFO info;
        std::memset(&info, 0, sizeof(info));
        SNDFILE* file = sf_open_virtual(&vio, SFM_READ, &info, &ctx);
        if(!file){
            throw std::runtime_error("Failed to decode audio file");
        }
        Audio audio_file(info.samplerate, info.channels, info.frames);
        sf_readf_float(file, audio_file.data.data(), info.frames);
        sf_close(file);
        return audio_file;
    }
    std::vector<unsigned char> encode_to_memory(const Audio& audio_data, const int format){
        std::vector<unsigned char> output_buffer;
        output_buffer.reserve((audio_data.frames*audio_data.channels*2)+128);

        WriteContext ctx = {
            .position = 0,
            .buffer = output_buffer
        };
        SF_VIRTUAL_IO vio = {write_get_filelen, write_seek, write_read, write_write, write_tell};
        SF_INFO info;
        std::memset(&info, 0, sizeof(info));
        info.frames = audio_data.frames;
        info.channels = audio_data.channels;
        info.format = format;
        info.samplerate = audio_data.sample_rate;

        SNDFILE* output_file = sf_open_virtual(&vio, SFM_WRITE, &info, &ctx);
        if(!output_file){
            throw std::runtime_error("Failed to encode audio file");
        }
        sf_writef_float(output_file, audio_data.data.data(), info.frames);

        sf_close(output_file);
        return output_buffer;
    }
    
}