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
    char command[200];
    u8 _pad[4];
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
z_recorder__setup(SHIZSize const resolution)
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
    
    sprintf(_recorder.command,
            "/usr/local/bin/ffmpeg -r 60 -f rawvideo -pix_fmt rgba -s %dx%d -i - -threads 0 -preset ultrafast -y -vcodec libx264rgb -crf 0 -vf vflip,scale=%d:%d output.mkv",
            input_width, input_height, output_width, output_height);
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
    }
    
    return true;
}

bool
z_recorder__stop()
{
    if (!_is_recording) {
        return false;
    }
    
    i32 const status = pclose(_recorder.output);
    
    _is_recording = false;
    
    if (status == -1) {
        z_io__error_context("RECORDER", "Unable to end recording; output may be corrupted");
        
        return false;
    }
    
    return true;
}

bool
z_recorder_is_recording()
{
    return _is_recording;
}
