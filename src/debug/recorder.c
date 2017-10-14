////
//    __|  |  | _ _| __  /  __|   \ |
//  \__ \  __ |   |     /   _|   .  |
//  ____/ _| _| ___| ____| ___| _|\_|
//
// Copyright (c) 2017 Jacob Hauberg Hansen
//
// This library is free software; you can redistribute and modify it
// under the terms of the MIT license. See LICENSE for details.
//

#include "recorder.h"

#include <stdlib.h> // NULL
#include <stdio.h> // fwrite, popen, pclose

#include "../internal_type.h"

#include "../viewport.h"
#include "../io.h"

typedef struct SHIZRecorder {
    FILE * output;
    GLubyte * buffer;
    u32 frame_size;
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
                  u8 const hz)
{
    SHIZRect clip = z_viewport__get_clip();
    
    u32 const input_width = (u32)clip.size.width;
    u32 const input_height = (u32)clip.size.height;
    
    u32 const output_width = (u32)resolution.width;
    u32 const output_height = (u32)resolution.height;
    
    if (_recorder.buffer != NULL) {
        free(_recorder.buffer);
    }

    _recorder.frame_size = input_width * input_height * (sizeof(GLubyte) * 4);
    _recorder.buffer = malloc(_recorder.frame_size);
    
    u8 const refresh_rate = hz > 0 ? hz : 60; // default to 60 fps
    
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
        z_io__warning_context("RECORDER", "Could not capture frame");
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
        z_io__error_context("RECORDER", "Unable to start recording; output file inaccessible");
        
        return false;
    } else {
        printf("Starting recording... (%s)\n", _recorder.command);
    }
    
    return true;
}

bool
z_recorder__stop()
{
    if (!_is_recording) {
        return false;
    }
    
    s32 const status = pclose(_recorder.output);
    
    _is_recording = false;
    
    if (status == -1) {
        z_io__error_context("RECORDER", "Unable to end recording; output may be corrupted");
        
        return false;
    } else {
        printf("Stopped recording\n");
    }
    
    return true;
}

bool
z_recorder_is_recording()
{
    return _is_recording;
}
