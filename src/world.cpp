// Header
#include "world.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>
#include <ctime>
#include <chrono>
// jsoncpp
// #include <json\json.h>
using namespace std;

bool          keyboard_esc_is_pressed = false;
bool          keyboard_left_is_pressed = false;
bool          keyboard_space_is_pressed = false;
bool          keyboard_right_is_pressed = false;

bool          program_on_title = true;
bool          program_on_game = false;
bool          program_on_death = false;
int           program_which_screen = 0;

bool          game_is_paused = true;

unsigned long paused_start_in_sec = 0;
unsigned long paused_start_in_msec = 0;

unsigned long paused_end_in_sec = 0;
unsigned long paused_end_in_msec = 0;

// the following variables are for use in alans state
bool INITED_LEVEL_1;
bool INITED_LEVEL_2;
bool INITED_LEVEL_3;
bool INITED_LEVEL_4;
bool INITED_LEVEL_5;
bool INITED_LEVEL_6;
bool INITED_LEVEL_7;
bool INITED_LEVEL_8;
bool INITED_LEVEL_9;
bool INITED_LEVEL_10;

unsigned int  score_time;
unsigned int  score_coin;
unsigned int  score_show;

unsigned long timestamp_game_start_in_sec;
unsigned long timestamp_game_start_in_msec;

unsigned long timestamp_last_supported_in_sec;
unsigned long timestamp_last_supported_in_msec;

bool          raider_platform_collide_now;
bool          raider_platform_collide_sound;

bool          raider_warning_now;
bool          raider_warning_sound;

unsigned long timestamp_last_played_land_audio;
unsigned long timestamp_last_played_walk_audio;

float         raider_walk_step_position;
bool          raider_should_die;

// the following variables are for use in alans config
float        walk_step_tolerance = 50.0f;
float        walk_time_tolerance = 250.0f;

unsigned int score_divisor = 750;


float        gravity_minimum           = 01.000f;
float        gravity_maximum           = 30.000f;
float        gravity_change            = 00.015f;

float        score_digit_1_team_offset = -22.5f;
float        score_digit_2_team_offset = -35.0f;
float        score_digit_3_team_offset = -47.5f;
float        score_digit_4_team_offset = -60.0f;

float        score_digit_1_sing_offset = +90.0f;
float        score_digit_2_sing_offset = +117.5f;
float        score_digit_3_sing_offset = +145.0f;
float        score_digit_4_sing_offset = +172.5f;

// Randomization of platform types
std::discrete_distribution<int> m_disc_dist({ 50, 20, 25, 20});
std::discrete_distribution<int> m_dist_tar({ 50, 25});
std::discrete_distribution<int> m_dist_ghost({ 50, 20, 0, 25});


// Same as static in c, local to compilation unit
namespace {
    const size_t MAX_PLATFORM = 10;
    const size_t PLATFORM_DELAY_MS = 900;
    const size_t LVL1_THRESHOLD = 10;
    const size_t LVL2_THRESHOLD = 20;
    const size_t LVL3_THRESHOLD = 30;
    const size_t LVL4_THRESHOLD = 40;
    const size_t LVL5_THRESHOLD = 50;
    const size_t LVL6_THRESHOLD = 60;
    const size_t LVL7_THRESHOLD = 70;
    const size_t LVL8_THRESHOLD = 80;
    const size_t LVL9_THRESHOLD = 90;
    const size_t LVL10_THRESHOLD = 100;
    const size_t STAND_ON_LAVA = 600;


    namespace {
        void glfw_err_cb(int error, const char* desc) {
            fprintf(stderr, "%d: %s", error, desc);
        }
    }
}

// helper logic for timestamp
unsigned long get_timestamp_in_sec() {
    return (unsigned long)std::time(0);
}

