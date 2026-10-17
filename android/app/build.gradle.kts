import java.util.zip.ZipEntry
import java.util.zip.ZipOutputStream
import javax.inject.Inject

plugins {
    alias(libs.plugins.android.application)
}

val repoRoot: File = rootProject.projectDir.parentFile
val repoVersionFile = repoRoot.resolve("VERSION")
val repoVersionName =
    repoVersionFile
        .takeIf { it.isFile }
        ?.readText()
        ?.trim()
        ?.takeIf { it.isNotEmpty() }
        ?: error("VERSION file is missing or empty: ${repoVersionFile.absolutePath}")

fun versionCodeOf(name: String): Int {
    val parts =
        Regex("""^(\d+)\.(\d+)(?:\.(\d+))?""").find(name)
            ?: error("VERSION '$name' is not a semantic version (expected MAJOR.MINOR[.PATCH])")
    val (major, minor, patch) = parts.destructured
    return major.toInt() * 10000 + minor.toInt() * 100 + (patch.toIntOrNull() ?: 0)
}

val engineAbis =
    linkedMapOf(
        "armeabi-v7a" to "armv7a-linux-androideabi",
        "arm64-v8a" to "aarch64-linux-android",
        "x86" to "i686-linux-android",
        "x86_64" to "x86_64-linux-android",
    )
val requestedAbis: List<String> =
    project.findProperty("salamAbis")?.toString()?.split(',')?.map { it.trim() }?.filter { it.isNotEmpty() }
        ?: engineAbis.keys.toList()
val engineGenRoot = layout.buildDirectory.dir("salam-engine")

abstract class SalamEngineTask
    @Inject
    constructor(
        private val exec: ExecOperations,
    ) : DefaultTask() {
        @get:Input abstract val abi: Property<String>

        @get:Input abstract val triple: Property<String>

        @get:InputFiles
        @get:PathSensitive(PathSensitivity.RELATIVE)
        abstract val sources: ConfigurableFileCollection

        @get:Internal abstract val script: RegularFileProperty

        @get:OutputDirectory abstract val outputDir: DirectoryProperty

        @TaskAction
        fun generate() {
            exec.exec {
                commandLine(
                    "sh",
                    script.get().asFile.absolutePath,
                    "--out",
                    outputDir.get().asFile.absolutePath,
                    "--target",
                    triple.get(),
                )
            }
        }
    }

abstract class SalamAssetsTask
    @Inject
    constructor(
        private val exec: ExecOperations,
    ) : DefaultTask() {
        @get:InputFiles
        @get:PathSensitive(PathSensitivity.RELATIVE)
        abstract val sources: ConfigurableFileCollection

        @get:Internal abstract val repo: DirectoryProperty

        @get:Input abstract val version: Property<String>

        @get:OutputDirectory abstract val outputDir: DirectoryProperty

        @TaskAction
        fun generate() {
            val root = repo.get().asFile
            val out = outputDir.get().asFile
            out.deleteRecursively()
            out.mkdirs()
            val stage = File(temporaryDir, "std")
            exec.exec {
                commandLine(
                    "sh",
                    root.resolve("tools/bash/build-mobile-engine.sh").absolutePath,
                    "--std",
                    stage.absolutePath,
                )
            }
            ZipOutputStream(out.resolve("std.zip").outputStream().buffered()).use { zip ->
                stage
                    .walkTopDown()
                    .filter { it.isFile }
                    .sortedBy { it.relativeTo(stage).invariantSeparatorsPath }
                    .forEach { f ->
                        val entry = ZipEntry(f.relativeTo(stage).invariantSeparatorsPath)
                        entry.time = 0
                        zip.putNextEntry(entry)
                        f.inputStream().use { it.copyTo(zip) }
                        zip.closeEntry()
                    }
            }
            out.resolve("std.version").writeText(version.get())
            val examples = out.resolve("examples")
            examples.mkdirs()
            root.resolve("tools/salam/editor-examples.tsv").copyTo(examples.resolve("index.tsv"))
            for (lang in listOf("en", "fa")) {
                val dir = root.resolve("tests/$lang/editor-selected")
                val dst = examples.resolve(lang)
                dst.mkdirs()
                dir.listFiles { f -> f.name.endsWith(".salam") }?.forEach { it.copyTo(dst.resolve(it.name)) }
            }
        }
    }

val engineSources =
    files(
        repoRoot.resolve("compiler"),
        repoRoot.resolve("std"),
        repoRoot.resolve("tools/bash/build-mobile-engine.sh"),
    )

val engineTasks =
    requestedAbis.map { abi ->
        val triple = engineAbis[abi] ?: error("unknown ABI '$abi' (known: ${engineAbis.keys})")
        tasks.register<SalamEngineTask>("salamEngineC_${abi.replace('-', '_')}") {
            this.abi.set(abi)
            this.triple.set(triple)
            sources.from(engineSources)
            script.set(repoRoot.resolve("tools/bash/build-mobile-engine.sh"))
            outputDir.set(engineGenRoot.map { it.dir(abi) })
        }
    }

val salamAssets =
    tasks.register<SalamAssetsTask>("salamAssets") {
        sources.from(
            repoRoot.resolve("std"),
            repoRoot.resolve("tests/en/editor-selected"),
            repoRoot.resolve("tests/fa/editor-selected"),
            repoRoot.resolve("tools/salam/editor-examples.tsv"),
        )
        repo.set(repoRoot)
        version.set(repoVersionName)
        outputDir.set(layout.buildDirectory.dir("generated/salam-assets"))
    }

android {
    namespace = "ir.salamlang.app"
    compileSdk {
        version =
            release(36) {
                minorApiLevel = 1
            }
    }
    ndkVersion = "30.0.16248370"

    defaultConfig {
        applicationId = "ir.salamlang.app"
        minSdk = 21
        targetSdk = 36
        versionCode =
            project.findProperty("appVersionCode")?.toString()?.toIntOrNull()
                ?: versionCodeOf(repoVersionName)
        versionName = project.findProperty("appVersionName")?.toString() ?: repoVersionName

        ndk {
            abiFilters += requestedAbis
        }
        externalNativeBuild {
            cmake {
                arguments += "-DSALAM_ENGINE_GEN_ROOT=${engineGenRoot.get().asFile.absolutePath}"
                arguments += "-DANDROID_STL=none"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = repoRoot.resolve("mobile/engine/CMakeLists.txt")
            version = "4.1.2"
        }
    }

    splits {
        abi {
            isEnable = true
            reset()
            include(*requestedAbis.toTypedArray())
            isUniversalApk = true
        }
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
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    buildFeatures {
        buildConfig = false
    }

    androidResources {
        localeFilters += setOf("fa", "en")
        noCompress += listOf("zip")
    }

    packaging {
        jniLibs {
            useLegacyPackaging = true
        }
        resources {
            excludes +=
                setOf(
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

androidComponents {
    onVariants { variant ->
        variant.sources.assets?.addGeneratedSourceDirectory(salamAssets, SalamAssetsTask::outputDir)
    }
}

tasks.configureEach {
    if (name.startsWith("configureCMake") || name.startsWith("buildCMake")) {
        dependsOn(engineTasks)
    }
}
