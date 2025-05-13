# **Sph Simulation** ***(IN PROGRESS)***
Offers planetary collision simulations (IN PROGRESS). Powered by my own bismuth ECS engine. 

# **Description**
Made in custom ECS, bismuth, which depending of loop type offers 30% efficiency boost compared to entt or 10% decrease (implemented tuple for loop).

# **Installation**

## **Build**
```
cmake -S . -B build "-DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DVCPKG_MANIFEST_INSTALL=ON -G Ninja 
cmake --build build
```

## **Usage**
```
./bin/prog
```