unsigned long get_timestamp_in_msec() {
    return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

// helper logic for scoreboard
int get_number_of_digits(int input_num) {
    if (input_num >= 1000) {
        return 4;
    }

    if (input_num >= 100) {
        return 3;
    }

    if (input_num >= 10) {
        return 2;
    }
    else {
        return 1;
    }
}

int get_number_nth_digit(int input_num, int position) {
    while (position--) {
        input_num /= 10;
    }

    return (input_num % 10);
}

World::World() :
    m_points(0),
    m_next_platform_spawn(0.f) {
    // Seeding rng with random device
    m_rng = std::default_random_engine(std::random_device()());
}


World::~World() {

}

// World initialization
bool World::init(vec2 screen) {
    //-------------------------------------------------------------------------
    // GLFW / OGL Initialization
    // Core Opengl 3.
    glfwSetErrorCallback(glfw_err_cb);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, 0);

    m_window = glfwCreateWindow((int)screen.x, (int)screen.y, "Raider Game Assignment", nullptr, nullptr);
    if (m_window == nullptr)
        return false;

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // vsync

    // Load OpenGL function pointers
    gl3w_init();

    // Setting callbacks to member functions (that's why the redirect is needed)
    // Input is handled using GLFW, for more info see
    // http://www.glfw.org/docs/latest/input_guide.html
    glfwSetWindowUserPointer(m_window, this);
    auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) {
        ((World*)glfwGetWindowUserPointer(wnd))->on_key(wnd, _0, _1, _2, _3);
    };
    auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) {
        ((World*)glfwGetWindowUserPointer(wnd))->on_mouse_move(wnd, _0, _1);
    };
    glfwSetKeyCallback(m_window, key_redirect);
    glfwSetCursorPosCallback(m_window, cursor_pos_redirect);

    // Create a frame buffer
    m_frame_buffer = 0;
    glGenFramebuffers(1, &m_frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

    // For some high DPI displays (ex. Retina Display on Macbooks)
    // https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
    int fb_width, fb_height;
    glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
    m_screen_scale = static_cast<float>(fb_width) / screen.x;

    // Initialize the screen texture
    m_screen_tex.create_from_screen(m_window);

    //-------------------------------------------------------------------------
    // Loading music and sounds
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    m_background_music = Mix_LoadMUS(audio_path("custom_music.wav"));
    m_raider_dead_sound = Mix_LoadWAV(audio_path("custom_dead.wav"));
    m_raider_jump_sound = Mix_LoadWAV(audio_path("custom_jump.wav"));
    m_raider_land_sound = Mix_LoadWAV(audio_path("custom_land.wav"));
    m_raider_coin_sound = Mix_LoadWAV(audio_path("custom_coin.wav"));
    m_raider_warn_sound = Mix_LoadWAV(audio_path("custom_warn.wav"));

    //TODO: need to change the sound for following
    m_bfireball_bounce_sound = Mix_LoadWAV(audio_path("fireball_bounce.wav"));
    m_bfireball_hit_sound    = Mix_LoadWAV(audio_path("fireball_hit.wav"));
    m_spike_hit_sound        = Mix_LoadWAV(audio_path("spike_hit.wav"));
    m_spike_appear_sound     = Mix_LoadWAV(audio_path("spike_fall.wav"));
    m_bfireball_spawn_sound  = Mix_LoadWAV(audio_path("fireball_fall.wav"));


    m_raider_land_1_sound = Mix_LoadWAV(audio_path("custom_land_1.wav"));
    m_raider_land_2_sound = Mix_LoadWAV(audio_path("custom_land_2.wav"));
    m_raider_land_3_sound = Mix_LoadWAV(audio_path("custom_land_3.wav"));
    m_raider_land_4_sound = Mix_LoadWAV(audio_path("custom_land_4.wav"));

    m_raider_walk_1_sound = Mix_LoadWAV(audio_path("custom_walk_1.wav"));
    m_raider_walk_2_sound = Mix_LoadWAV(audio_path("custom_walk_2.wav"));
    m_raider_walk_3_sound = Mix_LoadWAV(audio_path("custom_walk_3.wav"));
    m_raider_walk_4_sound = Mix_LoadWAV(audio_path("custom_walk_4.wav"));

    m_screen_navigate_sound = Mix_LoadWAV(audio_path("title_screen_navigate.wav"));
    m_screen_selected_sound = Mix_LoadWAV(audio_path("title_screen_selected.wav"));
    m_screen_death_sound    = Mix_LoadWAV(audio_path("title_screen_death.wav"));

    Mix_VolumeChunk(m_raider_coin_sound,      32);
    Mix_VolumeChunk(m_raider_warn_sound,      32);

    Mix_VolumeChunk(m_spike_hit_sound,        64);
    Mix_VolumeChunk(m_spike_appear_sound,     32);

    Mix_VolumeChunk(m_bfireball_bounce_sound, 64);
    Mix_VolumeChunk(m_bfireball_hit_sound,    64);
    Mix_VolumeChunk(m_bfireball_spawn_sound,  32);


    Mix_VolumeChunk(m_raider_land_1_sound, 72);
    Mix_VolumeChunk(m_raider_land_2_sound, 72);
    Mix_VolumeChunk(m_raider_land_3_sound, 72);
    Mix_VolumeChunk(m_raider_land_4_sound, 72);

    Mix_VolumeChunk(m_raider_walk_1_sound, 72);
    Mix_VolumeChunk(m_raider_walk_2_sound, 72);
    Mix_VolumeChunk(m_raider_walk_3_sound, 72);
    Mix_VolumeChunk(m_raider_walk_4_sound, 72);

    Mix_VolumeChunk(m_screen_navigate_sound, 32);
    Mix_VolumeChunk(m_screen_selected_sound, 32);
    Mix_VolumeChunk(m_screen_death_sound,    32);

    if (m_background_music == nullptr ||
        m_raider_dead_sound == nullptr ||
        m_raider_jump_sound == nullptr ||
        m_raider_land_sound == nullptr ||
        m_raider_coin_sound == nullptr ||
        m_raider_warn_sound == nullptr ||
        m_bfireball_bounce_sound == nullptr ||
        m_spike_hit_sound == nullptr ||
        m_spike_appear_sound == nullptr ||
        m_bfireball_hit_sound == nullptr ||
        m_bfireball_spawn_sound == nullptr ||

        m_raider_land_1_sound == nullptr ||
        m_raider_land_2_sound == nullptr ||
        m_raider_land_3_sound == nullptr ||
        m_raider_land_4_sound == nullptr ||

        m_raider_walk_1_sound == nullptr ||
        m_raider_walk_2_sound == nullptr ||
        m_raider_walk_3_sound == nullptr ||
        m_raider_walk_4_sound == nullptr ||

        m_screen_navigate_sound == nullptr ||
        m_screen_selected_sound == nullptr ||
        m_screen_death_sound    == nullptr)
    {
        fprintf(stderr, "Failed to load sounds");

        return false;
    }

    // Playing background music indefinitely
    if (!game_is_paused) {
        Mix_PlayMusic(m_background_music, -1);
    }

    fprintf(stderr, "Loaded music\n");


    // the following is state initialization #1
    INITED_LEVEL_1  = false;
    INITED_LEVEL_2  = false;
    INITED_LEVEL_3  = false;
    INITED_LEVEL_4  = false;
    INITED_LEVEL_5  = false;
    INITED_LEVEL_6  = false;
    INITED_LEVEL_7  = false;
    INITED_LEVEL_8  = false;
    INITED_LEVEL_9  = false;
    INITED_LEVEL_10 = false;

    paused_start_in_sec              = get_timestamp_in_sec();
    paused_start_in_msec             = get_timestamp_in_msec();

    score_time                       = 0;
    score_coin                       = 0;
    score_show                       = 0;

    timestamp_game_start_in_sec      = get_timestamp_in_sec();
    timestamp_game_start_in_msec     = get_timestamp_in_msec();

    timestamp_last_supported_in_sec  = get_timestamp_in_sec();
    timestamp_last_supported_in_msec = get_timestamp_in_msec();

    raider_platform_collide_now      = false;
    raider_platform_collide_sound    = false;

    raider_warning_now               = false;
    raider_warning_sound             = false;

    timestamp_last_played_land_audio = get_timestamp_in_msec();
    timestamp_last_played_walk_audio = get_timestamp_in_msec();

    raider_walk_step_position        = 300.0f;
    raider_should_die                = false;

    m_next_b_fireball_spawn       = 15000.f;
    m_b_fireball_spawn_interval   = 15000.f;
    m_next_fallingspikes_spawn    = 6000000.f;
    m_next_fallingspikes_interval = 6000000.f;
    fallingSpike_num              = 4;

    triggerWarning = false;

    //load highest score
    load_GameScore(status_path("scores.txt"));
    //load_GameScore(status_path("scores.json"));
    score_pos_y = 125.f;

    // initialize a platform
    if (!spawn_platform())
        return false;
    Platform& new_platform = m_platform.back();
    new_platform.set_position({ 300.f, 600.f });
    platform_speed = 0.6f;
    isRegular = true;
    isTar = false;
    isLava = false;
    isGhost = false;
    new_platform.type = 0;
    has_new_b_fireball = false;

    return (m_raider.init() &&
        m_water.init() &&
        create_spikeCeiling() &&

        m_score_label.init() &&
        m_score_digit_1.init() &&
        m_score_digit_2.init() &&
        m_score_digit_3.init() &&
        m_score_digit_4.init() &&
        m_screen.init() &&

        m_overlay_esc.init("overlay_key_esc_on.png", "overlay_key_esc_off.png") &&
        m_overlay_left.init("overlay_key_left_on.png", "overlay_key_left_off.png") &&
        m_overlay_space.init("overlay_key_space_on.png", "overlay_key_space_off.png") &&
        m_overlay_right.init("overlay_key_right_on.png", "overlay_key_right_off.png") &&

        spawn_fallingSpike(screen, fallingSpike_num) &&
        m_fireball.init() &&
        m_parafireball.init() &&
        m_parafireball2.init() &&
        m_parafireball3.init() &&
        m_backdrop.init() &&
        m_coin.init() &&
        m_exclamation.init());
}

// Releases all the associated resources
void World::destroy() {
    save_GameScore(status_path("scores.txt"));
    //save_GameScore(status_path("scores.json"));
    glDeleteFramebuffers(1, &m_frame_buffer);

    if (m_background_music != nullptr)
        Mix_FreeMusic(m_background_music);

    Mix_CloseAudio();

    m_raider.destroy();

    m_score_label.destroy();
    m_score_digit_1.destroy();
    m_score_digit_2.destroy();
    m_score_digit_3.destroy();
    m_score_digit_4.destroy();
    m_screen.destroy();

    m_overlay_esc.destroy();
    m_overlay_left.destroy();
    m_overlay_space.destroy();
    m_overlay_right.destroy();

    for (auto& spike : m_spike)
        spike.destroy();
    m_spike.clear();
    for (auto& fs : m_falling_spikes)
        fs.destroy();
    for (auto& coin : m_coins)
        coin.destroy();
    for (auto& platform : m_platform)
        platform.destroy();
    for (auto& fireball : m_b_fireballs)
        fireball.destroy();
    m_b_fireballs.clear();
    m_platform.clear();
    m_backdrop.destroy();
    m_exclamation.destroy();
    m_b_fireball.destroy();
    glfwDestroyWindow(m_window);
}

