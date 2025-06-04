# **Sph Simulation**
SPH planetary collision simulation (IN PROGRESS). Built on Bismuth, a custom Entity-Component-System (ECS) engine.

&nbsp;
# **Description**
Simulation is built on Bismuth, custom ECS library. Depending on choice of iteration pattern, Bismuth can be up to 30% faster than EnTT or 10% slower (Tuple for loop).
Currently it renders a basic simulation with 1000 particles in no gravity environment.

<p float="left">
  <img alt="Image of Bob from different perspective" src="https://github.com/MrSoilder23/sph_sim/blob/development/assets/screenshots/screenshot1.png" width="49.5%" /> 
  <img alt="Image of Bob" src="https://github.com/MrSoilder23/sph_sim/blob/development/assets/screenshots/screenshot2.png" width="49.5%" />
</p>

&nbsp;
# **Installation**

## **Build**
```
cmake -S . -B build "-DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DVCPKG_MANIFEST_INSTALL=ON -G Ninja 
cmake --build build
```

## **Usage**
To change window size open ./config/config.json

```
./bin/prog
```
