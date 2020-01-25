#pragma once

// internal
#include "common.hpp"
#include "raider.hpp"
#include "platform.hpp"
#include "water.hpp"
#include "spike.hpp"
#include "fallingSpike.hpp"
#include "fireball.hpp"
#include "score_label.hpp"
#include "score_digit.hpp"
#include "backdrop.hpp"
#include "coin.hpp"
#include "screen.hpp"
#include "overlay.hpp"
#include "warning.hpp"
#include "bouncingfireball.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class World
{
public:
    World();
    ~World();
    double radian;

    // Creates a window, sets up events and begins the game
    bool init(vec2 screen);

    // Releases all associated resources
    void destroy();

    // Steps the game ahead by ms milliseconds
    bool update(float ms);

    // Renders our scene
    void draw();

    // Should the game be over ?
    bool is_over()const;

private:
    // Generates a new platform
    bool spawn_platform();
    bool spawn_platform_tar();
    bool spawn_platform_lava();
    bool spawn_platform_ghost();

    // Generate coins
    bool spawn_coin();

    // Generate bouncing fireballs;
    bool spawn_b_fireball();

    bool create_spikeCeiling();
    bool spawn_fallingSpike(vec2 screen, int num);
    vec2 get_random_fallingSpike_position(int num, int i, fallingSpike fs, vec2 screen);

    void save_GameScore(const char* filename);
    void load_GameScore(const char* filename);
    int get_rank_and_saveScore();
    void draw_score(const mat3& projection, float y, unsigned int m_points,
        bool high_score, int high_numb);
    void draw_rank(const mat3& projection, float y, int rank);

    // !!! INPUT CALLBACK FUNCTIONS
    void on_key(GLFWwindow*, int key, int, int action, int mod);
    void on_mouse_move(GLFWwindow* window, double xpos, double ypos);

    bool check_triggerWarning();

    void trigger_deathEvent();

    Bouncingfireball random_b_fireball(Bouncingfireball bouncingfireball, vec2 screen);




private:
    // Window handle
    GLFWwindow* m_window;
    float m_screen_scale; // Screen to pixel coordinates scale factor

    // Screen texture
    // The draw loop first renders to this texture, then it is used for the water shader
    GLuint m_frame_buffer;
    Texture m_screen_tex;

    // Spike texture
    std::vector<Spike> m_spike;
    std::vector<fallingSpike> m_falling_spikes;
    float m_next_fallingspikes_spawn;
    float m_next_fallingspikes_interval;
    float prev_fs_y;
    int fallingSpike_num;


    // Platform texture
    std::vector<Platform> m_platform;
    std::vector<Platform> m_platform_tar;
    std::vector<Platform> m_platform_lava;
    std::vector<Platform> m_platform_ghost;

    // Coin texture
    std::vector<Coin> m_coins;

    // Fireball
    Fireball m_fireball;
    parabolicFireball m_parafireball;
    parabolicFireball m_parafireball2;
    parabolicFireball m_parafireball3;
    std::vector<Bouncingfireball> m_b_fireballs;

    // Warning texture
    Warning m_exclamation;

    // Water effect
    Water m_water;

    // scores for the raider, displayed in the window title
    unsigned int m_points;
    unsigned int highest_scores[5];
    int rank;

    // Game entities
    Raider m_raider;
    bool m_raider_isOnPlatform;
    bool m_raider_isOnTarPlatform;
    bool m_raider_isOnLavaPlatform;
    bool m_raider_isOnGhostPlatform;
    Coin m_coin;
    float score_pos_y;
    Bouncingfireball m_b_fireball;


    Score_Label m_score_label;
    Score_Digit m_score_digit_1;
    Score_Digit m_score_digit_2;
    Score_Digit m_score_digit_3;
    Score_Digit m_score_digit_4;
    gScreen     m_screen;

    gOverlay    m_overlay_esc;
    gOverlay    m_overlay_left;
    gOverlay    m_overlay_space;
    gOverlay    m_overlay_right;

    float m_next_platform_spawn;
    float prev_platform_x;
    Platform prev_platform;


    Mix_Music* m_background_music;
    Mix_Chunk* m_raider_dead_sound;
    Mix_Chunk* m_raider_jump_sound;
    Mix_Chunk* m_raider_land_sound;
    Mix_Chunk* m_raider_coin_sound;
    Mix_Chunk* m_raider_warn_sound;
    Mix_Chunk* m_bfireball_bounce_sound;
    Mix_Chunk* m_spike_hit_sound;
    Mix_Chunk* m_spike_appear_sound;
    Mix_Chunk* m_bfireball_hit_sound;
    Mix_Chunk* m_bfireball_spawn_sound;

    Mix_Chunk* m_raider_land_1_sound;
    Mix_Chunk* m_raider_land_2_sound;
    Mix_Chunk* m_raider_land_3_sound;
    Mix_Chunk* m_raider_land_4_sound;

    Mix_Chunk* m_raider_walk_1_sound;
    Mix_Chunk* m_raider_walk_2_sound;
    Mix_Chunk* m_raider_walk_3_sound;
    Mix_Chunk* m_raider_walk_4_sound;

    Mix_Chunk* m_screen_navigate_sound;
    Mix_Chunk* m_screen_selected_sound;
    Mix_Chunk* m_screen_death_sound;

    void play_land_sound();
    void play_walk_sound();
    bool random_platform(float ms, vec2 screen);



    void game_pause();
    void game_resume();
    bool game_restart();

    Backdrop m_backdrop;
    std::vector<vec2> lightsources;
    float backdrop_speed = 1.f / 8000.f;

    float platform_speed;
    int type;
    bool isRegular;
    bool isTar;
    bool isLava;
    bool isGhost;
    bool isActive;
    bool triggerWarning;

    bool have_new_platform;
    vec2 new_platform_pos;
    vec2 new_platform_bounding_box;
    float m_next_b_fireball_spawn;
    float m_b_fireball_spawn_interval;
    bool has_new_b_fireball;

    // C++ rng
    std::default_random_engine m_rng;
    std::uniform_real_distribution<float> m_dist; // default 0..1
    //std::discrete_distribution<int> m_disc_dist({20, 5, 5}); // default 0..1
};