bool World::update(float elapsed_ms) {
    if (game_is_paused) {
        return true;
    }

    if (m_raider.is_alive() && raider_should_die)
    {
        raider_should_die = false;

        Mix_PlayChannel(-1, m_spike_hit_sound, 0);

        trigger_deathEvent();
    }

    if (m_raider.is_alive())
    {
        score_time = (get_timestamp_in_msec() - timestamp_game_start_in_msec) / score_divisor;
        score_show = score_time + score_coin;

        m_points   = score_show;
    }

    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    vec2 screen = { (float)w / m_screen_scale, (float)h / m_screen_scale };

    if (m_raider_isOnLavaPlatform && raider_platform_collide_now) {
        m_raider.time_spent_on_lava += elapsed_ms;
        if (m_raider.time_spent_on_lava > 1200) {
            trigger_deathEvent();
        }
    }
    else {
        m_raider.time_spent_on_lava = 0;
    }

    if (m_raider_isOnTarPlatform) {
        Mix_PlayChannel(-1, m_raider_jump_sound, 0);
        m_raider.m_jump = true;
        m_raider.move({ 0.f, -6.0f - platform_speed * 2 });
        m_raider.y_velocity = m_raider.jump_speed * -1.0f;
    }

    m_raider_isOnPlatform = false;
    m_raider_isOnTarPlatform = false;
    m_raider_isOnLavaPlatform = false;
    m_raider_isOnGhostPlatform = false;
    m_raider.jump_speed = 10.f;

    for (auto& platform : m_platform) {
        while (m_raider.on_top_of(platform) && m_raider.is_alive()) {
            vec2 dirc = { 0.f,
                         m_raider.get_position().y - platform.get_position().y };
            vec2 norm_dirc = normalize(dirc);
            m_raider.move(norm_dirc);

            m_raider_isOnPlatform = true;
            if (platform.type == 1) {
                m_raider_isOnTarPlatform = true;
            }
            if (platform.type == 2) {
                m_raider_isOnLavaPlatform = true;
            }

            if (platform.type == 3) {
                m_raider_isOnGhostPlatform = true;
                platform.used = true;
            }

            timestamp_last_supported_in_sec = get_timestamp_in_sec();
            timestamp_last_supported_in_msec = get_timestamp_in_msec();
        }
    }

    if (m_raider_isOnPlatform) {
        m_raider.m_is_onPlatform = true;
    }
    else
    {
        m_raider.m_is_onPlatform = false;
    }

    if (m_raider_isOnPlatform)
    {
        raider_platform_collide_now = true;
        raider_platform_collide_sound = raider_platform_collide_sound;
        m_raider.m_jump = false;
    }
    else
    {
        raider_platform_collide_now = false;
        raider_platform_collide_sound = false;
        m_raider.m_jump = true;
    }

    if (raider_platform_collide_now && !raider_platform_collide_sound)
    {
        play_land_sound();

        raider_platform_collide_sound = true;

        // the following logic is for walking sounds
        raider_walk_step_position = m_raider.get_position().x;
    }

    if (!m_raider_isOnPlatform) {
        unsigned long delta_time;

        delta_time = get_timestamp_in_msec();
        delta_time = delta_time - timestamp_last_supported_in_msec;

        float delta_gravity;

        delta_gravity = delta_time * gravity_change;
        delta_gravity = delta_gravity + gravity_minimum;
        delta_gravity = fminf(delta_gravity, gravity_maximum);

        m_raider.move(vec2{ 0.f, delta_gravity });
    }
    else {
        m_raider.y_velocity = 0;
    }

    // Checking raider - Spike collisions
    // Needs to be implemented in the Raider class first
    for (const auto& spike : m_spike) {
        if (m_raider.collides_with(spike)) {
            if (m_raider.is_alive()) {
                trigger_deathEvent();
            }
            break;
        }
    }


    for (auto& fs : m_falling_spikes)
    {
        //play sound when spikes appear
        float y_current = fs.get_position().y;

        float y_minimum = 000.0f;
        float y_maximum = 800.0f;

        if (y_current > y_minimum &&
            y_current < y_maximum &&
            !fs.played_falling_sound)
        {
            Mix_PlayChannel(-1, m_spike_appear_sound, 0);

            fs.played_falling_sound = true;
        }

        if (y_current <= y_minimum ||
            y_current >= y_maximum)
        {
            fs.played_falling_sound = false;
        }

        // Checking raider - falling spike collisions
        // if (m_raider.collides_with(fs))
        // {
        //     if (m_raider.is_alive())
        //     {
        //         Mix_PlayChannel(-1, m_spike_hit_sound, 0);
        //         //trigger_deathEvent();
        //     }
        //     break;
        // }
    }

    if (m_raider.is_alive() && (
        m_raider.collides_with(m_fireball) ||
        m_raider.collides_with(m_parafireball) ||
        m_raider.collides_with(m_parafireball2) ||
        m_raider.collides_with(m_parafireball3))) {
        trigger_deathEvent();
    }

    for (const auto& b_fireball : m_b_fireballs) {
        if (m_raider.is_alive() && m_raider.collides_with(b_fireball)) {
            Mix_PlayChannel(-1, m_bfireball_hit_sound, 0);
            trigger_deathEvent();
        }
    }

    // fireball bouncing back when hit the wall except the bottom
    for (auto& b_fireball : m_b_fireballs) {
        if (b_fireball.get_position().x - 0.5f * b_fireball.get_bounding_box().x <= 20.f) {
            while (b_fireball.get_position().x - 0.5f * b_fireball.get_bounding_box().x <= 20.f) {
                b_fireball.move({ 1.f,0.f });
            }
            b_fireball.set_velocity({ -b_fireball.get_velocity().x, b_fireball.get_velocity().y });
            if (m_raider.is_alive()) {
                Mix_PlayChannel(-1, m_bfireball_bounce_sound, 0);
            }
        }
        else if (b_fireball.get_position().x + 0.5f * b_fireball.get_bounding_box().x >= screen.x - 20.f) {
            while (b_fireball.get_position().x + 0.5f * b_fireball.get_bounding_box().x >= screen.x - 20.f) {
                b_fireball.move({ -1.f,0.f });
            }
            b_fireball.set_velocity({ -b_fireball.get_velocity().x, b_fireball.get_velocity().y });
            if (m_raider.is_alive()) {
                Mix_PlayChannel(-1, m_bfireball_bounce_sound, 0);
            }
        }
    }




    int count = (int)m_coins.size();
    for (const auto& coin : m_coins) {
        if (m_raider.is_alive() && m_raider.collides_with(coin)) {
            Mix_PlayChannel(-1, m_raider_coin_sound, 0);
            m_coins.erase(m_coins.end() - count);

            score_coin = score_coin + 1;

            // m_raider.coin_current++;
            // m_raider.coin_collected_so_far++;
        }
        --count;
    }

    // Updating all entities, making the turtle
    // faster based on current.
    // In a pure ECS engine we would classify entities by their bitmap tags during the update loop
    // rather than by their class.
    m_raider.update(elapsed_ms, screen);
    m_b_fireball.update(elapsed_ms);

    int score_time_modded = score_time % 100;

    if (highest_scores[1] == 27)
    {
        if (!INITED_LEVEL_1)
        {
            // the following is hell
            m_next_fallingspikes_spawn    = 2000.f;
            m_next_fallingspikes_interval = 200.f;
            m_next_b_fireball_spawn       = 1000.f;
            m_b_fireball_spawn_interval   = 100.f;

            isRegular = true;
            isTar     = true;
            isLava    = true;
            isGhost   = true;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_1  = true;
        }
    }
    else if (score_time <= LVL1_THRESHOLD)
    {
        if (!INITED_LEVEL_1)
        {
            // the following is normal
            m_next_fallingspikes_spawn    = 5000000.f;
            m_next_fallingspikes_interval = 5000000.f;
            m_next_b_fireball_spawn       = 5000000.f;
            m_b_fireball_spawn_interval   = 5000000.f;

            isRegular = true;
            isTar     = false;
            isLava    = false;
            isGhost   = false;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_1  = true;
        }
    }
    else if (score_time_modded > LVL1_THRESHOLD && score_time_modded <= LVL2_THRESHOLD)
    {
        platform_speed      += 0.00015f;
        m_raider.jump_speed += 0.00015f;

        if (!INITED_LEVEL_2)
        {
            // the following is spikes
            m_next_fallingspikes_spawn    = 100.f;
            m_next_fallingspikes_interval = 4000.f;
            m_next_b_fireball_spawn       = 5000000.f;
            m_b_fireball_spawn_interval   = 5000000.f;

            isRegular = true;
            isTar     = false;
            isLava    = false;
            isGhost   = false;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_2  = true;
        }
    }
    else if (score_time_modded > LVL2_THRESHOLD && score_time_modded <= LVL3_THRESHOLD)
    {
        if (!INITED_LEVEL_3)
        {
            // the following is platforms
            m_next_fallingspikes_spawn    = 5000000.f;
            m_next_fallingspikes_interval = 5000000.f;
            m_next_b_fireball_spawn       = 5000000.f;
            m_b_fireball_spawn_interval   = 5000000.f;

            isRegular = true;
            isTar     = true;
            isLava    = true;
            isGhost   = true;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_3  = true;
        }
    }
    else if (score_time_modded > LVL3_THRESHOLD && score_time_modded <= LVL4_THRESHOLD)
    {
        if (!INITED_LEVEL_4)
        {
            // the following is fireballs
            m_next_fallingspikes_spawn    = 5000000.f;
            m_next_fallingspikes_interval = 5000000.f;
            m_next_b_fireball_spawn       = 100.f;
            m_b_fireball_spawn_interval   = 2000.f;

            isRegular = true;
            isTar     = false;
            isLava    = false;
            isGhost   = false;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_4  = true;
        }
    }
    else if (score_time_modded > LVL4_THRESHOLD && score_time_modded <= LVL5_THRESHOLD)
    {
        if (!INITED_LEVEL_5)
        {
            // the following is platforms
            m_next_fallingspikes_spawn    = 5000000.f;
            m_next_fallingspikes_interval = 5000000.f;
            m_next_b_fireball_spawn       = 5000000.f;
            m_b_fireball_spawn_interval   = 5000000.f;

            isRegular = true;
            isTar     = true;
            isLava    = true;
            isGhost   = true;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_5  = true;
        }
    }
    else if (score_time_modded > LVL5_THRESHOLD && score_time_modded <= LVL6_THRESHOLD)
    {
        platform_speed      += 0.00015f;
        m_raider.jump_speed += 0.00015f;

        if (!INITED_LEVEL_6)
        {
            // the following is spikes
            m_next_fallingspikes_spawn    = 100.f;
            m_next_fallingspikes_interval = 4000.f;
            m_next_b_fireball_spawn       = 5000000.f;
            m_b_fireball_spawn_interval   = 5000000.f;

            isRegular = true;
            isTar     = false;
            isLava    = false;
            isGhost   = false;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_6  = true;
        }
    }
    else if (score_time_modded > LVL6_THRESHOLD && score_time_modded <= LVL7_THRESHOLD)
    {
        if (!INITED_LEVEL_7)
        {
            // the following is platforms
            m_next_fallingspikes_spawn    = 5000000.f;
            m_next_fallingspikes_interval = 5000000.f;
            m_next_b_fireball_spawn       = 5000000.f;
            m_b_fireball_spawn_interval   = 5000000.f;

            isRegular = true;
            isTar     = true;
            isLava    = true;
            isGhost   = true;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_7  = true;
        }
    }
    else if (score_time_modded > LVL7_THRESHOLD && score_time_modded <= LVL8_THRESHOLD)
    {
        if (!INITED_LEVEL_8)
        {
            // the following is fireballs
            m_next_fallingspikes_spawn    = 5000000.f;
            m_next_fallingspikes_interval = 5000000.f;
            m_next_b_fireball_spawn       = 100.f;
            m_b_fireball_spawn_interval   = 2000.f;

            isRegular = true;
            isTar     = false;
            isLava    = false;
            isGhost   = false;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_8  = true;
        }
    }
    else if (score_time_modded > LVL8_THRESHOLD && score_time_modded <= LVL9_THRESHOLD)
    {
        if (!INITED_LEVEL_9)
        {
            // the following is platforms
            m_next_fallingspikes_spawn    = 5000000.f;
            m_next_fallingspikes_interval = 5000000.f;
            m_next_b_fireball_spawn       = 5000000.f;
            m_b_fireball_spawn_interval   = 5000000.f;

            isRegular = true;
            isTar     = true;
            isLava    = true;
            isGhost   = true;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_9  = true;
        }
    }
    else if (score_time_modded > LVL9_THRESHOLD && score_time_modded <= LVL10_THRESHOLD)
    {
        platform_speed      += 0.00015f;
        m_raider.jump_speed += 0.00015f;

        if (!INITED_LEVEL_10)
        {
            // the following is spikes
            m_next_fallingspikes_spawn    = 100.f;
            m_next_fallingspikes_interval = 4000.f;
            m_next_b_fireball_spawn       = 5000000.f;
            m_b_fireball_spawn_interval   = 5000000.f;

            isRegular = true;
            isTar     = false;
            isLava    = false;
            isGhost   = false;

            INITED_LEVEL_1  = false;
            INITED_LEVEL_2  = false;
            INITED_LEVEL_3  = false;
            INITED_LEVEL_4  = false;
            INITED_LEVEL_5  = false;
            INITED_LEVEL_6  = false;
            INITED_LEVEL_7  = false;
            INITED_LEVEL_8  = false;
            INITED_LEVEL_9  = false;
            INITED_LEVEL_10 = false;

            INITED_LEVEL_10 = true;
        }
    }

    if (score_time > LVL10_THRESHOLD)
    {
        platform_speed      += 0.00015f;
        m_raider.jump_speed += 0.00015f;
    }

    for (auto& platform : m_platform)
    {
        platform.update(elapsed_ms * platform_speed);
        platform.lightsource = m_raider.get_position();
        platform.illumination = min(200 / len(sub(m_raider.get_position(), platform.get_position())), 1.0f);
    }
    for (auto& coin : m_coins)
    {
        coin.update(elapsed_ms * platform_speed);
        coin.illumination = min(200 / len(sub(m_raider.get_position(), coin.get_position())), 1.0f);
    }
    for (auto& fireball : m_b_fireballs)
    {
        fireball.update(elapsed_ms);
    }

    //update falling_spikes
    int i = 0;
    int num = m_falling_spikes.size();
    for (auto& fs : m_falling_spikes) {
        //setting position
        if (fs.get_position().y > 1000.f) {
            fs.set_isFalling(false);
            fs.set_speed(0.f);
            vec2 pos = get_random_fallingSpike_position(num, i, fs, screen);
            prev_fs_y = pos.y;
            fs.set_position(pos);
        }
        fs.update(elapsed_ms);
        fs.illumination =  min(200 / len(sub(m_raider.get_position(), fs.get_position())), 1.0f);
        i++;
    }

    // Removing out of screen bouncingfireballs
    auto fireball_it = m_b_fireballs.begin();
    while (fireball_it != m_b_fireballs.end()) {
        float h = fireball_it->get_bounding_box().y / 2;
        float w = fireball_it->get_bounding_box().x * 0.8f;
        if (fireball_it->get_position().y - h > screen.y) {
            fireball_it = m_b_fireballs.erase(fireball_it);
            continue;
        }
        ++fireball_it;
    }

    // Spawning new bouncing fireballs
    m_next_b_fireball_spawn -= elapsed_ms;
    if (m_next_b_fireball_spawn <= 0) {
        if(spawn_b_fireball()) {
            Bouncingfireball& new_b_fireball = m_b_fireballs.back();
            new_b_fireball = random_b_fireball(new_b_fireball,screen);
            float dis = m_dist(m_rng);
            m_next_b_fireball_spawn = m_b_fireball_spawn_interval;
            has_new_b_fireball = true;
            Mix_PlayChannel(-1, m_bfireball_spawn_sound, 0);
        } else {
            return false;
        }
    } else {
        has_new_b_fireball = false;
    }

    //update warning status
    vec2 exclamation_pos = add(m_raider.get_position(), { 0, -85.f });
    m_exclamation.update(elapsed_ms, exclamation_pos);

    if (check_triggerWarning())
    {
        raider_warning_now = true;
        raider_warning_sound = raider_warning_sound;
        triggerWarning = true;
    }
    else
    {
        raider_warning_now = false;
        raider_warning_sound = false;
        triggerWarning = false;
    }

    if (raider_warning_now && !raider_warning_sound && m_raider.is_alive())
    {
        Mix_PlayChannel(-1, m_raider_warn_sound, 0);

        raider_warning_sound = true;
    }

    // Removing out of screen platform
    auto platform_it = m_platform.begin();
    platform_it = m_platform.begin();
    while (platform_it != m_platform.end()) {
        float h = platform_it->get_bounding_box().y / 2;
        float w = platform_it->get_bounding_box().x * 0.8f;
        if (platform_it->alpha <= 0.f) {
            platform_it = m_platform.erase(platform_it);
            isActive = false;

            continue;
        }
        if (platform_it->get_position().y + h < 0.f) {
            platform_it = m_platform.erase(platform_it);
            isActive = false;

            continue;
        }
        ++platform_it;
    }

    // Spawning new platform
    random_platform(elapsed_ms, screen);

    // Removing out of screen coins
    auto coin_it = m_coins.begin();
    while (coin_it != m_coins.end()) {
        float h = coin_it->get_bounding_box().y / 2;
        float w = coin_it->get_bounding_box().x * 0.8f;
        if (coin_it->get_position().y + h < 0.f) {
            coin_it = m_coins.erase(coin_it);

            continue;
        }
        ++coin_it;
    }

    // Spawning new coins
    if (have_new_platform) {
        float coin_appear_prob = m_dist(m_rng);
        if (coin_appear_prob > 0.4) {
            for (int i = 0; i < 3; ++i) {
                if (!spawn_coin())
                    return false;
                Coin& new_coin = m_coins.back();
                vec2 dim = new_coin.get_bounding_box();
                new_coin.set_position({ new_platform_pos.x + (i - 1) * 40,
                                       new_platform_pos.y - (dim.y / 2 + new_platform_bounding_box.y / 2) });
            }
        }
    }

    // falling spikes begin to fall
    m_next_fallingspikes_spawn -= elapsed_ms;
    if (m_next_fallingspikes_spawn < 0) {
        for (auto& fs : m_falling_spikes) {
            fs.set_isFalling(true);
        }
        m_next_fallingspikes_spawn = m_next_fallingspikes_interval;
    }

    // Check if raider has fall to the bottom of the screen
    if (m_raider.get_position().y > m_screen_tex.height) {
        if (m_raider.is_alive()) {
            Mix_PlayChannel(-1, m_raider_dead_sound, 0);
            m_water.set_raider_dead();
        }
        m_raider.kill();
    }


    // If raider is dead, restart the game after the fading animation
    if (!m_raider.is_alive() && m_water.get_raider_dead_time() > 1.5f)
    {
        Mix_PlayChannel(-1, m_screen_death_sound, 0);
        program_which_screen = 6;

        program_on_title = false;
        program_on_game  = false;
        program_on_death = true;

        game_restart();
        game_pause();
    }

    m_backdrop.update(elapsed_ms * backdrop_speed);
    m_backdrop.lightsource = m_raider.get_position();

    //Todo: need to decide how these fireballs and bouncing fireball are working together, it looks too much when
    // appear at the same time.
    //m_fireball.update(elapsed_ms, sub(m_raider.get_position(), m_fireball.get_position()));
    //m_parafireball.update(elapsed_ms);

    // the following logic is for walking sounds
    float old_position = raider_walk_step_position;
    float new_position = m_raider.get_position().x;

    bool condition_1 = fabs(old_position - new_position) > walk_step_tolerance;
    bool condition_2 = (get_timestamp_in_msec() - timestamp_last_played_land_audio) > walk_time_tolerance;

    if (condition_1 && condition_2)
    {
        raider_walk_step_position = m_raider.get_position().x;

        if (m_raider_isOnPlatform)
        {
            play_walk_sound();
        }
    }

    return true;
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void World::draw() {
    // Clearing error buffer
    gl_flush_errors();

    // Getting size of window
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);

    // Updating window title with points
    std::stringstream title_ss;
    title_ss << "Temple Fall";
    glfwSetWindowTitle(m_window, title_ss.str().c_str());

    /////////////////////////////////////
    // First render to the custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

    // Clearing backbuffer
    glViewport(0, 0, w, h);
    glDepthRange(0.00001, 10);
    const float clear_color[3] = { 0.f, 0.f, 0.f };
    glClearColor(clear_color[0], clear_color[1], clear_color[2], 1.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Fake projection matrix, scales with respect to window coordinates
    // PS: 1.f / w in [1][1] is correct.. do you know why ? (:
    float left = 0.f;// *-0.5;
    float top = 0.f;// (float)h * -0.5;
    float right = (float)w / m_screen_scale;// *0.5;
    float bottom = (float)h / m_screen_scale;// *0.5;

    float sx = 2.f / (right - left);
    float sy = 2.f / (top - bottom);
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    mat3 projection_2D{ {sx, 0.f, 0.f},
                       {0.f, sy, 0.f},
                       {tx, ty,  1.f} };



    // the following drawing calls are for program_on_game
    if (program_on_game) {
        m_backdrop.draw(projection_2D);

        for (auto& platform : m_platform) {
            platform.draw(projection_2D);
        }
        for (auto& coin : m_coins) {
            coin.draw(projection_2D);
        }

        m_raider.draw(projection_2D);

        if (triggerWarning && m_raider.is_alive())
        {
            m_exclamation.draw(projection_2D);
        }

        for (auto& fs : m_falling_spikes)
            fs.draw(projection_2D);

         //m_fireball.draw(projection_2D);

        m_parafireball.draw(projection_2D);

        m_parafireball2.draw(projection_2D);
        m_parafireball3.draw(projection_2D);
        m_b_fireball.draw(projection_2D);
        for (auto& fireball : m_b_fireballs) {
            fireball.draw(projection_2D);
        }

        for (auto& spike : m_spike) {
            spike.draw(projection_2D);
        }

        draw_score(projection_2D, score_pos_y, m_points, false, 0);

        m_overlay_esc.displayed_on = keyboard_esc_is_pressed;
        m_overlay_left.displayed_on = keyboard_left_is_pressed;
        m_overlay_space.displayed_on = keyboard_space_is_pressed;
        m_overlay_right.displayed_on = keyboard_right_is_pressed;

        m_overlay_esc.draw(projection_2D);
        m_overlay_left.draw(projection_2D);
        m_overlay_space.draw(projection_2D);
        m_overlay_right.draw(projection_2D);
    }

    for (auto& fs : m_falling_spikes)
    {
        if (m_raider.collides_with(fs) && fs.collides_exact_with(m_raider, projection_2D))
        {
            raider_should_die = true;
        }
    }

    // the following drawing calls are for program_on_title
    if (program_on_title)
    {
        m_screen.displayed_screen = program_which_screen;
        m_screen.draw(projection_2D);

        if (m_screen.displayed_screen == 5)
        {
            float height = 275.f;
            float offset = 1;

            for (auto score : highest_scores)
            {
                draw_score(projection_2D, height, score, true, offset);
                height += 50.f;
                offset += 1;
            }
        }
    }

    if (program_on_death) {
        m_screen.displayed_screen = program_which_screen;
        m_screen.draw(projection_2D);
        float height = 425.f;
        draw_score(projection_2D, height, m_points, false, 1);
        draw_rank(projection_2D, height + 50.f, rank);
    }

    /////////////////////
    // Truely render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clearing backbuffer
    glViewport(0, 0, w, h);
    glDepthRange(0, 10);
    glClearColor(0, 0, 0, 1.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_screen_tex.id);

    m_water.draw(projection_2D);

    //////////////////
    // Presenting
    glfwSwapBuffers(m_window);
}

// Should the game be over ?
bool World::is_over() const {
    return glfwWindowShouldClose(m_window);
}


void World::on_key(GLFWwindow*, int key, int, int action, int mod) {
    // HANDLE raider MOVEMENT HERE key is of 'type' GLFW_KEY_
    // action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT

    // the following keyboard input is for overlay
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            keyboard_esc_is_pressed = true;
            break;

        case GLFW_KEY_LEFT:
            keyboard_left_is_pressed = true;
            break;

        case GLFW_KEY_SPACE:
            keyboard_space_is_pressed = true;
            break;

        case GLFW_KEY_RIGHT:
            keyboard_right_is_pressed = true;
            break;

        default:
            break;
        }
    }
    else if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            keyboard_esc_is_pressed = false;
            break;

        case GLFW_KEY_LEFT:
            keyboard_left_is_pressed = false;
            break;

        case GLFW_KEY_SPACE:
            keyboard_space_is_pressed = false;
            break;

        case GLFW_KEY_RIGHT:
            keyboard_right_is_pressed = false;
            break;

        default:
            break;
        }
    }

    // the following keyboard input is for program_on_title
    if (program_on_title && action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_UP:
            if (program_which_screen == 0) // main menu (start)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 0;
            }
            else if (program_which_screen == 1) // main menu (lore)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 0;
            }
            else if (program_which_screen == 2) // main menu (score)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 1;
            }
            else if (program_which_screen == 3) // main menu (exit)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 2;
            }
            else if (program_which_screen == 4) // lore section (return)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 4;
            }
            else if (program_which_screen == 5) // score section (return)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 5;
            }

            break;

        case GLFW_KEY_DOWN:
            if (program_which_screen == 0) // main menu (start)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 1;
            }
            else if (program_which_screen == 1) // main menu (lore)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 2;
            }
            else if (program_which_screen == 2) // main menu (score)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 3;
            }
            else if (program_which_screen == 3) // main menu (exit)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 3;
            }
            else if (program_which_screen == 4) // lore section (return)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 4;
            }
            else if (program_which_screen == 5) // score section (return)
            {
                Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                program_which_screen = 5;
            }

            break;

        case GLFW_KEY_ENTER:
            if (program_which_screen == 0) // main menu (start)
            {
                Mix_PlayChannel(-1, m_screen_selected_sound, 0);
                program_which_screen = 0;

                program_on_title = false;
                program_on_game = true;
                program_on_death = false;

                game_resume();
            }
            else if (program_which_screen == 1) // main menu (lore)
            {
                Mix_PlayChannel(-1, m_screen_selected_sound, 0);
                program_which_screen = 4;
            }
            else if (program_which_screen == 2) // main menu (score)
            {
                Mix_PlayChannel(-1, m_screen_selected_sound, 0);
                program_which_screen = 5;
            }
            else if (program_which_screen == 3) // main menu (exit)
            {
                Mix_PlayChannel(-1, m_screen_selected_sound, 0);
                program_which_screen = 0;

                program_on_title = true;
                program_on_game = false;
                program_on_death = false;

                destroy();
                exit(0);
            }
            else if (program_which_screen == 4) // lore section (return)
            {
                Mix_PlayChannel(-1, m_screen_selected_sound, 0);
                program_which_screen = 1;
            }
            else if (program_which_screen == 5) // score section (return)
            {
                Mix_PlayChannel(-1, m_screen_selected_sound, 0);
                program_which_screen = 2;
            }

            break;

        default:
            break;
        }
    }

    // the following keyboard input is for program_on_death
    if (program_on_death && action == GLFW_RELEASE)
    {
        switch (key)
        {
            case GLFW_KEY_UP:
                if (program_which_screen == 6)
                {
                    Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                    program_which_screen = 6;
                }
                else if (program_which_screen == 7)
                {
                    Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                    program_which_screen = 6;
                }

                break;

            case GLFW_KEY_DOWN:
                if (program_which_screen == 6)
                {
                    Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                    program_which_screen = 7;
                }
                else if (program_which_screen == 7)
                {
                    Mix_PlayChannel(-1, m_screen_navigate_sound, 0);
                    program_which_screen = 7;
                }

                break;

            case GLFW_KEY_ENTER:
                if (program_which_screen == 6)
                {
                    Mix_PlayChannel(-1, m_screen_selected_sound, 0);
                    program_which_screen = 0;

                    program_on_title = false;
                    program_on_game  = true;
                    program_on_death = false;

                    game_resume();
                }
                else if (program_which_screen == 7)
                {
                    Mix_PlayChannel(-1, m_screen_selected_sound, 0);
                    program_which_screen = 0;

                    program_on_title = true;
                    program_on_game  = false;
                    program_on_death = false;

                    // game_restart(); not needed because already called on death
                    // game_pause();   not needed because already called on death
                }

                break;

            default:
                break;
        }
    }

    // the following keyboard input is for program_on_game
    if (program_on_game && action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
        Mix_PlayChannel(-1, m_screen_selected_sound, 0);
        program_which_screen = 0;

        program_on_title = true;
        program_on_game = false;
        program_on_death = false;

        game_restart();
        game_pause();
    }

    if (program_on_game && action == GLFW_PRESS && m_raider.is_alive()) {
        switch (key) {
        case GLFW_KEY_LEFT:
            if (game_is_paused) break;

            m_raider.x_velocity = m_raider.speed * -1.0f;
            m_raider.s_direction = 2;
            m_raider.m_is_move = true;

            break;

        case GLFW_KEY_RIGHT:
            if (game_is_paused) break;

            m_raider.x_velocity = m_raider.speed;
            m_raider.s_direction = 1;
            m_raider.m_is_move = true;

            break;

        case GLFW_KEY_SPACE:
            if (game_is_paused) break;

            if (m_raider_isOnPlatform) {
                Mix_PlayChannel(-1, m_raider_jump_sound, 0);
                m_raider.m_jump = true;
                m_raider.move({ 0.f, -5.0f - platform_speed * 2 });

                m_raider.y_velocity = m_raider.jump_speed * -1.0f;
            }
            break;

        default:
            break;
        }
    }
    else if (program_on_game && action == GLFW_RELEASE && m_raider.is_alive()) {
        switch (key) {
        case GLFW_KEY_LEFT:
            if (game_is_paused) break;

            if (!keyboard_left_is_pressed && !keyboard_right_is_pressed)
            {
                m_raider.x_velocity = 0.0f;
                m_raider.m_is_move = false;
            }

            break;

        case GLFW_KEY_RIGHT:
            if (game_is_paused) break;

            if (!keyboard_left_is_pressed && !keyboard_right_is_pressed)
            {
                m_raider.x_velocity = 0.0f;
                m_raider.m_is_move = false;
            }

            break;

        case GLFW_KEY_P:
            if (game_is_paused) {
                game_resume();
            }
            else {
                game_pause();
            }
            break;

        default:
            break;
        }
    }
}

