plugins {
    alias(libs.plugins.android.application)
}

val appId = project.findProperty("appId")?.toString() ?: "app.salam.demo"
val appName = project.findProperty("appName")?.toString() ?: "Salam App"
val appVersion = project.findProperty("appVersion")?.toString() ?: "1.0"
val appVersionCode = project.findProperty("appVersionCode")?.toString()?.toIntOrNull() ?: 1

android {
    namespace = "ir.salamlang.host"
    compileSdk = 36

    defaultConfig {
        applicationId = appId
        minSdk = 21
        targetSdk = 35
        versionCode = appVersionCode
        versionName = appVersion
        resValue("string", "app_name", appName)
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            signingConfig = signingConfigs.getByName("debug")
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    buildFeatures {
        resValues = true
    }
    packaging {
        jniLibs {
            useLegacyPackaging = true
        }
    }
}
