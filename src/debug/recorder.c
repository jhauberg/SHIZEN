#include "recorder.h" // z_recorder_*

#include <stdlib.h> // malloc, free, NULL
#include <stdio.h> // fwrite, popen, pclose, printf, sprintf, fprintf

#include "../internal.h"

#include "../viewport.h"

typedef struct SHIZRecorder {
    FILE * output;
    GLubyte * buffer;
    uint32_t frame_size;
    char command[196];
} SHIZRecorder;

static SHIZRecorder _recorder;

static bool _is_recording = false;

bool
z_recorder__init()
{
    _recorder.output = NULL;
    _recorder.buffer = NULL;
    
    sprintf(_recorder.command, "");
    
    return true;
}

bool
z_recorder__kill()
{
    if (z_recorder_is_recording()) {
        z_recorder__stop();
    }
    
    free(_recorder.buffer);
    
    return true;
}

void
z_recorder__setup(SHIZSize const resolution,
                  uint8_t const hz)
{
    SHIZRect clip = z_viewport__get_clip();
    
    uint32_t const input_width = (uint32_t)clip.size.width;
    uint32_t const input_height = (uint32_t)clip.size.height;
    
    uint32_t const output_width = (uint32_t)resolution.width;
    uint32_t const output_height = (uint32_t)resolution.height;
    
    if (_recorder.buffer != NULL) {
        free(_recorder.buffer);
    }

    _recorder.frame_size = input_width * input_height * (sizeof(GLubyte) * 4);
    _recorder.buffer = malloc(_recorder.frame_size);
    
    uint8_t const refresh_rate = hz > 0 ? hz : 60; // default to 60 fps
    
    char const * const output_filename = "output";
    
    char const * const command_format =
    // location of ffmpeg
    "/usr/local/bin/ffmpeg "
    // only output fatal errors
    "-loglevel panic "
    // set refresh rate (should match monitor refresh rate)
    "-r %d "
    // expect raw frame data
    "-f rawvideo "
    // set pixel format (must be RGBA)
    "-pix_fmt rgba "
    // set input frame resolution
    "-s %dx%d "
    // and read frame data from stdin
    "-i - "
    "-threads 0 "
    // set preset (anything goes; balance between file size and performance;
    // faster -> better performance, larger file size,
    // slower -> worse performance, smaller file size)
    "-preset fast "
    // overwrite file without asking
    "-y "
    // use x264 encoding
    "-vcodec libx264rgb "
    // set encoding/compression quality (smaller -> less compression, larger file)
    // (-preset ultrafast -crf 0 works well to disable compression, but filesize will suffer)
    "-crf 23 "
    // flip input vertically and scale down to expected output size
    // note that input and output sizes do not always match (e.g. on retina screens)
    "-vf vflip,scale=%d:%d "
    // set output destination (should be mkv or mp4)
    "%s.mkv";
    
    sprintf(_recorder.command,
            command_format,
            refresh_rate,
            input_width, input_height,
            output_width, output_height,
            output_filename);
}

void
z_recorder__capture()
{
    SHIZRect const clip = z_viewport__get_clip();
    
    GLint const x = (GLint)clip.origin.x;
    GLint const y = (GLint)clip.origin.y;
    
    GLsizei const width = (GLsizei)clip.size.width;
    GLsizei const height = (GLsizei)clip.size.height;
    
    glReadPixels(x, y, width, height,
                 GL_RGBA, GL_UNSIGNED_BYTE,
                 _recorder.buffer);
    
    size_t const capture_frames = 1;
    size_t const captured_frames = fwrite(_recorder.buffer,
                                          _recorder.frame_size,
                                          capture_frames,
                                          _recorder.output);
    
    if (captured_frames != capture_frames) {
        fprintf(stderr, "Could not capture frame");
    }
}

bool
z_recorder__start()
{
    if (_is_recording) {
        return false;
    }
    
    _recorder.output = popen(_recorder.command, "w");
    
    _is_recording = _recorder.output != NULL;
    
    if (!_is_recording) {
        fprintf(stderr, "Unable to start recording; output file inaccessible");
        
        return false;
    } else {
#ifdef SHIZ_DEBUG
        printf("Starting recording... (%s)\n", _recorder.command);
#endif
    }
    
    return true;
}

bool
z_recorder__stop()
{
    if (!_is_recording) {
        return false;
    }
    
    int32_t const status = pclose(_recorder.output);
    
    _is_recording = false;
    
    if (status == -1) {
        fprintf(stderr, "Unable to end recording; output may be corrupted");
        
        return false;
    } else {
#ifdef SHIZ_DEBUG
        printf("Stopped recording\n");
#endif
    }
    
    return true;
}

bool
z_recorder_is_recording()
{
    return _is_recording;
}