//Generate the spike ceiling
bool World::create_spikeCeiling() {
    for (int i = 0; i < 10; i++) {
        Spike spike;
        if (spike.init()) {
            m_spike.emplace_back(spike);
        }
        else {
            fprintf(stderr, "Failed to generate spike.");
            return false;
        }

        Spike& new_spike = m_spike.back();
        new_spike.set_position({ 40.f + 60.f * i, 40.f });
    }
    return true;
}

// Creates a new platform and if successfull adds it to the list of platform
bool World::spawn_platform() {
    Platform platform;
    if (platform.init()) {
        platform.type = 0;
        m_platform.emplace_back(platform);

        return true;
    }
    fprintf(stderr, "Failed to spawn platform");
    return false;
}

void World::on_mouse_move(GLFWwindow* window, double xpos, double ypos) {
    ///leave this for future use
}

// helper logic for land sound
int land_sound_current = 0;
int land_sound_maximum = 4;

void World::play_land_sound()
{
    if (land_sound_current == 0)
    {
        Mix_PlayChannel(-1, m_raider_land_1_sound, 0);
    }

    if (land_sound_current == 1)
    {
        Mix_PlayChannel(-1, m_raider_land_2_sound, 0);
    }

    if (land_sound_current == 2)
    {
        Mix_PlayChannel(-1, m_raider_land_3_sound, 0);
    }

    if (land_sound_current == 3)
    {
        Mix_PlayChannel(-1, m_raider_land_4_sound, 0);
    }

    land_sound_current = (land_sound_current + 1) % land_sound_maximum;

    timestamp_last_played_land_audio = get_timestamp_in_msec();
}

