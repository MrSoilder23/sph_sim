#pragma once

namespace sapphire_config {
    constexpr static float G = 1.0f; // TO-DO change to 6.674E-11
    constexpr static float INITIAL_SPACING = 0.5f;
    constexpr static float SMOOTHING_LENGTH = 4.0f;

    // Pressure
    constexpr float REST_DENSITY = 0.2f;
    constexpr float STIFFNESS = 10.0f;

    // SpatialHash
    constexpr int SPATIAL_LENGTH = 32;
    constexpr float SPATIAL_LENGTH_MAX = SPATIAL_LENGTH*sapphire_config::SMOOTHING_LENGTH;
    constexpr size_t SPATIAL_SIZE = SPATIAL_LENGTH*SPATIAL_LENGTH*SPATIAL_LENGTH; // 3D
    constexpr uint32_t HASH_SIZE = 8192;

    // GPU
    constexpr unsigned int WORKGROUP_SIZE = 64;

}