// The Android project of the Adventure: APKs or AABs with the Adventure's
// native library (built by CMake from the engine's ../../CMakeLists.txt), the
// engine's activity and a launcher activity. The flavor "player" gets its
// games through a file picker, the flavor "game" packs one game. See
// app/build.gradle.kts and the README.

pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode = RepositoriesMode.FAIL_ON_PROJECT_REPOS
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "Adventure"
include(":app")