// helper logic for walk sound
int walk_sound_current = 0;
int walk_sound_maximum = 4;

void World::play_walk_sound()
{
    if (walk_sound_current == 0)
    {
        Mix_PlayChannel(-1, m_raider_walk_1_sound, 0);
    }

    if (walk_sound_current == 1)
    {
        Mix_PlayChannel(-1, m_raider_walk_2_sound, 0);
    }

    if (walk_sound_current == 2)
    {
        Mix_PlayChannel(-1, m_raider_walk_3_sound, 0);
    }

    if (walk_sound_current == 3)
    {
        Mix_PlayChannel(-1, m_raider_walk_4_sound, 0);
    }

    walk_sound_current = (walk_sound_current + 1) % walk_sound_maximum;

    timestamp_last_played_walk_audio = get_timestamp_in_msec();
}

void World::save_GameScore(const char* filename) {
    std::fstream out;
    out.open(filename, std::fstream::out);
    for (int i = 0; i < sizeof(highest_scores) / sizeof(highest_scores[0]); i++) {
        out << highest_scores[i] << " ";
    }
    out << std::endl;
    out.close();

    /*std::fstream out;
    out.open(filename, std::fstream::out);
    Json::Value root;
    Json::Value scores(Json::arrayValue);
    for (int i = 0; i < sizeof(highest_scores) / sizeof(highest_scores[0]); i++) {
        scores.append(Json::Value(highest_scores[i]));
    }
    root["scores"] = scores;
    Json::StyledWriter styledWriter;
    out << styledWriter.write(root);

    out.close();*/
}

