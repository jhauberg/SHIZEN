#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SHIZEN/shizen.h>

int main() {
    SHIZWindowSettings settings = SHIZWindowSettingsDefault; {
        settings.title = "SHIZEN BOUNCE";
        settings.fullscreen = false;
        settings.vsync = true;
        settings.size = SHIZSizeMake(320, 240);
        settings.pixel_size = 2;
    }
    
    if (!z_startup(settings)) {
        exit(EXIT_FAILURE);
    }
    
    u8 const tick_frequency = 30;
    
    SHIZSize const screen = z_get_display_size();
    
    f32 const gravity = -9.81f;
    f32 const platform_friction = 0.76f;
    f32 const ball_radius = 12;
    
    SHIZVector2 const screen_center = SHIZVector2Make(screen.width / 2,
                                                      screen.height / 2);
    
    f32 const one_quarter_screen_height = (screen.height / 4);
    
    SHIZSize const platform_size = SHIZSizeMake(100, 10);
    SHIZRect const platform =
        SHIZRectMake(SHIZVector2Make(screen_center.x - (platform_size.width / 2),
                                     screen_center.y - one_quarter_screen_height),
                     platform_size);
    
    SHIZAnimatableVector2 ball_position =
        SHIZAnimatedVector2(SHIZVector2Make(screen_center.x,
                                            screen_center.y + one_quarter_screen_height));
    
    SHIZVector2 ball_velocity = SHIZVector2Zero;
    
    while (!z_should_finish()) {
        z_timing_begin(); {
            while (z_time_tick(tick_frequency)) {
                z_input_update();
                
                if (z_input_released(SHIZInputEscape)) {
                    z_request_finish();
                }
                
                if (z_input_down(SHIZInputUp)) {
                    ball_velocity.y += 50;
                } else if (z_input_down(SHIZInputDown)) {
                    ball_velocity.y -= 50;
                }
                
                ball_velocity.y += gravity;
                
                z_animate_vec2_add(&ball_position, ball_velocity);
                
                f32 const platform_top_y = platform.origin.y + platform_size.height;
                f32 const ball_bottom_y = ball_position.value.y - ball_radius;
                
                if (ball_bottom_y < platform_top_y) {
                    ball_velocity.y *= platform_friction;
                    ball_velocity.y *= -1;
                    
                    SHIZVector2 clamped_ball_position = ball_position.value;
                    
                    clamped_ball_position.y = platform_top_y + ball_radius;
                    
                    ball_position = SHIZAnimatedVector2(clamped_ball_position);
                }
            }
        }
        
        f64 const interpolation = z_timing_end();
        
        z_drawing_begin(SHIZColorBlack); {
            SHIZVector2 const position =
                z_animate_vec2_blend(&ball_position, interpolation);
            
            z_draw_circle(position,
                          SHIZColorWhite,
                          SHIZDrawModeFill,
                          ball_radius,
                          16);
            
            z_draw_rect(platform,
                        SHIZColorWhite,
                        SHIZDrawModeOutline);
        }
        
        z_drawing_end();
    }
    
    if (!z_shutdown()) {
        exit(EXIT_FAILURE);
    }
    
    return 0;
}
