#include <stdlib.h>
#include <math.h>

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
    
    uint8_t const tick_frequency = 60;
    
    SHIZSize const screen = z_get_display_size();
    
    float const gravity = -9.81f;
    float const platform_friction = 0.86f;
    float const ball_radius = 16;
    
    SHIZVector2 const screen_center = z_get_display_point(SHIZAnchorCenter);
   
    float const one_quarter_screen_height = (screen.height / 4);
    
    SHIZSize const platform_size = SHIZSizeMake(140, 15);
    SHIZRect const platform =
        SHIZRectMake(SHIZVector2Make(screen_center.x - (platform_size.width / 2),
                                     screen_center.y - one_quarter_screen_height),
                     platform_size);
    
    SHIZAnimatableVector2 ball_position =
        SHIZAnimatedVector2(SHIZVector2Make(screen_center.x,
                                            screen_center.y + one_quarter_screen_height));
    
    SHIZAnimatable ball_squish_scale = SHIZAnimated(1);
    
    double const ball_squish_duration = 0.2;
    
    SHIZVector2 ball_velocity = SHIZVector2Zero;
    
    while (!z_should_finish()) {
        z_timing_begin(); {
            while (z_time_tick(tick_frequency)) {
                z_input_update();
                
                if (z_input_released(SHIZInputEscape)) {
                    z_request_finish();
                }
                
                if (z_input_down(SHIZInputUp)) {
                    ball_velocity.y += 35;
                } else if (z_input_down(SHIZInputDown)) {
                    ball_velocity.y -= 35;
                }
                
                ball_velocity.y += gravity;
                
                z_animate_vec2_add(&ball_position, ball_velocity);
                z_animate_to(&ball_squish_scale, 1, ball_squish_duration);
                
                float const platform_top_y = platform.origin.y + platform_size.height;
                float const ball_bottom_y = ball_position.value.y - (ball_radius * ball_squish_scale.result);
                
                if (ball_bottom_y < platform_top_y) {
                    float squish_scale = 1;
                    
                    if (fabsf(ball_velocity.y) > ball_radius * ball_radius) {
                        squish_scale = ball_radius / fabsf(ball_velocity.y);
                        
                        if (squish_scale < 0.6f) {
                            squish_scale = 0.6f;
                        } else if (squish_scale > 1) {
                            squish_scale = 1;
                        }
                    }
                    
                    ball_squish_scale = SHIZAnimated(squish_scale);
                    
                    ball_velocity.y *= platform_friction;
                    ball_velocity.y *= -1;
                    
                    SHIZVector2 clamped_ball_position = ball_position.value;
                    
                    clamped_ball_position.y = platform_top_y + (ball_radius * ball_squish_scale.result);
                    
                    ball_position = SHIZAnimatedVector2(clamped_ball_position);
                }
            }
        }
        
        double const interpolation = z_timing_end();
        
        z_drawing_begin(SHIZColorBlack); {
            SHIZVector2 const position =
                z_animate_vec2_blend(&ball_position, interpolation);
            
            float const squish_scale =
                z_animate_blend(&ball_squish_scale, interpolation);
            
            z_draw_circle_scaled(position,
                                 SHIZColorWhite,
                                 SHIZDrawModeFill,
                                 ball_radius,
                                 20,
                                 SHIZVector2Make(1, squish_scale));
            
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