void World::load_GameScore(const char* filename) {
    std::fstream in;
    in.open(filename, std::fstream::in);
    for (int i = 0; i < sizeof(highest_scores) / sizeof(highest_scores[0]); i++) {
        in >> highest_scores[i];
    }
    in.close();

    /*Json::Value root;
    Json::Reader reader;
    std::ifstream doc(filename);
    doc >> root;
    auto scores_from_file = root["scores"];
    for (int i = 0; i < sizeof(highest_scores) / sizeof(highest_scores[0]); i++) {
        highest_scores[i] = scores_from_file[i].asInt();
    }*/
}

bool World::random_platform(float ms, vec2 screen) {

    ms = ms * 1.10f;

    int spread = 12;
    float step_size = 0.3f;
    float noisy_unit = 1.0f + step_size * (rand() % spread - spread / 2);
    m_next_platform_spawn -= ms * platform_speed * noisy_unit;
    if (m_platform.size() <= MAX_PLATFORM && m_next_platform_spawn <= 0.f) {
        if (!spawn_platform())
            return false;

        Platform& new_platform = m_platform.back();
        if (!isTar) {
            new_platform.type = 0;
        }
        else if (!isGhost) {
            new_platform.type = m_dist_tar(m_rng);
        }
        else if (!isLava) {
            new_platform.type = m_dist_ghost(m_rng);
        }
        else
            new_platform.type = m_disc_dist(m_rng);

        // to avoid multiple lava platforms spawning too close in time
        if (prev_platform.type == 2) {
            new_platform.type = 0;
        }
        // same thing for the spring platform
        if (prev_platform.type == 1) {
            new_platform.type = 0;
        }

        float plat_width = new_platform.get_bounding_box().x;
        float random_x = (float)rand() / (float)(RAND_MAX) * (screen.x - plat_width)
            + (plat_width / 2);
        // compute random_x to avoid platform spawn too close to each other
        // if random_x is too close to prev platform and is to the right of prev
        if (random_x > prev_platform_x && ((random_x - prev_platform_x) < plat_width / 2)) {
            // if random_x is too close to the right edge
            if (random_x + plat_width / 2 >= screen.x - plat_width) {
                random_x -= plat_width / 2 + (random_x - prev_platform_x);
            }
            else random_x += plat_width / 2;
        }
        // if random_x is too close to prev platform and is to the left of prev
        else if (random_x < prev_platform_x && ((prev_platform_x - random_x) < plat_width / 2)) {
            // if random_x is too close to the left edge
            if (random_x - plat_width / 2 <= plat_width) {
                random_x += plat_width / 2 + (prev_platform_x - random_x);
            }
            else random_x -= plat_width / 2;
        }


        new_platform.set_position({ random_x, screen.y + 10.f });
        m_next_platform_spawn = PLATFORM_DELAY_MS;
        prev_platform_x = random_x;
        prev_platform = new_platform;
        //set have_new_platform to true if has new one this update
        have_new_platform = true;
        new_platform_pos = { random_x, screen.y + 10.f };
        new_platform_bounding_box = new_platform.get_bounding_box();
    }
    else {
        have_new_platform = false;
    }
    return true;
}

