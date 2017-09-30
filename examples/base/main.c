#include <stdlib.h>

#include <SHIZEN/shizen.h>

int main() {
    SHIZWindowSettings settings = SHIZWindowSettingsDefault; {
        settings.title = "SHIZEN BASE";
        settings.fullscreen = false;
        settings.vsync = true;
        settings.size = SHIZSizeMake(320, 240);
        settings.pixel_size = 2;
    }
    
    if (!z_startup(settings)) {
        exit(EXIT_FAILURE);
    }
    
    u8 const tick_frequency = 30;
    
    while (!z_should_finish()) {
        z_timing_begin(); {
            while (z_time_tick(tick_frequency)) {
                z_input_update();
                
                if (z_input_released(SHIZInputEscape)) {
                    z_request_finish();
                }
            }
        }
        
        f64 const interpolation = z_timing_end();
        
        z_drawing_begin(SHIZColorBlack); {
            // draw stuff
        }
        
        z_drawing_end();
    }
    
    if (!z_shutdown()) {
        exit(EXIT_FAILURE);
    }
    
    return 0;
}
