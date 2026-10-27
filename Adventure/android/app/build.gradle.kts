import javax.inject.Inject

plugins {
    id("com.android.application")
}

// the engine's checkout, whose CMakeLists.txt builds libAdventure.so and the
// engine's libCGE.so
val cgeDir: File = rootDir.resolve("../..").canonicalFile
val advDir: File = cgeDir.resolve("Adventure/AdvEngine")

// The flavor "game" packs one game, the folder with its game.dat (gfx.dat,
// sfx.dat ...), as an app of its own: -Padventure.game=<folder>, with
// -Padventure.applicationId, -Padventure.name and -Padventure.versionCode
// (or in gradle.properties). By default it is the demo game.
val gameDir: File = providers.gradleProperty("adventure.game").map { file(it) }
    .getOrElse(advDir.resolve("test/data")).canonicalFile
val gameApplicationId: String = providers.gradleProperty("adventure.applicationId")
    .getOrElse("de.captain_online.adventure.demo")
val gameName: String = providers.gradleProperty("adventure.name").getOrElse("Adventure Demo")
val gameVersionCode: Int = providers.gradleProperty("adventure.versionCode").map { it.toInt() }.getOrElse(1)

// The release signing key comes from Gradle properties, e.g. in
// ~/.gradle/gradle.properties; without them the release build is unsigned.
val releaseKeystore = providers.gradleProperty("adventure.keystore")

android {
    namespace = "de.captain_online.adventure"
    compileSdk = 37
    ndkVersion = "30.0.16248370"

    defaultConfig {
        applicationId = "de.captain_online.adventure"
        minSdk = 24
        targetSdk = 36
        versionCode = 1
        versionName = "0.1"
        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }
        externalNativeBuild {
            cmake {
                // the engine and the Adventure share STL objects; of the
                // engine's projects only the Adventure runs on Android
                arguments += listOf("-DANDROID_STL=c++_shared",
                    "-DPROJECT_HEROQUEST=OFF", "-DPROJECT_OSMRACER=OFF")
                targets += "Adventure"
            }
        }
    }

    flavorDimensions += "content"
    productFlavors {
        // the Adventure player: a list of games, added with a file picker
        create("player") {
            dimension = "content"
            resValue("string", "app_name", "Adventure")
        }
        // one game, packed as assets (game/)
        create("game") {
            dimension = "content"
            applicationId = gameApplicationId
            versionCode = gameVersionCode
            resValue("string", "app_name", gameName)
        }
    }

    buildFeatures {
        resValues = true
    }

    externalNativeBuild {
        cmake {
            path = cgeDir.resolve("CMakeLists.txt")
            // the SDK's CMake, which comes with Ninja; Gradle installs it
            version = "4.1.2"
        }
    }

    sourceSets {
        getByName("main") {
            // de.captain.cge.CgeActivity
            java.srcDir(cgeDir.resolve("Engine/android/java"))
        }
    }

    signingConfigs {
        if (releaseKeystore.isPresent) {
            create("release") {
                storeFile = file(releaseKeystore.get())
                storePassword = providers.gradleProperty("adventure.keystorePassword").orNull
                keyAlias = providers.gradleProperty("adventure.keyAlias").orNull
                keyPassword = providers.gradleProperty("adventure.keyPassword").orNull
            }
        }
    }

    buildTypes {
        getByName("release") {
            isMinifyEnabled = false
            if (releaseKeystore.isPresent)
                signingConfig = signingConfigs.getByName("release")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

// The files the Adventure reads as assets (CGE::Assets): its engine.ini and
// the engine's fonts (fonts/), as next to the desktop executable.
abstract class CollectEngineAssets : DefaultTask() {
    @get:InputFile
    abstract val engineIni: RegularFileProperty

    @get:InputDirectory
    abstract val fonts: DirectoryProperty

    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @get:Inject
    abstract val fs: FileSystemOperations

    @TaskAction
    fun collect() {
        fs.sync {
            into(outputDir)
            from(engineIni)
            from(fonts) { into("fonts") }
        }
    }
}

// The game of the flavor "game" in game/; the launcher activity copies it
// into the app's storage, since the Adventure reads its files as plain files.
abstract class CollectGame : DefaultTask() {
    @get:InputDirectory
    abstract val game: DirectoryProperty

    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @get:Inject
    abstract val fs: FileSystemOperations

    @TaskAction
    fun collect() {
        if (!game.get().file("game.dat").asFile.isFile)
            throw GradleException("${game.get()} has no game.dat (set adventure.game to a game's folder)")
        fs.sync {
            into(outputDir)
            from(game) { into("game") }
        }
    }
}

val collectEngineAssets = tasks.register<CollectEngineAssets>("collectEngineAssets") {
    engineIni = advDir.resolve("Engine/engine.ini")
    fonts = cgeDir.resolve("Engine/fonts")
}

val collectGame = tasks.register<CollectGame>("collectGame") {
    game = gameDir
}

androidComponents {
    onVariants { variant ->
        variant.sources.assets?.addGeneratedSourceDirectory(collectEngineAssets, CollectEngineAssets::outputDir)
        if (variant.productFlavors.any { it.second == "game" })
            variant.sources.assets?.addGeneratedSourceDirectory(collectGame, CollectGame::outputDir)
    }
}