void World::game_pause() {
    // performing changes to paused vars
    game_is_paused = true;

    paused_start_in_sec = get_timestamp_in_sec();
    paused_start_in_msec = get_timestamp_in_msec();

    Mix_HaltMusic();
}

void World::game_resume() {
    // performing changes to paused vars
    game_is_paused = false;

    paused_end_in_sec = get_timestamp_in_sec();
    paused_end_in_msec = get_timestamp_in_msec();

    // performing changes to timestamp vars
    unsigned long delta_sec = paused_end_in_sec - paused_start_in_sec;
    unsigned long delta_msec = paused_end_in_msec - paused_start_in_msec;

    timestamp_game_start_in_sec += delta_sec;
    timestamp_game_start_in_msec += delta_msec;

    timestamp_last_supported_in_sec += delta_sec;
    timestamp_last_supported_in_msec += delta_msec;

    timestamp_last_played_land_audio += delta_sec;
    timestamp_last_played_walk_audio += delta_msec;

    Mix_PlayMusic(m_background_music, -1);
}


bool World::game_restart() {
    //save game score
    rank = get_rank_and_saveScore();

    m_raider.destroy();
    m_raider.init();
    m_platform.clear();
    for (auto& spike : m_spike)
        spike.destroy();
    m_spike.clear();
    for (auto& fs : m_falling_spikes)
        fs.destroy();
    m_falling_spikes.clear();
    m_coins.clear();
    m_b_fireballs.clear();
    isRegular = true;
    isTar = false;
    isLava = false;
    isGhost = false;
    platform_speed = 0.6f;
    m_fireball.set_position({ -10.f, 70.f });
    m_parafireball.cast(60.f, 0.2f);
    m_parafireball2.cast(60.f, 0.2f);
    m_b_fireball.destroy();


    if (!create_spikeCeiling()) return false;
    if (!spawn_fallingSpike({ 600.f, 800.f }, fallingSpike_num)) return false;

    m_water.reset_raider_dead_time();

    // the following is state initialization #2
    INITED_LEVEL_1  = false;
    INITED_LEVEL_2  = false;
    INITED_LEVEL_3  = false;
    INITED_LEVEL_4  = false;
    INITED_LEVEL_5  = false;
    INITED_LEVEL_6  = false;
    INITED_LEVEL_7  = false;
    INITED_LEVEL_8  = false;
    INITED_LEVEL_9  = false;
    INITED_LEVEL_10 = false;

    m_backdrop.destroy();

    m_score_label.destroy();
    m_score_digit_1.destroy();
    m_score_digit_2.destroy();
    m_score_digit_3.destroy();
    m_score_digit_4.destroy();
    m_screen.destroy();

    m_overlay_esc.destroy();
    m_overlay_left.destroy();
    m_overlay_space.destroy();
    m_overlay_right.destroy();

    paused_start_in_sec              = get_timestamp_in_sec();
    paused_start_in_msec             = get_timestamp_in_msec();

    score_time                       = 0;
    score_coin                       = 0;
    score_show                       = 0;

    timestamp_game_start_in_sec      = get_timestamp_in_sec();
    timestamp_game_start_in_msec     = get_timestamp_in_msec();

    timestamp_last_supported_in_sec  = get_timestamp_in_sec();
    timestamp_last_supported_in_msec = get_timestamp_in_msec();

    raider_platform_collide_now      = false;
    raider_platform_collide_sound    = false;

    raider_warning_now               = false;
    raider_warning_sound             = false;

    timestamp_last_played_land_audio = get_timestamp_in_msec();
    timestamp_last_played_walk_audio = get_timestamp_in_msec();

    raider_walk_step_position        = 300.0f;
    raider_should_die                = false;

    m_score_label.init();
    m_score_digit_1.init();
    m_score_digit_2.init();
    m_score_digit_3.init();
    m_score_digit_4.init();
    m_screen.init();

    m_overlay_esc.init("overlay_key_esc_on.png", "overlay_key_esc_off.png");
    m_overlay_left.init("overlay_key_left_on.png", "overlay_key_left_off.png");
    m_overlay_space.init("overlay_key_space_on.png", "overlay_key_space_off.png");
    m_overlay_right.init("overlay_key_right_on.png", "overlay_key_right_off.png");

    m_backdrop.init();

    // initialize a platform
    if (!spawn_platform())
        return false;
    Platform& new_platform = m_platform.back();
    new_platform.type = 0;
    new_platform.set_position({ 300.f, 600.f });

    return true;
}

// return -1 if m_points does not belong to top 5;
// return rank of 1...5 otherwise
int World::get_rank_and_saveScore() {
    int result = -1;
    int prev = -1;
    for (int i = 0; i < sizeof(highest_scores) / sizeof(highest_scores[0]); i++) {
        if (m_points > highest_scores[i] && prev == -1) {
            prev = highest_scores[i];
            highest_scores[i] = m_points;
            result = i + 1;
        }
        else if (prev != -1) {
            int temp = highest_scores[i];
            highest_scores[i] = prev;
            prev = temp;
        }
    }
    return result;
}

void World::draw_rank(const mat3& projection, float y, int rank) {
    m_score_label.set_position({ 300.f, y });

    m_score_label.set_scale({ 0.75f * 1.80f, 0.75f * 1.00f });
    m_score_label.death_screen_rank_show = true;
    m_score_label.death_screen_rank = rank;
    m_score_label.draw(projection);
    m_score_label.death_screen_rank_show = false;
    m_score_label.set_scale({ 0.75f * 1.00f, 0.75f * 1.00f });
}

