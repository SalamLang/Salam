plugins {
    alias(libs.plugins.android.application)
}

// The repo-root VERSION file is the single source of truth for every Salam
// artifact, so the app reads its version from there instead of carrying a
// second copy that silently drifts. CI can still override either value with
// -PappVersionName / -PappVersionCode (see _reusable-android-build.yml).
val repoVersionFile = rootProject.projectDir.parentFile.resolve("VERSION")
val repoVersionName =
    repoVersionFile
        .takeIf { it.isFile }
        ?.readText()
        ?.trim()
        ?.takeIf { it.isNotEmpty() }
        ?: error("VERSION file is missing or empty: ${repoVersionFile.absolutePath}")

// 0.3.0 -> 300. Play requires a monotonically increasing integer; this stays
// ordered as long as minor and patch stay below 100. Any pre-release suffix
// (0.3.0-rc1) is ignored - it does not change the ordering.
fun versionCodeOf(name: String): Int {
    val parts =
        Regex("""^(\d+)\.(\d+)(?:\.(\d+))?""").find(name)
            ?: error("VERSION '$name' is not a semantic version (expected MAJOR.MINOR[.PATCH])")
    val (major, minor, patch) = parts.destructured
    return major.toInt() * 10000 + minor.toInt() * 100 + (patch.toIntOrNull() ?: 0)
}

android {
    namespace = "ir.salamlang.app"
    compileSdk {
        version =
            release(36) {
                minorApiLevel = 1
            }
    }

    defaultConfig {
        applicationId = "ir.salamlang.app"
        minSdk = 24
        targetSdk = 36
        versionCode =
            project.findProperty("appVersionCode")?.toString()?.toIntOrNull()
                ?: versionCodeOf(repoVersionName)
        versionName = project.findProperty("appVersionName")?.toString() ?: repoVersionName

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    signingConfigs {
        create("release") {
            val keystoreFile = System.getenv("ANDROID_KEYSTORE_FILE")
            if (!keystoreFile.isNullOrBlank()) {
                storeFile = file(keystoreFile)
                storePassword = System.getenv("ANDROID_KEYSTORE_PASSWORD")
                keyAlias = System.getenv("ANDROID_KEY_ALIAS")
                keyPassword = System.getenv("ANDROID_KEY_PASSWORD")
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro",
            )
            signingConfig =
                if (System.getenv("ANDROID_KEYSTORE_FILE").isNullOrBlank()) {
                    signingConfigs.getByName("debug")
                } else {
                    signingConfigs.getByName("release")
                }
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    buildFeatures {
        buildConfig = false
    }

    // The app UI itself only ships Persian + English strings; without this,
    // every dependency (AppCompat/Material/etc.) bundles its translations
    // for ~70 locales into the APK.
    androidResources {
        localeFilters += setOf("fa", "en")
    }

    packaging {
        resources {
            excludes += setOf(
                "META-INF/*.version",
                "META-INF/DEPENDENCIES",
                "META-INF/LICENSE*",
                "META-INF/NOTICE*",
                "META-INF/*.kotlin_module",
                "kotlin/**",
                "DebugProbesKt.bin",
            )
        }
    }

    lint {
        checkReleaseBuilds = false
        abortOnError = false
    }
}

dependencies {
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.activity)
    implementation(libs.androidx.core.splashscreen)
    implementation(libs.androidx.webkit)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
}