void World::draw_score(const mat3& projection, float y, unsigned int m_points,
    bool high_score, int high_numb)
{
    int number_of_digits = get_number_of_digits(m_points);
    int number_of_points = (int)fmin(9999, m_points);

    if (high_score)
    {
        m_score_label.high_score_show = true;
        m_score_label.high_score_numb = high_numb;
    }
    else
    {
        m_score_label.high_score_show = false;
        m_score_label.high_score_numb = 0;
    }

    if (number_of_digits == 4 || high_score)
    {
        m_score_label.set_position({ 300.0f + score_digit_4_team_offset, y });
        m_score_label.draw(projection);

        m_score_digit_1.m_number = get_number_nth_digit(number_of_points, 3);
        m_score_digit_2.m_number = get_number_nth_digit(number_of_points, 2);
        m_score_digit_3.m_number = get_number_nth_digit(number_of_points, 1);
        m_score_digit_4.m_number = get_number_nth_digit(number_of_points, 0);

        m_score_digit_1.set_position({ 300.0f + score_digit_4_team_offset + score_digit_1_sing_offset, y });
        m_score_digit_2.set_position({ 300.0f + score_digit_4_team_offset + score_digit_2_sing_offset, y });
        m_score_digit_3.set_position({ 300.0f + score_digit_4_team_offset + score_digit_3_sing_offset, y });
        m_score_digit_4.set_position({ 300.0f + score_digit_4_team_offset + score_digit_4_sing_offset, y });

        m_score_digit_1.draw(projection);
        m_score_digit_2.draw(projection);
        m_score_digit_3.draw(projection);
        m_score_digit_4.draw(projection);
    }

    else if (number_of_digits == 3)
    {
        m_score_label.set_position({ 300.0f + score_digit_3_team_offset, y });
        m_score_label.draw(projection);

        m_score_digit_1.m_number = get_number_nth_digit(number_of_points, 2);
        m_score_digit_2.m_number = get_number_nth_digit(number_of_points, 1);
        m_score_digit_3.m_number = get_number_nth_digit(number_of_points, 0);

        m_score_digit_1.set_position({ 300.0f + score_digit_3_team_offset + score_digit_1_sing_offset, y });
        m_score_digit_2.set_position({ 300.0f + score_digit_3_team_offset + score_digit_2_sing_offset, y });
        m_score_digit_3.set_position({ 300.0f + score_digit_3_team_offset + score_digit_3_sing_offset, y });

        m_score_digit_1.draw(projection);
        m_score_digit_2.draw(projection);
        m_score_digit_3.draw(projection);
    }

    else if (number_of_digits == 2)
    {
        m_score_label.set_position({ 300.0f + score_digit_2_team_offset, y });
        m_score_label.draw(projection);

        m_score_digit_1.m_number = get_number_nth_digit(number_of_points, 1);
        m_score_digit_2.m_number = get_number_nth_digit(number_of_points, 0);

        m_score_digit_1.set_position({ 300.0f + score_digit_2_team_offset + score_digit_1_sing_offset, y });
        m_score_digit_2.set_position({ 300.0f + score_digit_2_team_offset + score_digit_2_sing_offset, y });

        m_score_digit_1.draw(projection);
        m_score_digit_2.draw(projection);
    }

    else if (number_of_digits == 1)
    {
        m_score_label.set_position({ 300.0f + score_digit_1_team_offset, y });
        m_score_label.draw(projection);

        m_score_digit_1.m_number = get_number_nth_digit(number_of_points, 0);

        m_score_digit_1.set_position({ 300.0f + score_digit_1_team_offset + score_digit_1_sing_offset, y });

        m_score_digit_1.draw(projection);
    }

    else
    {
        m_score_label.set_position({ 300.0f + score_digit_1_team_offset, y });
        m_score_label.draw(projection);

        m_score_digit_1.m_number = get_number_nth_digit(number_of_points, 0);

        m_score_digit_1.set_position({ 300.0f + score_digit_1_team_offset + score_digit_1_sing_offset, y });

        m_score_digit_1.draw(projection);
    }
}

bool World::spawn_coin() {
    Coin coin;
    if (coin.init()) {
        m_coins.emplace_back(coin);

        return true;
    }
    fprintf(stderr, "Failed to spawn coin");
    return false;
}

bool World::spawn_b_fireball() {
    Bouncingfireball bouncingfireball;
    if (bouncingfireball.init()) {
        m_b_fireballs.emplace_back(bouncingfireball);
        return true;
    }
    fprintf(stderr, "Failed to spawn bouncing fireball");
    return false;
}

// spawn num of fallingspikes
bool World::spawn_fallingSpike(vec2 screen, int num) {
    for (int i = 0; i < num; i++) {
        fallingSpike fs;
        if (!fs.init()) {
            return false;
        }
        else {
            m_falling_spikes.emplace_back(fs);
            fallingSpike& new_fs = m_falling_spikes.back();
            vec2 pos = get_random_fallingSpike_position(num, i, new_fs, screen);
            prev_fs_y = pos.y;
            new_fs.set_position(pos);
        }
    }
    return true;
}

vec2 World::get_random_fallingSpike_position(int num, int i, fallingSpike fs, vec2 screen) {
    float fs_w = fs.get_bounding_box().x;
    float rd = (float)rand() / (float)(RAND_MAX) * (screen.x / num) / 2;
    float random_x = (screen.x / num) * i + rd;
    if ((random_x - fs_w / 2.f) <= 0.f)
        random_x += abs(random_x - fs_w);
    if (random_x + fs_w / 2.f >= screen.x)
        random_x -= (random_x + fs_w / 2.f - screen.x);

    float random_y = (float)rand() / (float)(RAND_MAX)* screen.y * 0.75 - (screen.y);
    float fs_h = fs.get_bounding_box().y;
    // if random_y is too close to prev fs and is below prev
    if (random_y > prev_fs_y && ((random_y - prev_fs_y) < fs_h / 2)) {
        // if random_y is too close to the bottom edge
        if (random_y + fs_h / 2 >= 0.f) {
            random_y -= fs_h + (random_y - prev_fs_y);
        }
        else random_y += fs_h;
    }
    // if random_y is too close to prev fs and is on top of prev
    else if (random_y < prev_fs_y && ((prev_fs_y - random_y) < fs_h / 2)) {
        random_y -= fs_h;
    }

    return { random_x, random_y };
}

bool World::check_triggerWarning() {
    bool fs_falling = false;
    for (auto& fs : m_falling_spikes)
        fs_falling = (fs.isFalling && fs.get_position().y < 0.f);
    bool b_fireball_falling = false;
    for (auto& bf : m_b_fireballs)
        b_fireball_falling = (bf.get_position().y < 0.f);
    return
        (fs_falling || b_fireball_falling);
}

Bouncingfireball World::random_b_fireball(Bouncingfireball bouncingfireball, vec2 screen) {
    vec2 r_pos = m_raider.get_position();
    float side_pro = m_dist(m_rng);
    float updown_pro = m_dist(m_rng);
    float speed = 0.3;

    if (side_pro > 0.5)
    {
        //lefttop
        bouncingfireball.set_position({ 100.f,-200.f });
//        vec2 dir = sub(r_pos, { 100.f,100.f });
        float r1 = m_dist(m_rng);
        float r2 = m_dist(m_rng);
        vec2 dir = {1,r1/3+0.4f};
        vec2 norm_dir = normalize(dir);
        bouncingfireball.set_velocity(mul(norm_dir, speed));

        return bouncingfireball;
    }
    else
    {
        //righttop
        bouncingfireball.set_position({ screen.x - 100.f,-200.f });
//        vec2 dir = sub(r_pos, { screen.x - 100.f,100.f });
        float r1 = m_dist(m_rng);
        float r2 = m_dist(m_rng);
        vec2 dir = {-1,r2/3+0.4f};
        vec2 norm_dir = normalize(dir);
        bouncingfireball.set_velocity(mul(norm_dir, speed));

        return bouncingfireball;
    }
}

void World::trigger_deathEvent() {
    Mix_PlayChannel(-1, m_raider_dead_sound, 0);
    m_water.set_raider_dead();
    m_raider.y_velocity = -8.0f;
    m_raider.rotation_speed = (float)M_PI / 75.f;
    m_raider.m_jump = false;
    m_raider.kill();
}
